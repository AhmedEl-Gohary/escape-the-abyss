# Escape the Abyss - 3D Horror Escape Room Game

## Game Overview
**Escape the Abyss** is an immersive 3D horror escape room game where players are trapped in a creepy haunted house and must solve puzzles, collect items, and fight off monsters to escape. The game is divided into two main environments, each offering unique challenges and obstacles that require quick thinking and strategy. Your primary goal is to escape the eerie house and survive the dangerous woods filled with monsters, all while racing against the clock. 

### Storyline:
You begin trapped in a dark, mysterious room within a haunted house. Armed with limited visibility, you must search for keys and items that help you unlock doors, solve puzzles, and find your way out. Once you escape the house, you are thrust into the eerie woods where monsters lurk and relentlessly pursue you. Fight back with swords and torches to survive the dangerous creatures and make your escape before time runs out.

## Features

### 1. **First Environment - The Haunted House**:
- **Setting**: Start in a dark room with limited visibility and a mysterious atmosphere.
- **Goal**: Solve puzzles, unlock doors, and escape the haunted house.
- **Obstacles**: Walls and locked doors prevent progress.
- **Collectibles**: Keys and flashlights to aid navigation.
- **Gameplay**: Search for items, solve puzzles, and progress deeper into the house.

### 2. **Second Environment - The Woods**:
- **Setting**: After escaping the house, you are thrust into dark woods, where danger lurks around every corner.
- **Goal**: Survive and escape by killing monsters.
- **Obstacles**: Monsters that relentlessly pursue you, reducing your health upon collision.
- **Collectibles**: Swords and torches to defend against monsters.
- **Gameplay**: Fight off monsters and manage your health to stay alive and complete the escape.

### 3. **Main Character**:
- **Models**: The player character is a human, and the monsters are various terrifying creatures.
- **Camera Views**:
  - **First-person view**: The camera simulates the player's eye, providing an immersive experience.
  - **Third-person view**: The camera follows the player from behind and slightly above, showing the upper part of the character.
  
### 4. **Gameplay Mechanics**:
- **Controls**:
  - **Movement**: WASD for movement.
  - **Pick up items**: E to pick up objects.
  - **Jump**: SPACE to jump.
  - **Interact with objects**: Left mouse button to turn on the flashlight or swing a sword.
  - **Camera View**: C to switch between first-person and third-person views.
  - **Menu Selection**: Mouse buttons for navigating menus.

- **Score**: The score is based on the time taken to escape, with faster completion times yielding better scores.
- **Health**: The player has 3 lives in the woods environment. Each time a monster hits you, you lose one life.

### 5. **Sound and Animation**:
- **Animations**:
  - Walking animation for the player (with faint footsteps).
  - Animation when items are picked up.
  - A hit animation for the player or monsters (step back or red flash).
  - Door opening sound and animation.
  
- **Sound Effects**:
  - Faint footsteps while walking.
  - Different sounds for each collectible item.
  - Hit sounds when the player or monsters are damaged.
  - Door opening sounds.

### 6. **Lighting**:
- **Flickering Light**: A light bulb flickers and dims/brightens as time passes, adding to the suspense and atmosphere.
- **Flashlight**: The player's flashlight acts as a moving light source, illuminating the environment.

## Installation Instructions
To run **Escape the Abyss**, you'll need the following libraries. You can install them using the following commands:

```bash
sudo apt update
sudo apt install build-essential cmake freeglut3-dev libglew-dev libglm-dev libassimp-dev
sudo apt-get install libjpeg-dev libpng-dev libtiff-dev
