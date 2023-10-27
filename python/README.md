# Python:Inference using yolov7-object-detection of onnx-cpu
This is code is inference onnx model for yolov7-object-detection. (The nms is exist in model)

## Getting Started

### Pre-requirements

### Docker (Ubuntu / WSL2)
Install **docker** before installing the docker container.

- [Tutorial-docker](https://docs.docker.com/engine/install/ubuntu/)

- **Add docker to sudo group** 
    - [Tutourial](https://docs.docker.com/engine/install/linux-postinstall/)
    ``` 
    sudo groupadd docker
    sudo usermod -aG docker $USER
    sudo chmod 777 /var/run/docker.sock
    ```

- Build docker
    ```shell
    cd ./python/env
    sudo chmod u+x ./docker/*.sh
    ./docker/build.sh
    ```
- Run container
    ```shell
    cd ./infer-yolov7-obj
    sudo chmod u+x ./python/env/docker/*.sh
    xhost +
    ./python/env/docker/run.sh
    ```

### Anaconda  (Ubuntu / Windows)
- Install **anaconda** for python env
    - [Tutorial-anaconda](https://docs.anaconda.com/free/anaconda/install/linux/)
- Python ENV

    **CPU**
    ```shell
    conda create -n yolov7-obj python=3.10 -c conda-forge -y
    conda activate yolov7-obj
    pip install -r ./docker/requirements_310.txt
    ```

### Run code
```shell
# Windows
python ./python/obj-yolov7-demo.py -m "D:\workspace\infer-yolov7-obj\model\yolov7-tiny.onnx" -i "D:\workspace\infer-yolov7-obj\data\mask.jpeg" -t 0.5

# Ubuntu
python3 ./python/obj-yolov7-demo.py -m /workspace/model/yolov7-tiny.onnx -i /workspace/data/mask.jpeg -t 0.5
```
<details>
    <summary> Result
    </summary>
    <div align="center">
        <img width="80%" height="80%" src="../docs/result.png">
    </div>
</details>

## Reference
- Sample images from **Pexels**
    - https://www.pexels.com/
