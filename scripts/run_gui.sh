#!/usr/bin/env bash
set -e

# Docker에서 SDL2 GUI 창까지 띄우는 예시입니다.
# Linux: 실행 전 xhost +local:docker 필요
# Windows: VcXsrv/Xming 실행 후 DISPLAY 설정 필요
IMAGE_NAME="chwoong/team_00_project:0.1.0"

export MSYS_NO_PATHCONV=1
export MSYS2_ARG_CONV_EXCL="*"

docker run -it --rm \
  --platform linux/amd64 \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v "/$(pwd):/workspace" \
  -w //workspace \
  --name farm-sim-gui-container \
  "$IMAGE_NAME" \
  bash -c "apt-get update && apt-get install -y cmake pkg-config libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev fonts-noto-cjk && ENABLE_GUI=ON bash scripts/build.sh && ./build/main"
