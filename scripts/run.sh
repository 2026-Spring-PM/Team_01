#!/bin/bash
# 호스트에서 실행하는 Docker 진입 스크립트
# Docker 박스를 띄우고 build.sh로 빌드 후 main 실행

set -e

IMAGE_NAME="chwoong/team_00_project:0.1.0"

# Windows Git Bash의 경로 자동 변환 비활성화
export MSYS_NO_PATHCONV=1
export MSYS2_ARG_CONV_EXCL="*"

docker run -it --rm \
  --platform linux/amd64 \
  -v "/$(pwd):/workspace" \
  -w //workspace \
  --name farm-sim-container \
  "$IMAGE_NAME" \
  bash -c "bash scripts/build.sh && ./build/main"