#include "sat.h"
#include "fsound.h"
#include "fade.h"
#include "play.h"
#include "between.h"

/* Globals */

const ipoint alt1[6]={1,0, -1,0, -1,1, 0,1, -1,-1, 0,-1};
const ipoint alt2[6]={1,0, -1,0, 1,1, 0,1, 0,-1, 1,-1};

int pendgstimeout;

spriteareasdata spriteareas;
soundsdata sounds;
tadata tainfo;
downdata down;
tcelldata tcell;
dropdata drop;
cell array[playgrid_xsize][playgrid_ysize];
int *backdroptile;
int types[16], numtypes;
slide danger;
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
