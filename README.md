# Team 01 - Farm Village Simulator

Spring 2026 Programming Methodology Team Project

## Requirements

### Terminal version
- Docker Desktop or local C++17/CMake environment

### GUI version
- `SDL2`
- `SDL2_image`
- `SDL2_ttf`
- On Ubuntu/WSL:

```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev fonts-noto-cjk
```

## Local Build & Run

### GUI version

```bash
rm -rf build
cmake -S . -B build -DENABLE_GUI=ON
cmake --build build
./build/main
```

### Terminal version

```bash
rm -rf build
cmake -S . -B build -DENABLE_GUI=OFF
cmake --build build
./build/main
```

## Docker Build & Run

### Terminal version

```bash
bash scripts/run.sh
```

### GUI version

Linux X11 example:

```bash
xhost +local:docker
bash scripts/run_gui.sh
```

Windows users need an X server such as VcXsrv or Xming before running Docker GUI.

## Project Structure

- `src/` — C++ source code
- `assets/` — PNG GUI sprites
- `scripts/` — build/run scripts
- `docs/` — reports and design docs

## GUI Notes

The SDL2 GUI now covers the main map, player movement, HUD, event icons, shop, farming choices, livestock choices, building construction, kitchen, workshop, fishing minigame, blackjack-style mining minigame, inventory, and quit confirmation.

Detailed menus use SDL popup buttons through `SDL_ShowMessageBox`, so they no longer require typing into the terminal while GUI mode is active.
