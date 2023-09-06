mkdir build
cd build

gcc -I/usr/include/SDL2 -include/usr/include/SDL2/SDL.h -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP "../tnfs_math.c" "../tnfs_base.c" "../tnfs_collision.c" "../tnfs_fiziks.c" "../tnfs_sdl_main.c"

gcc -L/usr/include/SDL2 -L/usr/include/GL ./tnfs_base.o ./tnfs_collision.o ./tnfs_fiziks.o ./tnfs_math.o ./tnfs_sdl_main.o  -lSDL2 -lSDL2main -lm -lGL -lGLU
