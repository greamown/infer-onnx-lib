#!/bin/bash
# ---------------------------------------------------------
# Color ANIS
RED='\033[0;31m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
NC='\033[0m'

# ---------------------------------------------------------
# Set the default value of the getopts variable 
SERVER=false
MOUNT_CAMERA=""
SET_VERSION=""
COMMAND="bash"
WORKSPACE="/workspace"
DOCKER_NAME="yolov7-obj-onnx"
TAG_VER="latest"
DOCKER_IMAGE="yolov7-obj-onnx:${TAG_VER}"

# ---------------------------------------------------------
# Help
function help(){
	echo "-----------------------------------------------------------------------"
	echo "Run the YOLOV7-Seg/Cls-ONNX-GPU environment."
	echo
	echo "Syntax: scriptTemplate [-sh]"
	echo "options:"
	echo "s		Server mode for non vision user"
	echo "h		help."
	echo "-----------------------------------------------------------------------"
}
while getopts "sh" option; do
	case $option in
		s )
			SERVER=true
			;;
		h )
			help
			exit
			;;
		\? )
			help
			exit
			;;
		* )
			help
			exit
			;;
	esac
done

# ---------------------------------------------------------
sudo apt-get install -qy boxes > /dev/null 2>&1

# ---------------------------------------------------------
# SERVER or DESKTOP MODE
if [[ ${SERVER} = false ]];then
	mode="DESKTOP"
	SET_VERSION="-v /etc/localtime:/etc/localtime:ro -v /tmp/.x11-unix:/tmp/.x11-unix:rw -e DISPLAY=unix${DISPLAY}"
	# let display could connect by every device
	xhost + > /dev/null 2>&1
else
	mode="SERVER"
fi

# ---------------------------------------------------------
# Combine Camera option
all_cam=$(ls /dev/video*)
cam_arr=(${all_cam})

for cam_node in "${cam_arr[@]}"
do
    MOUNT_CAMERA="${MOUNT_CAMERA} --device ${cam_node}:${cam_node}"
done

# ---------------------------------------------------------
title="\n\
MODE:  ${mode}\n\
DOCKER: ${DOCKER_NAME} \n\
CAMERA:  $((${#cam_arr[@]}/2))\n\
COMMAND: ${COMMAND}"

# ---------------------------------------------------------
# Run container
docker_cmd="    docker run \
                --user root \
                --rm -it \
                --name ${DOCKER_NAME} \
                --net=host --ipc=host \
                -w ${WORKSPACE} \
                -v `pwd`:${WORKSPACE} \
                --privileged \
                -v /dev:/dev \
                ${MOUNT_CAMERA} \
                ${SET_VERSION} \
                ${DOCKER_IMAGE} \"${COMMAND}\"
            "

echo -e "${BLUE}"
echo -e "Command: ${docker_cmd}"
echo -e "${NC}"
bash -c "${docker_cmd}"