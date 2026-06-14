# Team 01 - Farm Village Simulator

Spring 2026 Programming Methodology Team Project

## Quick Start

The recommended final-submission command is:

```bash
bash scripts/run.sh
```

This builds the Docker image if needed, compiles the game inside Docker, and launches the terminal version immediately.

## Docker Build & Run

### Terminal Version

```bash
bash scripts/run.sh
```

This command uses `docker/Dockerfile` and the image name `team01-farm-sim:final`.

To build the image manually:

```bash
bash docker/docker_build.sh
```

### GUI Version

GUI mode requires a host display server because the game uses SDL2.

Linux X11 example:

```bash
xhost +local:docker
bash scripts/run_gui.sh
```

Windows users need an X server such as VcXsrv or Xming and a valid `DISPLAY` setting before running GUI mode through Docker.

## Local Build & Run

### Terminal Version

```bash
rm -rf build
cmake -S . -B build -DENABLE_GUI=OFF
cmake --build build
./build/main
```

or:

```bash
bash scripts/build.sh
./build/main
```

### GUI Version

Install SDL2 development packages first.

Ubuntu/WSL:

```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev fonts-noto-cjk
```

Then build and run:

```bash
rm -rf build
cmake -S . -B build -DENABLE_GUI=ON
cmake --build build
./build/main
```

or:

```bash
ENABLE_GUI=ON bash scripts/build.sh
./build/main
```

## Project Structure

- `src/`: C++ source and header files
- `assets/`: PNG GUI sprites and game assets
- `scripts/`: build and run scripts
- `docker/`: Dockerfile and Docker image build script
- `docs/`: final reports, if included

## Game Overview

Farm Village Simulator is a farm management game with agriculture, livestock, marketplace trading, construction, crafting, level progression, random events, mini-games, and optional SDL2 GUI mode.

Core features include crop growth, animal production, kitchen cooking, monster food crafting, shop trust, tool upgrades, fishing, blackjack-style mining, storage expansion, skill trees, random events, and a tutorial.
