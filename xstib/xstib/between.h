#include <SDL/SDL.h>

/* Auto */

extern void gameloop(void)
;
extern void initplayfield(void)
;
extern void initgame(void)
;
extern char getch(void)
;
extern void addhiscore(int lev, int cscore, SDL_Surface *screen,
  SDL_Surface* backscreen);

extern void drawbutton(int add, SDL_Surface *screen, int x, int y, char *name, int state)
;
extern void redrawbutton(SDL_Surface* screen, int num, int state)
;
extern void drawtitles(SDL_Surface *screen, int level)
;
extern void drawhiscores(SDL_Surface *screen, int level)
;
extern int titleloop(SDL_Surface *screen, SDL_Surface* backscreen)
;
extern void resethiscores(SDL_Surface *screen, SDL_Surface* backscreen)
;
extern void savehiscores(void)
;
extern void losesounds(void)
;
