# tnfs-1995

Recreation and analysis of the physics engine from the classic racing game from 1995.

This project aims to reconstitute/reverse engineer accurately as possible the physics formulas used in the game, decompiled from DOS, PSX and 3DO binaries, and manually rewritten to C.

## Features
* 2D car driving simulation, with a 2-wheel tire grip model;
* 3D box rigid body collision physics;
* Engine, clutch and transmission simulation;
* Now supports loading TNFSSE "TRI" track files, or it will auto generate a random track, if not supplied;
* Fixed point math;
* No assets required;
* Basic wireframe 3D graphics; 
* SDL2/OpenGL 3D in pure C.

## How to play
1. clone/download repo;
2. install `libsdl2-dev`;
3. `make`
4. run `./build/tnfs [<optional_track_file.tri>]`

Use the keys to drive:
* Up/Down - Accel and brake
* Left/Right - Steer
* Space - Handbrake
* A/Z - Change gears
* C - Change camera
* D - Flip over the car
* R - Reset car
* F1 - Turn ABS on/off
* F2 - Turn TCS on/off
* F3 - Change traction (RWD/AWD/FWD)
* F4 - Change Auto/Manual transmission mode
* F5 - Change cheat modes/easter eggs: 1. Crashing Cars (3DO); 2. Rally Mode (PSX)

## Resources/Tools used
* https://ghidra-sre.org/
* https://github.com/grumpycoders/pcsx-redux
* https://github.com/AndyGura/nfs-resources-converter
* https://www.cmls.polytechnique.fr/cmat/auroux/nfs/index.us.html
