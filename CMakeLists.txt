cmake_minimum_required(VERSION 3.16)
project(FarmSim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra)

option(ENABLE_GUI "Build SDL2 PNG GUI" ON)

file(GLOB_RECURSE SOURCES "src/*.cpp")

if(ENABLE_GUI)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
    pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
    message(STATUS "GUI build enabled: SDL2=${SDL2_VERSION}, SDL2_image=${SDL2_IMAGE_VERSION}, SDL2_ttf=${SDL2_TTF_VERSION}")
else()
    message(STATUS "GUI build disabled: terminal mode only")
    list(FILTER SOURCES EXCLUDE REGEX ".*/GuiRenderer\\.cpp$")
endif()

add_executable(main ${SOURCES})
target_include_directories(main PRIVATE src)

if(ENABLE_GUI)
    target_compile_definitions(main PRIVATE FARM_GUI)
    target_include_directories(main PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})
    target_link_directories(main PRIVATE ${SDL2_LIBRARY_DIRS} ${SDL2_IMAGE_LIBRARY_DIRS} ${SDL2_TTF_LIBRARY_DIRS})
    target_link_libraries(main PRIVATE ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES})
    target_compile_options(main PRIVATE ${SDL2_CFLAGS_OTHER} ${SDL2_IMAGE_CFLAGS_OTHER} ${SDL2_TTF_CFLAGS_OTHER})
endif()
