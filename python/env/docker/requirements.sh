#!/bin/bash
# ---------------------------------------------------------
# Color ANIS
RED='\033[1;31m'
BLUE='\033[1;34m'
YELLOW='\033[1;33m'
GREEN='\033[1;32m'
NC='\033[0m'

function printstr(){
    echo -e "${BLUE}"
    echo $1
    echo -e "${NC}"
}

# Initial
printstr "$(date +"%T") Initialize ... "
apt-get update -qqy
apt-get install -y --no-install-recommends libstdc++6 ca-certificates \
        python3-setuptools python3-wheel python3-pip unattended-upgrades \
        && unattended-upgrade

ROOT=`pwd`
echo "Workspace is ${ROOT}"

# OpenCV
printstr "$(date +"%T") Install OpenCV " 
apt-get install -qqy ffmpeg libsm6 libxext6 #> /dev/null 2>&1
apt-get install -y build-essential libopencv-dev
pip3 install "opencv-python>=4.1.1" #> /dev/null 2>&1

# Onnxruntime
printstr "$(date +"%T") Pip install onnxruntime torch " 
pip install "numpy>=1.18.5,<1.24.0"
pip install onnxruntime==1.15.0
pip install "torch>=1.7.0,!=1.12.0"
pip install "torchvision>=0.8.1,!=0.13.0"

printstr "Done"