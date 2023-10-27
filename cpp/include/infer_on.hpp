#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <sstream>

#include "onnxruntime_cxx_api.h"

struct NetConfig
{
	bool nms;
	float confThreshold;
	float nmsThreshold;
	std::string modelPath;
	std::vector<std::string> Classes;
};

typedef struct BoxInfo
{
	float x1;
	float y1;
	float x2;
	float y2;
	float score;
	int label;
} BoxInfo;

class YOLO
{
public:
	YOLO(NetConfig config);
	std::vector<BoxInfo> detect(cv::Mat& frame);
	void drawBoxes(cv::Mat& frame, std::vector<BoxInfo>& generatedBoxes, const ColorPalette& palette);
private:
	int inpWidth;
	int inpHeight;
	int outputNodeDim;
	int numberOfProposals;
	std::vector<Ort::AllocatedStringPtr> inputNamesPtr;
	std::vector<Ort::AllocatedStringPtr> outputNamesPtr;
	std::vector<const char*> inputNames;
	std::vector<const char*> outputNames;
	std::vector<std::string> ClassNames;
	int numClass;

	bool nms;
	float confThreshold;
	float nmsThreshold;

	// Initial model arch
	Ort::Env ortEnv = Ort::Env(ORT_LOGGING_LEVEL_ERROR, "YOLODetector");
	Ort::SessionOptions sessionOptions = Ort::SessionOptions();
	Ort::Session ortSession = Ort::Session(nullptr);
	void modelParam();
	void postProcess(cv::Mat& frame, std::vector<Ort::Value>& outputTensor, std::vector<BoxInfo>& generatedBoxes);
	void featMapScale(float ratioHeight, float ratioWidth, float* pData, std::vector<BoxInfo>& generatedBoxes);
	void nonMaximumSuppression(std::vector<BoxInfo>& inputBoxes);

};

YOLO::YOLO(NetConfig config)
{
	// Setting Threshold
	this->nms = config.nms;
	this->confThreshold = config.confThreshold;
	this->nmsThreshold = config.nmsThreshold;

	// Loading model
	sessionOptions.SetGraphOptimizationLevel(ORT_ENABLE_ALL);
	ORTCHAR_T* path = const_cast<ORTCHAR_T*>(config.modelPath.c_str());
	ortSession = Ort::Session(ortEnv, path, sessionOptions);
	modelParam();

	// Setting parameter
	this->ClassNames = config.Classes;
	this->numClass = ClassNames.size();
}

void YOLO::modelParam()
{

	std::vector<std::vector<int64_t>> inputNodeDims;
	std::vector<std::vector<int64_t>> outputNodeDims;
	size_t numInputNodes = ortSession.GetInputCount();
	size_t numOutputNodes = ortSession.GetOutputCount();
	Ort::AllocatorWithDefaultOptions allocator;

	// Get input node name/dims
	for (int i = 0; i < numInputNodes; i++) {
		Ort::TypeInfo inputTypeInfo = ortSession.GetInputTypeInfo(i);
		auto inputTensorInfo = inputTypeInfo.GetTensorTypeAndShapeInfo();
		auto inputDims = inputTensorInfo.GetShape();
		inputNodeDims.push_back(inputDims);

		Ort::AllocatedStringPtr inputNamePtr = ortSession.GetInputNameAllocated(i, allocator);
		this->inputNames.push_back(inputNamePtr.get());
		this->inputNamesPtr.push_back(std::move(inputNamePtr));
	}

	// Get output node name/dims
	for (int i = 0; i < numOutputNodes; i++) {
		Ort::TypeInfo outputTypeInfo = ortSession.GetOutputTypeInfo(i);
		auto outputTensorInfo = outputTypeInfo.GetTensorTypeAndShapeInfo();
		auto outputDims = outputTensorInfo.GetShape();
		outputNodeDims.push_back(outputDims);

		Ort::AllocatedStringPtr outputNamePtr = ortSession.GetOutputNameAllocated(i, allocator);
		this->outputNames.push_back(outputNamePtr.get());
		this->outputNamesPtr.push_back(std::move(outputNamePtr));
	}

	this->inpHeight = inputNodeDims[0][2];
	this->inpWidth = inputNodeDims[0][3];
	this->outputNodeDim = outputNodeDims[0][2];
	this->numberOfProposals = outputNodeDims[0][1];
}

std::vector<BoxInfo> YOLO::detect(cv::Mat& frame) {
	std::vector<BoxInfo> generatedBoxes;

	// Preprocess images
	cv::Mat blob = cv::dnn::blobFromImage(frame, 1 / 255.0, \
		cv::Size(this->inpWidth, this->inpHeight), \
		cv::Scalar(0, 0, 0), true, false);
	slog::info << "Blob shape: " << blob.size << slog::endl;
	// CHW format
	std::vector<int64_t> inputShape = { 1,  blob.size[1], blob.size[2], blob.size[3] };

	// Create input and output arrays
	auto allocatorInfo = Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, \
		OrtMemType::OrtMemTypeDefault);
	Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
		allocatorInfo, \
		blob.ptr<float>(), \
		blob.total(), \
		inputShape.data(), \
		inputShape.size()
	);
	// Run inference
	std::vector<Ort::Value> outputTensor = ortSession.Run(
		Ort::RunOptions{ nullptr }, \
		inputNames.data(), \
		& inputTensor, \
		1, \
		outputNames.data(), \
		outputNames.size()
	);
	// Postprocess
	postProcess(frame, outputTensor, generatedBoxes);

	return generatedBoxes;
}

