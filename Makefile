CC=gcc
CFLAGS=-O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP
LDFLAGS=-L/usr/include/SDL2 -L/usr/include/GL 
LIBS=-lSDL2 -lSDL2main -lm -lGL -lGLU
INCL=-I/usr/include/SDL2 -include/usr/include/SDL2/SDL.h

O=build

OBJS=$(O)/tnfs_base.o $(O)/tnfs_collision.o $(O)/tnfs_fiziks.o $(O)/tnfs_math.o $(O)/tnfs_sdl_main.o

all:	mkdir $(O)/tnfs 	
		
clean:
	rm -f $(O)/*

mkdir: 
	mkdir -p build

$(O)/tnfs:	$(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(O)/tnfs $(LIBS)

$(O)/%.o: %.c
	$(CC) $(INCL) $(CFLAGS) -c $< -o $@

