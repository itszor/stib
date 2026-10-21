#include <SDL/SDL.h>

typedef enum {
  byte,
  word,
  scale,
  texturemap,
  transparent
} plottype;

typedef enum {
  s16x16,
  s24x24,
  s32x32,
  s64x32
} sizeindex;

typedef enum {
  left,
  centre,
  right
} align;

typedef struct {
  int l,b,r,t;
} rect;

typedef struct {
  int x, y;
} ipoint;

typedef struct {
  float x, y;
} fpoint;

typedef struct {
  plottype type;
  sizeindex size;
  int *area;
  int number;
  ipoint pos;
  fpoint dir;
  fpoint grav;
  rect visible;
  union {
    int data[8];
    struct {
      ipoint vertex[4];
    } quad;
  } info;
} sprinfo;

typedef struct {
  int number;
  sprinfo *base;
  int *screenbase;
  int screenbuffer;
} satinfo;

typedef struct {
  int *sprite;
  int *screen;
  int xpos;
  int ypos;
  int length;
} bminfo;

typedef struct {
  int x;
  int y;
  struct {
    int offset;
    int scale;
  } tab[24];
} squiggleinfo;

extern satinfo sat_data;

extern void sat_screenmode(int);
extern int* sat_getscreenbase(void);
extern int  sat_swapbanks(int);
extern void sat_blit(SDL_Surface* from, SDL_Surface* to, int xpos, int ypos);
extern void sat_resetscreen(int*);
extern void sat_loadfile(char*, int*, int);
extern void sat_savefile(char*, int*, int);
extern void sat_mouseop(int*, int*, int*);
extern int  sat_timeop(void);
extern void sat_plot_32x32_noclip_unmask_word(int *sprite, int *screen, int x, int y);
extern void sat_plot_32x32_noclip_mask_word(int *sprite, int *screen, int x, int y);
extern void sat_plot_24x24_noclip_mask_word(int *sprite, int *screen, int x, int y);
extern void sat_plot_12x24_noclip_unmask_word(int *sprite, int *screen, int x, int y);
extern void sat_plot_24x24_noclip_unmask_word(int *sprite, int *screen, int x, int y);
extern void sat_plot_24x24_noclip_mask_byte(int *sprite, int *screen, int x, int y);
extern void sat_plot_24x24_clip_mask_byte(int *sprite, int *screen, int x, int y);
extern void sat_plot_16x16_noclip_mask_byte(int *sprite, int *screen, int x, int y);
extern void sat_plot_16x16_noclip_mask_word(int *sprite, int *screen, int x, int y);
extern void sat_plot_64x32_noclip_mask_word(int *sprite, int *screen, int x, int y);
extern void sat_plot_backdrop(SDL_Surface *sprite, SDL_Surface *screen);
extern void sat_plot_XxY_noclip_mask_byte(int *sprite, int *screen, int x, int y);
extern void sat_plot_barmiddle(SDL_Surface* sprite, SDL_Surface* screen,
  int xpos, int ypos, int length);
extern void sat_plot_squiggled(SDL_Surface *sprite, SDL_Surface *screen, 
  squiggleinfo *info);

extern satinfo sat_data;

/* Auto */

extern void sat_initialise(void)
;
extern void sat_loadsprites(int** area, char *name)
;

extern void sat_makesurfaces(int* area, SDL_Surface** into, int xsize,
  int ysize, int num, int transparent);

extern void sat_makearbsurfaces(int* area, SDL_Surface** into, int num,
  int transparent);

extern void sat_printf(SDL_Surface **font, SDL_Surface *screen, int x, int y, align ment, char *format, ...)
;
extern void vdustr(int a, ...)
;
extern void writec(int n)
;
extern void sat_stretch(SDL_Surface* dest, SDL_Surface* src, int factor);
extern void sat_refresh(SDL_Surface* dest, SDL_Surface* src);
