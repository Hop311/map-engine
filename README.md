
# Map-Rendering Engine

Made for OpenVic2 Hackathon.

## Controls
- Escape to exit application.
- Mouse to rotate camera (avoid looking directly up or down as the camera will flip upside down).
- WASD for movement relative to the camera's facing direction and the up vector (0,1,0).
- Space and left shift for movement up/down along the up vector.
- Left control to increase movement speed.

## Build Instructions
Before building, make sure the macro `MAP_DIR` at the top of `Graphics.cpp` is the correct path to your Vic2 install map folder (or really any folder containing `terrain/colormap.dds`, `terrain.bmp` and `terrain/texturesheet.tga`).
The program can be built with MSVC or MinGW:
```
git clone https://github.com/Hop311/map-engine.git
cd map-engine
git submodule update --init --recursive
cmake -S . -B build
cmake --build build
```
The script `build.sh` can also be used. Either method, if successful, the program will be located at `./build/map-engine`.
