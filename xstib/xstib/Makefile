# Project:   Game

CC =	gcc
LD =	ld

# Toolflags:
CFLAGS = -c -O2 -g `sdl-config --cflags`
LDFLAGS = -g
LIBS = -lm `sdl-config --libs` -lSDL_mixer
INCLUDE =

OBJECTS =	between.o global.o levels.o play.o sat.o userint.o flexisound.o fade.o

SOURCE =	between.c global.c levels.c play.c sat.c userint.c flexisound.c fade.c

.PHONY:	clean

all: xstib

clean:
	rm -f *.o xstib

xstib:	$(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

%.o:	%.c
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@

.depend:	$(SOURCE)
	$(CC) -MM $(INCLUDE) $(SOURCE) > .depend

include .depend
