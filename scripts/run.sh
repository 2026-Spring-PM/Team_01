#!/usr/bin/env bash
set -e

# 기존 과제 Docker 이미지에서 터미널 모드로 실행합니다.
IMAGE_NAME="ryanwj1017/team01_farm_gui:0.1.0"

export MSYS_NO_PATHCONV=1
export MSYS2_ARG_CONV_EXCL="*"

docker run -it --rm \
    --platform linux/amd64 \
    -v "/$(pwd):/workspace" \
    -w //workspace \
    --name farm-sim-container \
    "$IMAGE_NAME" \
    bash -c "bash scripts/build.sh && ./build/main"
