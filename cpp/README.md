# Cpp:Inference using yolov7-object-detection of onnx
This is code is inference onnx model for yolov7-object-detection. (The nms is exist in model)
* [.run (Ubuntu/WLS2)](#run-ubuntu-wls2)
* [.exe (Windows)](#exe-windows)
* [.dll (Windows)](#dll-windows)

## Getting Started

## .run (Ubuntu/WLS2)
### Pre-requirements

- **GCC**
    ```shell
    sudo apt update
    sudo apt install build-essential -y
    ```
- **cmake**
    ```shell
    sudo apt update
    sudo apt install cmake -y
    ```
- **gflags**

    ```shell
    sudo apt-get install libgflags-dev
    ```
- **opencv-cpp**

    [Tutorial](https://docs.opencv.org/4.x/d7/d9f/tutorial_linux_install.html)
    ```shell
    sudo apt-get install libopencv-dev
    ```
- **onnxruntime**

    [Tutorial](https://onnxruntime.ai/)
    - Version: **onnxruntime-linux-x64-1.15.1**
    - Download to ./externals and extrac to ./externals

### Compile
```shell
cd cpp
cmake .
make
```

### Run
```shell
# video
./main -i ../data/mask.mp4 -m ../model/yolov7-tiny.onnx -t 0.5 
# image
./main -i ../data/mask.jpeg -m ../model/yolov7-tiny.onnx -t 0.5
# webcam
./main -i 0 -m ../model/yolov7-tiny.onnx -t 0.5
```

<details>
    <summary> Result
    </summary>
    <div align="center">
        <img width="80%" height="80%" src="../docs/cpp_result.png">
    </div>
</details>


## .exe (Windows)
### Pre-requirements

- **GCC**

    [Tutorial](https://sourceforge.net/projects/mingw-w64/)

- **cmake**

    [Tutorial](https://cmake.org/download)

- **gflags**

    [Tutorial](https://github.com/gflags/gflags)

- **opencv-cpp**

    [Tutorial](https://docs.opencv.org/4.x/dd/d6e/tutorial_windows_visual_studio_opencv.html)

- **onnxruntime**

    [Tutorial](https://onnxruntime.ai/)
    - Version: **onnxruntime-win-x64-1.15.1**
    - Download to ./externals and extrac to ./externals

### Compile
- Compile *CMakeLists.txt*
    ```shell
    mkdir build
    cd build/
    cmake ..
    ```
- Using **Visual studio** make Debug or Release .dll files. 

## .dll (Windows)

`This method is not yet completed.`

### Pre-requirements

- **GCC**

    [Tutorial](https://sourceforge.net/projects/mingw-w64/)

- **cmake**

    [Tutorial](https://cmake.org/download)

- **gflags**

    [Tutorial](https://github.com/gflags/gflags)

- **opencv-cpp**

    [Tutorial](https://docs.opencv.org/4.x/dd/d6e/tutorial_windows_visual_studio_opencv.html)

- **onnxruntime**

    [Tutorial](https://onnxruntime.ai/)
    - Version: **onnxruntime-win-x64-1.15.1**
    - Download to ./externals and extrac to ./externals

### Compile
- Change *CMakeLists_package.txt* to *CMakeLists.txt*.
- Compile *CMakeLists.txt*
    ```shell
    mkdir build
    cd build/
    cmake ..
    ```
- Using **Visual studio** make Debug or Release .dll files. 

## Reference
- Sample images from **Pexels**
    - https://www.pexels.com/
