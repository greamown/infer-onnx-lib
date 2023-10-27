#include <iostream>
#include <experimental/filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <opencv2/opencv.hpp>

namespace fs = std::experimental::filesystem;

// Get parent path
std::string getFolderPath(const std::string &filePath) {
    fs::path fullPath(filePath);
    fs::path folderPath = fullPath.parent_path();

    return folderPath.string();
}

// Get all txt path at folderPath
std::vector<std::string> findTxtFilesInFolder(const std::string &folderPath) {
    std::vector<std::string> txtFilePaths;

    try {
        for (const auto &entry : fs::directory_iterator(folderPath)) {
            if (fs::is_regular_file(entry.status()) && entry.path().extension() == ".txt") {
                txtFilePaths.push_back(entry.path().string());
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return txtFilePaths;
}

// Get all classes in txtFile
std::vector<std::string> readTxtFile(const std::string &filePath) {
    std::vector<std::string> lines;
    std::ifstream file(filePath);

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            line.pop_back();
            lines.push_back(line);
        }
        file.close();
    } else {
        std::cerr << "Error: Unable to open file " << filePath << std::endl;
    }

    return lines;
}

// Check file is/not image
bool isImage(const std::string &path) {
    std::vector<std::string> imageExtensions = {".jpg", ".jpeg", ".png", ".bmp", \
                                                    ".JPG", ".JPEG", ".PNG", ".BMP"};
    fs::path filePath(path);
    std::string extension = filePath.extension().string();
    
    for (const std::string &imageExt : imageExtensions) {
        if (extension == imageExt) {
            return true;
        }
    }
    
    return false;
}

// Count time
void CountTotalTime(double& totalTime,                                                  \
                        const std::chrono::high_resolution_clock::time_point& startTime, \
                        const std::chrono::high_resolution_clock::time_point& endTime,   \
                        std::string printContent
                    ){
    // FPS variables
    std::chrono::duration<double> timeDiff = endTime - startTime;
    totalTime = timeDiff.count();
    slog::info <<  printContent << ":" << totalTime << "s" << slog::endl;
}

// Color Palette
class ColorPalette {
private:
    std::vector<cv::Scalar> palette;

    static double getRandom(double a = 0.0, double b = 1.0) {
        static std::default_random_engine e;
        std::uniform_real_distribution<> dis(a, std::nextafter(b, std::numeric_limits<double>::max()));
        return dis(e);
    }

    static double distance(const cv::Scalar& c1, const cv::Scalar& c2) {
        auto dh = std::fmin(std::fabs(c1[0] - c2[0]), 1 - fabs(c1[0] - c2[0])) * 2;
        auto ds = std::fabs(c1[1] - c2[1]);
        auto dv = std::fabs(c1[2] - c2[2]);

        return dh * dh + ds * ds + dv * dv;
    }

    static cv::Scalar maxMinDistance(const std::vector<cv::Scalar>& colorSet,
                                     const std::vector<cv::Scalar>& colorCandidates) {
        std::vector<double> distances;
        distances.reserve(colorCandidates.size());
        for (auto& c1 : colorCandidates) {
            auto min =
                *std::min_element(colorSet.begin(), colorSet.end(), [&c1](const cv::Scalar& a, const cv::Scalar& b) {
                    return distance(c1, a) < distance(c1, b);
                });
            distances.push_back(distance(c1, min));
        }
        auto max = std::max_element(distances.begin(), distances.end());
        return colorCandidates[std::distance(distances.begin(), max)];
    }

    static cv::Scalar hsv2rgb(const cv::Scalar& hsvColor) {
        cv::Mat rgb;
        cv::Mat hsv(1, 1, CV_8UC3, hsvColor);
        cv::cvtColor(hsv, rgb, cv::COLOR_HSV2RGB);
        return cv::Scalar(rgb.data[0], rgb.data[1], rgb.data[2]);
    }

public:
    explicit ColorPalette(size_t n) {
        palette.reserve(n);
        std::vector<cv::Scalar> hsvColors(1, {1., 1., 1.});
        std::vector<cv::Scalar> colorCandidates;
        size_t numCandidates = 100;

        hsvColors.reserve(n);
        colorCandidates.resize(numCandidates);
        for (size_t i = 1; i < n; ++i) {
            std::generate(colorCandidates.begin(), colorCandidates.end(), []() {
                return cv::Scalar{getRandom(), getRandom(0.8, 1.0), getRandom(0.5, 1.0)};
            });
            hsvColors.push_back(maxMinDistance(hsvColors, colorCandidates));
        }

        for (auto& hsv : hsvColors) {
            // Convert to OpenCV HSV format
            hsv[0] *= 179;
            hsv[1] *= 255;
            hsv[2] *= 255;

            palette.push_back(hsv2rgb(hsv));
        }
    }

    const cv::Scalar& operator[](size_t index) const {
        return palette[index % palette.size()];
    }
};