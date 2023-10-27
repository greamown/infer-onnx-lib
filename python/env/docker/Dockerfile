# Dockerfile to run ONNXRuntime with CUDA, CUDNN integration
FROM python:3.10
# ------------------------------------------------------------------
WORKDIR /workspace
COPY ["./requirements.sh", "/workspace/" ]
USER root
# ------------------------------------------------------------------
ENV DEBIAN_FRONTEND noninteractive  
RUN chmod +x ./requirements.sh \
    && ./requirements.sh \
    && rm ./*.sh 
# ------------------------------------------------------------------
# 寫入進入W
ENTRYPOINT [ "/bin/bash", "-c" ]