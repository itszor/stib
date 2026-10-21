#include "sat.h"
#include "fsound.h"
#include "fade.h"
#include "play.h"
#include "between.h"

#include <SDL/SDL.h>

/* Globals */

const ipoint alt1[6]={1,0, -1,0, -1,1, 0,1, -1,-1, 0,-1};
const ipoint alt2[6]={1,0, -1,0, 1,1, 0,1, 0,-1, 1,-1};

/*gamestatedata gamestate, pendingstate;*/
int pendgstimeout;

spriteareasdata spriteareas;
sdlsurfacedata sdlsurfaces;

soundsdata sounds;
tadata tainfo;
downdata down;
tcelldata tcell;
dropdata drop;
cell array[playgrid_xsize][playgrid_ysize];
SDL_Surface *backdroptile;
int types[16], numtypes;
fslide danger;
slide score;
hst scoretab;
levelinfo lev;
nextshapeinfo nxshp;
int shuffletimeindex=0;
int returntodesk;
int burningstate;

stibshooterdata stibshooter;
playfielddata playfield;
gamedata game;
delaysfxdata bigfanfare;

int *sintab, *costab;
int ws_desktopquit;

int mouse_xpos, mouse_ypos, mouse_buttons;

SDL_Surface* vidscreen, *backscreen;

int windowzoom;
