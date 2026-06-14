#!/usr/bin/env bash
set -euo pipefail

# GUI mode requires a host display server.
# Linux: run `xhost +local:docker` before this script.
# Windows: run an X server such as VcXsrv/Xming and set DISPLAY.
IMAGE_NAME="${IMAGE_NAME:-team01-farm-sim:final}"
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

export MSYS_NO_PATHCONV=1
export MSYS2_ARG_CONV_EXCL="*"

if ! docker image inspect "${IMAGE_NAME}" >/dev/null 2>&1; then
    bash "${ROOT_DIR}/docker/docker_build.sh"
fi

docker run -it --rm \
  --platform linux/amd64 \
  -e DISPLAY="${DISPLAY:-host.docker.internal:0}" \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v "${ROOT_DIR}:/workspace" \
  -w /workspace \
  --name farm-sim-gui-container \
  "${IMAGE_NAME}" \
  bash -lc "ENABLE_GUI=ON bash scripts/build.sh && ./build/main"
