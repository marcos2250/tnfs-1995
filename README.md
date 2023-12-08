# tnfs-1995

Recreation and analysis of the physics engine from the classic racing game from 1995.

This project aims to reconstitute accurately as possible the physics formulas used in the game, decompiled from DOS, PSX and 3DO versions.

## Features
* 2D car driving simulation, with a 2-wheel tire grip model;
* 3D box rigid body collision physics;
* Engine, clutch and transmission simulation;
* Fixed point math for vectors and matrices;
* No assets required;
* SDL2/OpenGL 3D in pure C.

## Controls
Use the keys to drive around the parking lot area:
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
* F5 - Enable crash car easter egg
