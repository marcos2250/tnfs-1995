# tnfs-1995

Recreation and analysis of the simulation of The Need for Speed (1995): car physics, AI, collisions and camera.

The code is reverse engineered from the DOS, PSX, 3DO and Win95 (SE) binaries and rewritten by hand in C, as close to the
original as possible. Where the platforms differ, each version is kept in the code and marked. The Ghidra projects used
for the analysis are in `ghidra-tnfs/`.

What it is:
* a simulation core: car physics, engine and gearbox, tyres, fence and car-to-car collisions, the crash model, AI
  racers, traffic and police, and replays in the game's own `.RPL` format;
* a small SDL2/OpenGL wireframe viewer to drive and debug it.

What it is not:
* a complete game: there are no textures, car models, sound, menus or HUD. Front ends provide those and use this code
  as their simulation.

Live example: [TNFS Web](https://tnfsw.guraklgames.com/) is a browser remake of the game that uses this project as its
physics and AI backend.

Also check my *3DO* version **tnfs-1994** total remake project at: https://github.com/marcos2250/tnfs-1994

## Features
* 2D car driving simulation, with a 2-wheel tire grip model;
* 3D box rigid body collision physics;
* Engine, clutch and transmission simulation;
* AI opponent, traffic and police cars;
* Now supports loading TNFSSE "TRI" track files, or it will auto generate a random track, if not supplied;
* Loads the car's "PBS" physics specs (unpacked);
* Replay recording and playback;
* Fixed point math;
* No assets required;
* Basic wireframe 3D graphics; 
* SDL2/OpenGL 3D in pure C.

## How to play
1. clone/download repo;
2. install `libsdl2-dev`;
3. `make`
4. run `./build/tnfs [<optional_track_file.tri> [<optional_car_file.pbs>]]`

The game's `CARSPECS/*.PBS` files are QFS packed: unpack them first (for example with nfs-resources-converter).

Use the keys to drive:
* Up/Down - Accel and brake
* Left/Right - Steer
* Space - Handbrake
* A/Z - Change gears
* C - Change camera
* D - Crash opponent's car
* R - Reset car
* F1 - Turn ABS on/off
* F2 - Turn TCS on/off
* F3 - Change traction (RWD/AWD/FWD)
* F4 - Change Auto/Manual transmission mode
* F5 - Change cheat modes/easter eggs: 1. Crashing Cars (3DO); 2. Rally Mode (PSX)
* F6/F7/F8 - Replay the last 10 seconds / the whole race / the highlights; F9 - Back to the race
* P - Pause or step the replay; 1-4 - Replay speed; [ / ] - Rewind / fast forward; Home/End - Start / end of the replay

A gamepad also works: left stick steers, triggers are throttle and brake, shoulders change gears, A is the handbrake.

## Resources/Tools used
* https://ghidra-sre.org/
* https://github.com/grumpycoders/pcsx-redux
* https://github.com/AndyGura/nfs-resources-converter
* https://www.cmls.polytechnique.fr/cmat/auroux/nfs/index.us.html
