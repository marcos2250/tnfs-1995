# tnfs-1995

Recreation and analysis of the physics engine from the classic racing game from 1995.

This project aims to reconstitute accurately as possible the physics formulas used in the game, decompiled from DOS, PSX and 3DO binaries, and manually rewritten to C.

## Features
* 2D car driving simulation, with a 2-wheel "bicycle model" tire grip model;
* 3D box rigid body collision physics;
* Fixed point math for vectors and matrices;
* No assets or any extra file needed;
* SDL2/OpenGL in pure C.

## How to play
1. clone/download repo;
2. install `libsdl2`;
3. `make`
4. run `./build/tnfs`

Use the keys to drive around the parking lot area:
* Up/Down - Accel and brake
* Left/Right - Steer
* Space - Handbrake
* C - Change camera
* A - Forward drive
* Z - Reverse
* D - Flip over the car
* F1 - Turn ABS on/off
* F2 - Turn TCS on/off
* F3 - Change traction (RWD/AWD/FWD)
* F4 - Enable crash car easter egg
