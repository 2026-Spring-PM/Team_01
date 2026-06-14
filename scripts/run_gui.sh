#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# macOS: Docker containers cannot access the macOS display system.
# Use a native local build instead (SDL2 uses Cocoa directly, no XQuartz needed).
if [[ "$(uname)" == "Darwin" ]]; then
    echo "[INFO] macOS detected: skipping Docker, using native local build."

    # Check that required SDL2 libraries are installed via Homebrew.
    MISSING=()
    for lib in sdl2 sdl2_image sdl2_ttf pkg-config; do
        if ! brew list --formula "$lib" &>/dev/null; then
            MISSING+=("$lib")
        fi
    done

    if [[ ${#MISSING[@]} -gt 0 ]]; then
        echo "[ERROR] Missing Homebrew packages: ${MISSING[*]}"
        echo "        Run: brew install ${MISSING[*]}"
        exit 1
    fi

    cd "${ROOT_DIR}"
    ENABLE_GUI=ON bash scripts/build.sh
    ./build/main
    exit 0
fi

# Linux / Windows (WSL): use Docker + X11 forwarding.
# Linux: run `xhost +local:docker` before this script.
# Windows: run an X server such as VcXsrv/Xming and set DISPLAY.
IMAGE_NAME="${IMAGE_NAME:-team01-farm-sim:final}"

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
