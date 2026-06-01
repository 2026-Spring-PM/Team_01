#!/usr/bin/env bash
set -e

rm -rf build
mkdir -p build

# CMake로 빌드 (GUI ON)
cmake -S . -B build -DENABLE_GUI=ON
cmake --build build

echo ""
echo "[OK] Build successful: build/main (GUI mode)"