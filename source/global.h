/* Globals */

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

extern const ipoint alt1[6];
extern const ipoint alt2[6];

extern int pendgstimeout;

extern spriteareasdata spriteareas;
extern soundsdata sounds;
extern tadata tainfo;
extern downdata down;
extern tcelldata tcell;
extern dropdata drop;
extern cell array[playgrid_xsize][playgrid_ysize];
extern int *backdroptile;
extern int types[16], numtypes;
extern slide danger;
extern slide score;
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

extern char *global_reguser;
