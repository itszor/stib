/* Auto */

extern void gameloop(void)
;
extern void initplayfield(void)
;
extern void initgame(void)
;
extern char getch(void)
;
extern void addhiscore(int lev, int cscore, int *screen)
;
extern void drawbutton(int add, int *screen, int x, int y, char *name, int state)
;
extern void redrawbutton(int *screen, int num, int state)
;
extern void drawtitles(int *screen, int level)
;
extern void drawhiscores(int *screen, int level)
;
extern int titleloop(int *screen)
;
extern void resethiscores(int *screen)
;
extern void savehiscores(void)
;
extern void losesounds(void)
;
