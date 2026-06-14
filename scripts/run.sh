#!/usr/bin/env bash
set -euo pipefail

IMAGE_NAME="${IMAGE_NAME:-team01-farm-sim:final}"
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

export MSYS_NO_PATHCONV=1
export MSYS2_ARG_CONV_EXCL="*"

if ! docker image inspect "${IMAGE_NAME}" >/dev/null 2>&1; then
    bash "${ROOT_DIR}/docker/docker_build.sh"
fi

docker run -it --rm \
  --platform linux/amd64 \
  -v "${ROOT_DIR}:/workspace" \
  -w /workspace \
  --name farm-sim-container \
  "${IMAGE_NAME}" \
  bash -lc "bash scripts/build.sh && ./build/main"
