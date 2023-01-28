
# Map-Rendering Engine

Made for OpenVic2 Hackathon.

## Controls
- Escape to exit application.
- Mouse to rotate camera (avoid looking directly up or down as the camera will flip upside down).
- WASD for movement relative to the camera's facing direction and the up vector (0,1,0).
- Space and left shift for movement up/down along the up vector.
- Left control to increase speed by 5x.

## Build Instructions
Can be built with MSVC or MinGW:
```
git submodule update --init --recursive
cmake -S . -B build
cd build
cmake --build .
```
