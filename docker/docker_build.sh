#!/usr/bin/env bash
set -euo pipefail

IMAGE_NAME="${IMAGE_NAME:-team01-farm-sim:final}"
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

docker build -t "${IMAGE_NAME}" -f "${ROOT_DIR}/docker/Dockerfile" "${ROOT_DIR}"

echo "[OK] Docker image built: ${IMAGE_NAME}"
