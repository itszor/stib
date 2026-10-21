/* Globals */

#include <SDL/SDL.h>

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#include <SDL/SDL.h>

extern const ipoint alt1[6];
extern const ipoint alt2[6];

/*extern gamestatedata gamestate, pendingstate;*/
extern int pendgstimeout;

extern spriteareasdata spriteareas;
extern sdlsurfacedata sdlsurfaces;

extern soundsdata sounds;
extern tadata tainfo;
extern downdata down;
extern tcelldata tcell;
extern dropdata drop;
extern cell array[playgrid_xsize][playgrid_ysize];
extern SDL_Surface *backdroptile;
extern int types[16], numtypes;
extern slide score;
extern fslide danger;
extern hst scoretab;
extern levelinfo lev;
extern nextshapeinfo nxshp;
extern int shuffletimeindex;
extern int returntodesk;
extern int burningstate;

extern stibshooterdata stibshooter;
extern playfielddata playfield;
extern gamedata game;
extern delaysfxdata bigfanfare;

extern int *sintab, *costab;

extern int ws_desktopquit;

extern char* global_reguser;

extern SDL_Surface* vidscreen, *backscreen;

extern int windowzoom;
