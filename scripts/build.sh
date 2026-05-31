#!/usr/bin/env bash
set -e

# 기본값은 터미널 모드입니다. GUI 빌드는 ENABLE_GUI=ON bash scripts/build.sh 로 실행하세요.
ENABLE_GUI_VALUE="${ENABLE_GUI:-OFF}"

cmake -S . -B build -DENABLE_GUI=${ENABLE_GUI_VALUE}
cmake --build build

echo ""
echo "[OK] Build successful: build/main (ENABLE_GUI=${ENABLE_GUI_VALUE})"
