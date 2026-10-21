/* sat - sprites and things (the assembler: s.satcode; and c.sat) */

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

extern void sat_screenmode(int);
extern int* sat_getscreenbase(void);
extern int  sat_swapbanks(int);
extern void sat_resetscreen(int*);
extern void sat_loadfile(char*, int*);
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
extern void sat_plot_backdrop(int *sprite, int *screen);
extern void sat_plot_XxY_noclip_mask_byte(int *sprite, int *screen, int x, int y);
extern void sat_plot_barmiddle(bminfo *info);
extern void sat_plot_squiggled(int *sprite, int *screen, squiggleinfo *info);

extern satinfo sat_data;

/* Auto */

extern void sat_initialise(void)
;
extern void sat_loadsprites(int** area, char *name)
;
extern void sat_printf(int *font, int *screen, int x, int y, align ment, char *format, ...)
;
extern void vdustr(int a, ...)
;
extern void writec(int n)
;
