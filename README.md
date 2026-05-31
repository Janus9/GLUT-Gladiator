# GLUT Gladiator

## Game Info

GLUT Gladiator is a procedurally generated top-down roguelike. 

The objective of the game is simple, follow the navigation arrow to the center of the cave, and destroy the boss!

Along the way, collect powerups, and fight ever increasing difficulty enemies. 

The game can be saved/loaded and is fully destructible

## Build Information

GLUT Gladiator is built on Windows using **Msys2**, and **GNU Make**.

### Required Tools

Before building, make sure the following are installed and available from your terminal:

- 64-bit msys64/ucrt64 g++
- msys64/ucrt64 mingw32-make
- OpenGL: Glew, Freeglut
- Sound Engine **TODO**

You can verify the tools with:

```bash
g++ --version
 - (Needs to be at least V16.0.0)
mingw32-make --version
 - GNU Make 4.4.1
```

There are included (but required) header libraries located inside of ``common/include``:
 - stb_image (Image loader) https://github.com/nothings/stb/blob/master/stb_image.h
 - toml.hpp (TOML Configuration Parser) https://github.com/toml-lang/toml

### How To

To build, simply run:
 - ```make``` - Build application in debugging mode
 - ```make debug``` - Build application in debugging mode
 - ```make release``` - Build application in release mode
 - ```make clean``` - Clean application build files (recommended for odd build behaviors)
 - ```make publish PUB_DIR="publish"``` - Builds the application in release mode and packages the application into the provided **PUB_DIR** folder

# Download Information

Precompiled versions of the game are available via the **Releases** tab on the right hand side. 

To download, simply click on the version you wish to download, then download the top file (not the source code)

# Contributions

- **Frank Bernal**: (Audio and Characters, Windows features) 
  - Sourcing audio and enemy sprites. 
  - Character, Music, and menu sound effects. 
  - Traveling enemies. 
  - Seamless fullscreen/windowed mode transition. 
  - Active/Inactive state when window is minimized or not active.
  - Debugging
  
- **Joshua Bayt**: (Lead Software Engineer and Project Lead) 
  - World generation algorithm and world structure
  - Save/Load functionality
  - Artwork for tiles and turret sprites
  - Shader implemenation + lighting
  - Particle engine + Bullet system
  
- **Daniel Remington**: (Assistant Programmer and Menu Designer)
  - Helped to design menu system
  - Helped with the scene manager
  - Created the menu artwork and structure
  - Expanded on In-Game Level Editor
  - Made a HUD display for Editor
  - Assisted with image clarity for level editor

