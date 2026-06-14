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

**macOS:**

```bash
brew install cmake
rm -rf build
cmake -S . -B build -DENABLE_GUI=OFF
cmake --build build
./build/main
```

**Ubuntu/WSL:**

```bash
sudo apt update
sudo apt install -y build-essential cmake
rm -rf build
cmake -S . -B build -DENABLE_GUI=OFF
cmake --build build
./build/main
```

or (any platform):

```bash
bash scripts/build.sh
./build/main
```

### GUI Version

#### macOS (Homebrew)

SDL2 uses the native macOS display (Cocoa). No XQuartz or Docker needed.

```bash
brew install cmake sdl2 sdl2_image sdl2_ttf pkg-config
ENABLE_GUI=ON bash scripts/build.sh
./build/main
```

or equivalently:

```bash
bash scripts/run_gui.sh
```

`scripts/run_gui.sh` automatically detects macOS and runs a native local build instead of Docker.

#### Ubuntu/WSL

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

## Controls

### Terminal Mode

| Key | Action |
|---|---|
| `W / A / S / D` | Move the player |
| `J` | Interact with the current tile or the building door |
| `E` | Open inventory |
| `P` | Open skill tree |
| `Q` | Quit the game |
| `T` | Quick interaction with a nearby animal inside the barn |

In terminal mode, most systems are accessed by moving to the correct location and pressing `J`.
For example, the shop, bed, kitchen, workshop, storage, barn, fishing area, and mine are all used through tile interaction.

### GUI Mode

| Key / Input | Action |
|---|---|
| `W / A / S / D` | Move the player |
| `J / Enter / Space` | Interact with the current tile or selected menu item |
| `E` | Open inventory |
| `P` | Open skill tree |
| `Q / Esc` | Quit or close certain panels |
| Mouse | Select buttons and menu items in GUI panels |
| `W / S` | Move menu selection in supported GUI menus |
| `Enter / Space` | Confirm selected GUI menu item |

GUI mode provides visual panels, sprites, item icons, pop-up messages, and mouse-based menu interaction.

## Core Gameplay Loop

The player manages a farm for 100 in-game days. Each day, the player can plant and harvest crops, raise livestock, cook food, upgrade tools, trade items at the shop, explore mini-games, and improve skills through level progression.

A typical day follows this loop:

1. Plant crops or harvest mature crops.
2. Take care of livestock in the barn and collect animal products.
3. Sell products or buy seeds and animals at the shop.
4. Use the kitchen, workshop, and storage to expand the farm economy.
5. Play fishing or mining mini-games to obtain valuable resources.
6. Sleep at the bed to advance to the next day.
7. React to random events such as drought, cold weather, wildlife, or special lucky days.

The goal is to build a stronger and more profitable farm by combining farming, crafting, trading, skill growth, and mini-game rewards.

## Core Features

### Agriculture System

The game includes a crop system where the player can plant seeds, wait for crops to grow over several days, and harvest them for items and experience. Different crops have different growth behavior, and apple trees can produce fruit repeatedly without disappearing after harvest.

During summer drought events, crops may die if they are not watered, so the player must manage farm risk as the days progress.

### Livestock and Barn System

Livestock is managed inside the barn. Animals can grow, move around the barn map, and produce items such as eggs, milk, or meat depending on the animal type.

Inside the barn, the player can use `T` for quick interaction with a nearby animal or `J` to open the barn management menu. This separates livestock gameplay from the main farm map and gives animals their own dedicated space.

### Marketplace and Economy

The shop allows the player to buy seeds, animals, and other useful items, while also selling crops, animal products, fish, ores, cooked food, and monster food.

The game includes a shop trust system. Selling strange or risky food can produce high profit, but failed sales may reduce shop trust and limit access to higher-grade items.

### Construction, Kitchen, Workshop, and Storage

The farm includes construction and facility systems.

- The kitchen allows the player to cook recipes and create monster food by combining ingredients.
- The workshop allows tool upgrades for the hoe, fishing rod, and pickaxe.
- The storage system expands the player's inventory capacity.
- Buildings are accessed by standing in front of the correct door tile and pressing the interaction key.

These systems encourage the player to convert raw resources into higher-value items and long-term upgrades.

### Tool Upgrade System

The player can upgrade tools through the workshop.

- Hoe upgrades reduce stamina cost for farming.
- Fishing rod upgrades improve fishing efficiency.
- Pickaxe upgrades improve mining safety and information quality.

Tool upgrades make repeated actions more efficient and support long-term progression.

### Level and Skill Tree System

The player gains experience from farming, fishing, mining, livestock care, and other activities. When enough experience is gained, the player levels up and receives skill points.

The skill tree has three main paths:

- `STR`: improves stamina-related and mining-related performance.
- `INT`: improves production, crafting, and trading efficiency.
- `LUK`: improves lucky outcomes, rare rewards, and special event chances.

This allows players to develop different play styles depending on how they want to manage the farm.

### Random Event System

Random events can occur when a new day starts. These events make gameplay less predictable and require the player to adapt.

Examples include:

- Cold weather, which can reduce stamina.
- Summer drought, which threatens crops unless they are watered.
- Wildlife events, which can affect the farm unless blocked by certain skills.
- Starry night blessings, which provide a rare lucky-day effect.

### Fishing Mini-Game

Fishing is a timing-based mini-game. The player must stop the moving marker inside the target zone. Better timing can produce higher-quality results such as `Great` or `Perfect`.

Fishing provides fish and experience, and upgraded fishing rods can make the activity more efficient.

### Mining Mini-Game

Mining is a blackjack-style risk-and-reward mini-game. The player repeatedly chooses whether to mine again or escape with the current reward.

Each mining action increases collapse pressure. If the pressure becomes too high, the player may lose the accumulated reward. Better pickaxes provide better pressure hints or safer outcomes.

### Tutorial System

At the beginning of the game, the tutorial introduces the main controls and gameplay systems through the Seed Grandma NPC. The tutorial explains movement, farming, stamina, level progression, shop usage, cooking, tool upgrades, fishing, mining, livestock, and skills.
