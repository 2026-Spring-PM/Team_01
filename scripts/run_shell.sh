#!/bin/bash
# 디버깅용: Docker 박스 안의 셸로 진입

export MSYS_NO_PATHCONV=1
export MSYS2_ARG_CONV_EXCL="*"

IMAGE_NAME="chwoong/team_00_project:0.1.0"

docker run -it --rm \
  --platform linux/amd64 \
  -v "/$(pwd):/workspace" \
  -w //workspace \
  --name farm-sim-shell \
  "$IMAGE_NAME" \
  /bin/bash