void YOLO::postProcess(cv::Mat& frame, std::vector<Ort::Value>& outputTensor, std::vector<BoxInfo>& generatedBoxes) {
	// Detection and Real of rate
	float ratioHeight = (float)frame.rows / this->inpHeight;
	float ratioWidth = (float)frame.cols / this->inpWidth;
	float* pData = outputTensor[0].GetTensorMutableData<float>();
	if (this->nms) {
		featMapScale(ratioHeight, ratioWidth, pData, generatedBoxes);
		nonMaximumSuppression(generatedBoxes);
	}
	else {
		std::vector<float> group;
		for (Ort::Value& output : outputTensor) {
			float* outputData = output.GetTensorMutableData<float>();
			// Get output nums
			size_t numElements = output.GetTensorTypeAndShapeInfo().GetElementCount();
			// Collect information
			for (size_t i = 0; i < numElements; i++) {
				group.push_back(outputData[i]);
				// std::cout << outputData[i] << std::endl;
				if (group.size() == 7) {
					float box_score = group[6];
					// Threshold
					if (box_score > this->confThreshold) {
						float xmin = group[1] * ratioWidth;
						float ymin = group[2] * ratioHeight;
						float xmax = group[3] * ratioWidth;
						float ymax = group[4] * ratioHeight;
						int label = group[5];
						generatedBoxes.push_back(BoxInfo{ xmin, ymin, xmax, ymax, box_score, label });
					}
					group.clear();
				}
			}
		}
	}
}

void YOLO::featMapScale(float ratioHeight, float ratioWidth, float* pData, std::vector<BoxInfo>& generatedBoxes) {
	int n = 0, k = 0;
	for (n = 0; n < this->numberOfProposals; n++)
	{
		float box_score = pData[4];
		if (box_score > this->confThreshold)
		{
			int max_ind = 0;
			float max_class_socre = 0;
			for (k = 0; k < this->numClass; k++)
			{
				if (pData[k + 5] > max_class_socre)
				{
					max_class_socre = pData[k + 5];
					max_ind = k;
				}
			}
			max_class_socre *= box_score;
			if (max_class_socre > this->confThreshold)
			{
				float cx = pData[0] * ratioWidth;  ///cx
				float cy = pData[1] * ratioHeight;   ///cy
				float w = pData[2] * ratioWidth;   ///w
				float h = pData[3] * ratioHeight;  ///h

				float xmin = cx - 0.5 * w;
				float ymin = cy - 0.5 * h;
				float xmax = cx + 0.5 * w;
				float ymax = cy + 0.5 * h;

				generatedBoxes.push_back(BoxInfo{ xmin, ymin, xmax, ymax, max_class_socre, max_ind });
			}
		}
		pData += this->outputNodeDim;
	}
}

// Removes overlapping boxes with lower scores
void YOLO::nonMaximumSuppression(std::vector<BoxInfo>& inputBoxes) {
	sort(inputBoxes.begin(), inputBoxes.end(), [](BoxInfo a, BoxInfo b) { return a.score > b.score; });
	std::vector<float> vArea(inputBoxes.size());
	for (int i = 0; i < int(inputBoxes.size()); ++i)
	{
		vArea[i] = (inputBoxes.at(i).x2 - inputBoxes.at(i).x1 + 1)
			* (inputBoxes.at(i).y2 - inputBoxes.at(i).y1 + 1);
	}

	std::vector<bool> isSuppressed(inputBoxes.size(), false);
	for (int i = 0; i < int(inputBoxes.size()); ++i)
	{
		if (isSuppressed[i]) { continue; }
		for (int j = i + 1; j < int(inputBoxes.size()); ++j)
		{
			if (isSuppressed[j]) { continue; }
			float xx1 = (std::max)(inputBoxes[i].x1, inputBoxes[j].x1);
			float yy1 = (std::max)(inputBoxes[i].y1, inputBoxes[j].y1);
			float xx2 = (std::min)(inputBoxes[i].x2, inputBoxes[j].x2);
			float yy2 = (std::min)(inputBoxes[i].y2, inputBoxes[j].y2);

			float w = (std::max)(float(0), xx2 - xx1 + 1);
			float h = (std::max)(float(0), yy2 - yy1 + 1);
			float inter = w * h;
			float ovr = inter / (vArea[i] + vArea[j] - inter);

			if (ovr >= this->nmsThreshold)
			{
				isSuppressed[j] = true;
			}
		}
	}

	int idx_t = 0;
	inputBoxes.erase(remove_if(inputBoxes.begin(), inputBoxes.end(), \
		[&idx_t, &isSuppressed](const BoxInfo& f) { return isSuppressed[idx_t++]; }), inputBoxes.end());
}

// Draw box on frame
void YOLO::drawBoxes(cv::Mat& frame, std::vector<BoxInfo>& generatedBoxes, const ColorPalette& palette) {
	for (size_t i = 0; i < generatedBoxes.size(); ++i)
	{
		int xmin = int(generatedBoxes[i].x1);
		int ymin = int(generatedBoxes[i].y1);
		const auto& color = palette[generatedBoxes[i].label];
		cv::rectangle(frame, cv::Point(xmin, ymin), cv::Point(int(generatedBoxes[i].x2), int(generatedBoxes[i].y2)), color, 2);
		std::string label = cv::format("%.2f", generatedBoxes[i].score);
		label = this->ClassNames[generatedBoxes[i].label] + ":" + label;
		cv::putText(frame, label, cv::Point(xmin, ymin - 5), cv::FONT_HERSHEY_SIMPLEX, 0.75, color, 1);
	}
}
