#!/bin/bash
# Docker 박스 안에서 실행되는 빌드 스크립트
# g++로 직접 컴파일하여 build/main 생성

set -e

# build 폴더 없으면 생성
mkdir -p build

# src/ 및 하위 폴더의 모든 .cpp 파일 수집
SOURCES=$(find src -name "*.cpp")

# g++로 컴파일
g++ -std=c++17 -Wall -Wextra -Isrc -o build/main $SOURCES

echo ""
echo "[OK] Build successful: build/main"