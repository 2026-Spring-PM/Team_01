cmake_minimum_required(VERSION 3.16)
project(FarmSim LANGUAGES CXX)

# C++ 표준
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 경고 옵션
add_compile_options(-Wall -Wextra)

# src/ 및 하위 폴더의 모든 .cpp 파일 자동 수집
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 실행파일 생성
add_executable(main ${SOURCES})
target_include_directories(main PRIVATE src)