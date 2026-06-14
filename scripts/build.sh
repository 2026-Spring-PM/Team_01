#!/usr/bin/env bash
set -euo pipefail

# Default final-submission build is terminal mode for maximum compatibility.
# Use ENABLE_GUI=ON bash scripts/build.sh to build the SDL2 GUI version.
ENABLE_GUI_VALUE="${ENABLE_GUI:-OFF}"

cmake -S . -B build -DENABLE_GUI="${ENABLE_GUI_VALUE}"
cmake --build build

echo ""
echo "[OK] Build successful: build/main (ENABLE_GUI=${ENABLE_GUI_VALUE})"
