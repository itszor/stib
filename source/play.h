#define playgrid_xsize 20
#define playgrid_ysize 16
#define rad(N) ((N)/57.2957795)
#define maskindex(B,N,X,Y) ((int*)((int)(B)+(N)*(X)*(Y)*2))
#define nomaskindex(B,N,X,Y) ((int*)((int)(B)+(N)*(X)*(Y)))
#define nomaskarb(B,N) ((int*)((int)(B)+((int*)(B))[(N)]))
#define cposx(X,Y) (lev.indent.x+(X)*lev.blocksize.x+((Y) & 1 ? lev.halfsizex : 0))
#define cposy(Y,S) (lev.indent.y+(Y)*lev.blocksize.y+(S))
#define shootspeed 3

#define buf1(S) (S)
#define buf2(S) ((int*)((int)(S)+81920))

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

typedef struct {
  int inuse;
  int angle;
} stibshooterdata;

typedef struct {
  unsigned int exploding : 1;
  unsigned int shuffling : 1;
  unsigned int burning   : 1;
  unsigned int detaching : 1;
} playfielddata;

typedef enum {
  alive,
  disconnected,
  soontodie,
  soontowin,
  died,
  won
} gamedata;

typedef struct {
  int delaysfx;
  int timeout;
} delaysfxdata;

typedef struct {
  int *background;
  int *blocks;
  int *shooter;
  int *shootbase;
  int *danger;
  int *gamefont;
  int *smaller;
  int *titles;
  int *lakeoffire;
  int *megahack;
} spriteareasdata;

typedef struct {
  int boing;
  int fanfare;
  int fanfare2;
  int fire;
  int lockin;
  int transport;
  int wedied;
  int whoosh;
} soundsdata;

typedef struct {
  char name[20];
  int score;
} hiscore;

typedef struct {
  hiscore entry[5][10];
} hst;

typedef union {
  unsigned int value;
  struct {
    unsigned int sprite    : 8;
    unsigned int full      : 1;
    unsigned int diff      : 1;
    unsigned int counted   : 1;
    unsigned int exploding : 1;
    unsigned int explodesp : 3;
    unsigned int burnt     : 1;
    unsigned int null	   : 16;
  } info;
} cell;

typedef struct {
  char sprite;
  fpoint pos;
  fpoint dir;
} dropcell;

typedef void (*sprite_plotter) (int *sprite, int *screen, int x, int y);

typedef struct {
  ipoint blocksize;
  ipoint indent;
  ipoint gridsize;
  int halfsizex;
  int leftedge, rightedge;
  int lookahead;
  int proximity;
  int startheight;
  int stibuse;
  int stibdec;
  int dudsin;
  int dangerinc;
  int sep;
  int *bricksprite;
  ipoint add;
  int *sparea;
  sprite_plotter byteplotter, wordplotter;
} levelinfo;

typedef struct {
  int current, future;
} slide;

typedef struct {
  int l,b,r,t;
  int cstate;
  char text[16];
} rect2;

typedef struct {
  int number;
  rect2 area[10];
} tadata;

typedef struct {
  slide pixels;
  int rows;
} downdata;

typedef struct {
  slide cell;
  fpoint movepos, movedir;
} tcelldata;

typedef struct {
  int number;
  dropcell drop[playgrid_xsize*playgrid_ysize];
} dropdata;

typedef struct {
  enum {
    stable,
    moving,
    pendingnew
  } state;
  int pos;
  slide cell;
} nextshapeinfo;

/* Auto */

extern void playagame(void)
;
extern void drawgamescreen(int *screen, int angle, int regulator)
;
extern void drawplayarea(int *screen, int fallingflag, int timeindex)
;
extern void drawdevice(int *screen, int angle)
;
extern void drawnext(int *screen, int pos)
;
extern void alterasprite(void)
;
extern int locktogrid(int x, int y, int *xgrid, int *ygrid)
;
extern int countnum(int startx, int starty)
;
extern void testarea(void)
;
extern void countunattached(void)
;
extern void dropunattached(void)
;
extern void evaluatetypes(void)
;
extern void initfire(int angle)
;
extern void dodetach(int *screen)
;
extern void doexplode(void)
;
extern void dofire(int *screen, int regulator)
;
extern void burntest(void)
;
extern void doburn(int regulator)
;
extern void updatestate(int regulator)
;
