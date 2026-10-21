#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "kernel.h"
#include "swis.h"

#include "sat.h"
#include "fsound.h"
#include "fade.h"
#include "play.h"
#include "between.h"
#include "levels.h"
#include "global.h"

#include "DeskLib:swi.h"
#include "DeskLib:sprite.h"
#include "DeskLib:screen.h"
#include "DeskLib:wimpswis.h"
#include "DeskLib:kbd.h"


void gameloop(void)
{
  int *coltab;
  int *screen;
  int levelnumber=0;
  sprite_area ptr;
  int screenmem;
  int loop;

  returntodesk=false;

  srand(sat_timeop());

  sat_loadsprites(&spriteareas.titles, "Stib:data.titles");

  /* try to ensure 160k of screen memory */
  SWI(1, 2, SWI_OS_ReadDynamicArea, 2, NULL, &screenmem);
  if (screenmem<160*1024) SWI(2, 0, SWI_OS_ChangeDynamicArea, 2, 160*1024-screenmem);
  /* If a problem occurs, just sod it & crash, probably */

  sat_screenmode(15);
  sat_screenmode(13);
  vdustr(5,23,0,10,16,0,0,0,0,0,0,-1); /* turn off cursor */
  sat_initialise();
  sat_resetscreen(sat_data.screenbase); /* change back to vdu_1, hardware_1 and cls */
  sat_data.screenbuffer=1;
  screen=sat_data.screenbase;

  sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.titles, 9), screen, 87, 53);

  sat_loadsprites(&spriteareas.background, "Stib:data.32x32");
  sat_loadsprites(&spriteareas.blocks,     "Stib:data.24x24");
  sat_loadsprites(&spriteareas.shooter,    "Stib:data.16x16");
  sat_loadsprites(&spriteareas.shootbase,  "Stib:data.64x32");
  sat_loadsprites(&spriteareas.danger,     "Stib:data.bar");
  sat_loadsprites(&spriteareas.gamefont,   "Stib:data.gamefont");
  sat_loadsprites(&spriteareas.smaller,    "Stib:data.smaller");
  sat_loadsprites(&spriteareas.lakeoffire, "Stib:data.32x32m");
  sat_loadsprites(&spriteareas.megahack,   "Stib:data.12x24");
  sat_loadsprites(&coltab,                 "Stib:data.fadecol"); /* actually, not sprites */

  ptr=Sprite_LoadFile("Stib:data.ptr");

  sounds.boing=flexisound_add("Stib:data.boing");
  sounds.fanfare=flexisound_add("Stib:data.fanfare");
  sounds.fanfare2=flexisound_add("Stib:data.fanfare2");
  sounds.fire=flexisound_add("Stib:data.fire");
  sounds.lockin=flexisound_add("Stib:data.lockin");
  sounds.transport=flexisound_add("Stib:data.transport");
  sounds.wedied=flexisound_add("Stib:data.wedied");
  sounds.whoosh=flexisound_add("Stib:data.whoosh");

  sintab=malloc(360*4*4);
  costab=malloc(360*4*4);

  for (loop=0; loop<360*4; loop++)
  {
    sintab[loop]=(int)(sin(rad((float)loop/4.))*256.);
    costab[loop]=(int)(cos(rad((float)loop/4.))*256.);
  }

  while (!returntodesk)
  {
    sat_resetscreen(sat_data.screenbase);
    sat_data.screenbuffer=1;
    screen=sat_data.screenbase;

    drawtitles(buf2(screen), levelnumber);
    fade_zoom(buf1(screen), buf2(screen));
    /* turn on pointer */
    SWI(8,0,SWI_OS_SpriteOp,256+36,ptr,"titleptr",2,1,1,0,0);
    setlevelinfo(levelnumber=titleloop(screen));
    SWI(2,0,0x400D8,0,-1);                /* turn off pointer */
    if (returntodesk) continue;           /* drop out of loop if rtdesk */
    initplayfield();
    initgame();

    /* xfade in */
    screen=sat_data.screenbase;
    drawgamescreen(buf2(screen), 0, 0);

    sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.titles, 6), buf1(screen), 95, 104);
    sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.titles, 6), buf2(screen), 95, 104);

    fade_xfade(buf1(screen), buf2(screen), coltab);

    playagame();

    sat_resetscreen(sat_data.screenbase);   /* change back to vdu_1, hardware_1 and cls */
    sat_data.screenbuffer=1;
    screen=sat_data.screenbase;
    sat_plot_backdrop(backdroptile, screen);

    sat_printf(spriteareas.gamefont, screen, 160, 64, centre, "score: %d", score.future);

    if (game==won)
    {
      flexisound_play(sounds.fanfare, 2, 0x17f, 0x1700);
      sat_printf(spriteareas.gamefont, screen, 160, 32, centre,
        "level %d completed!", levelnumber+1);
      sat_printf(spriteareas.gamefont, screen, 160, 80, centre,
        "level completion bonus: %d", (levelnumber+1)*1000);
      score.future+=(levelnumber+1)*1000;
      sat_printf(spriteareas.gamefont, screen, 160, 96, centre,
        "stib use bonus: %d", lev.stibuse);
      score.future+=lev.stibuse;
      sat_printf(spriteareas.gamefont, screen, 160, 112, centre,
        "final score: %d", score.future);
    }
    else
    {
      sat_printf(spriteareas.gamefont, screen, 160, 32, centre, "level incomplete!");
      sat_printf(spriteareas.gamefont, screen, 160, 112, centre, "no bonus");
    }

    addhiscore(levelnumber, score.future, screen);
  }

  losesounds();

  /* free all stuff before return to desktop */
  free(costab);
  free(sintab);
  free(ptr);
  free(coltab);
  free(spriteareas.megahack);
  free(spriteareas.lakeoffire);
  free(spriteareas.smaller);
  free(spriteareas.gamefont);
  free(spriteareas.danger);
  free(spriteareas.shootbase);
  free(spriteareas.shooter);
  free(spriteareas.blocks);
  free(spriteareas.background);
  free(spriteareas.titles);

  /* return screen to what it were before */
  Wimp_SetMode(screen_mode);
}


void initplayfield(void)
{
  register int xloop, yloop;

  for (xloop=0; xloop<lev.gridsize.x; xloop++)
  {
    for (yloop=0; yloop<lev.gridsize.y; yloop++)
    {
      array[xloop][yloop].value=0;
      if (yloop<lev.startheight)
      {
        if (lev.dudsin==true && yloop>0 && rand()%20==17) array[xloop][yloop].info.sprite=7;
          else array[xloop][yloop].info.sprite=rand()%7;
        array[xloop][yloop].info.full=true;
      }
    }
  }
}


void initgame(void)
{
  danger.future=0;
  danger.current=0;
  down.pixels.current=0;
  down.pixels.future=0;
  down.rows=0;
  score.future=0;
  score.current=0;
  pendgstimeout=-1;
  nxshp.state=stable;
  burningstate=-1;

  /* choose a backdrop */
  backdroptile=nomaskindex(spriteareas.background, rand()%8, 32, 32);

  evaluatetypes();
  tcell.cell.current=types[rand()%numtypes];
  tcell.cell.future=types[rand()%numtypes];
  nxshp.cell.current=tcell.cell.current;
  nxshp.cell.future=tcell.cell.future;

  stibshooter.inuse=false;
  stibshooter.angle=0;
  playfield.exploding=0;
  playfield.shuffling=0;
  playfield.burning=0;
  playfield.detaching=0;
  game=alive;
  bigfanfare.delaysfx=false;
}


char getch(void)
{
  _kernel_swi_regs in;
  _kernel_swi(OS_ReadC, &in, &in);
  return in.r[0];
}


void addhiscore(int lev, int cscore, int *screen)
{
  static char newname[22]=""; /* just initialise to start with */
  static unsigned char inp;
  int pos, shuf;
  const int xp=75, yp=152, xf=75;
  int mx, my, mb;

  /* Flush keyboard buffer */
  _kernel_osbyte(15,1,0);

  if (cscore>scoretab.entry[lev][9].score)
  {
    sat_printf(spriteareas.gamefont, screen, 160, 215, centre,
      "enter name and press return");
    sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.titles, 5), screen, xp, yp);
    sat_printf(spriteareas.gamefont, screen, xf+5, yp+17, left, "%s", newname);

    while ((inp=getch())!=13)
    {
      int oldlen=strlen(newname);

      if (inp>='A' && inp<='Z') inp|=32;

      if ((inp==127 || inp==8) && oldlen>0) newname[oldlen-1]=0;
      else
      {
        if (oldlen<14)
        {
          if (inp==63 || inp==143  ||
             (inp>=32 && inp<=35)  ||
             (inp>=97 && inp<=122) ||
             (inp>=39 && inp<=59)) newname[oldlen]=inp, newname[oldlen+1]=0;
        }
      }

      sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.titles, 5), screen, xp, yp);
      sat_printf(spriteareas.gamefont, screen, xf+5, yp+17, left, "%s", newname);
    }
    for (pos=0; scoretab.entry[lev][pos].score>=cscore && pos<10; pos++);
    for (shuf=8; shuf>=pos; shuf--)
      scoretab.entry[lev][shuf+1]=scoretab.entry[lev][shuf];
    strcpy(scoretab.entry[lev][pos].name, newname);
    scoretab.entry[lev][pos].score=cscore;
  }
  else
  {
    sat_printf(spriteareas.gamefont, screen, 160, 215, centre,
      "click mouse to continue");
    sat_mouseop(&mx, &my, &mb);
    while (mb!=0) sat_mouseop(&mx, &my, &mb);
    while (mb==0) sat_mouseop(&mx, &my, &mb);
  }
}


void drawbutton(int add, int *screen, int x, int y, char *name, int state)
{
  #ifdef DEMO
  if (add>=4 && add<=7) state=8;
  #endif
  sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.titles, state), screen, x, y);
  #ifdef DEMO
  if (add>=4 && add<=7) state=0;
  #endif
  sat_printf(spriteareas.gamefont, screen, x+32+state, y+4+state, centre, "%s", name);
  if (add!=-1)
  {
    tainfo.area[add].l=x;
    tainfo.area[add].r=x+64;
    tainfo.area[add].t=y;
    tainfo.area[add].b=y+15;
    tainfo.area[add].cstate=state;
    strcpy(tainfo.area[add].text, name);
  }
}


void redrawbutton(int *screen, int num, int state)
{
  if (tainfo.area[num].cstate!=state)
  {
    sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.titles, state),
      screen, tainfo.area[num].l, tainfo.area[num].t);
    sat_printf(spriteareas.gamefont, screen,
      tainfo.area[num].l+32+state, tainfo.area[num].t+4+state,
      centre, "%s", tainfo.area[num].text);
    tainfo.area[num].cstate=state;
  }
}


void drawtitles(int *screen, int level)
{
  register int xloop, yloop;

  for (xloop=0; xloop<320; xloop+=64)
  {
    for (yloop=0; yloop<256; yloop+=64)
    {
      sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.titles, 2), screen, xloop, yloop);
    }
  }

  sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.titles, 3), screen, 32, 16);
  drawhiscores(screen, level);

  #ifdef DEMO
  sat_printf(spriteareas.gamefont, screen, 160, 4, centre, "demo version");
  #endif

  sat_printf(spriteareas.gamefont, screen, 160, 242, centre, "(c) monster software 1996");
  sat_printf(spriteareas.gamefont, screen, 160, 124, centre, "hiscores");

  drawbutton(0, screen, 8, 180, "play", 0);
  drawbutton(1, screen, 8, 200, "clear", 0);
  drawbutton(2, screen, 8, 220, "desktop", 0);
  drawbutton(3, screen, 248, 140, "practise", level==0);
  drawbutton(4, screen, 248, 160, "easy",     level==1);
  drawbutton(5, screen, 248, 180, "medium",   level==2);
  drawbutton(6, screen, 248, 200, "hard",     level==3);
  drawbutton(7, screen, 248, 220, "evil",     level==4);
}


void drawhiscores(int *screen, int level)
{
  register int loop;
  static const int hleft=80, hright=240, htop=138;

  sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.titles, 4), screen, hleft, htop);
  for (loop=0; loop<10; loop++)
  {
    sat_printf(spriteareas.gamefont, screen, hleft+3, htop+4+loop*9, left, "%s",
      scoretab.entry[level][loop].name);
    sat_printf(spriteareas.gamefont, screen, hright-2, htop+4+loop*9, right, "%d",
      scoretab.entry[level][loop].score);
  }
}


int titleloop(int *screen)
{
  int loop, mx, my, mb, done=0, hit;
  static int level=0;

  while (!done)
  {
    sat_mouseop(&mx, &my, &mb);
    mx/=4;
    my=255-(my/4);
    if (mb==4)
    {
      hit=-1;

      for (loop=0; loop<8; loop++)
        if (mx>tainfo.area[loop].l && mx<tainfo.area[loop].r &&
          my>tainfo.area[loop].t && my<tainfo.area[loop].b) hit=loop;

      if (hit!=-1)
      {
        if (hit>=3)
        {
          level=hit-3;
          redrawbutton(screen, 3, level==0);
          redrawbutton(screen, 4, level==1);
          redrawbutton(screen, 5, level==2);
          redrawbutton(screen, 6, level==3);
          redrawbutton(screen, 7, level==4);
          drawhiscores(screen, level);
        }
        else
        {
          redrawbutton(screen, 0, hit==0);
          redrawbutton(screen, 1, hit==1);
          redrawbutton(screen, 2, hit==2);
        }
        while (mb!=0) sat_mouseop(&mx, &my, &mb);
        if (hit<3)
        {
          redrawbutton(screen, 0, 0);
          redrawbutton(screen, 1, 0);
          redrawbutton(screen, 2, 0);
        }

        switch (hit)
        {
          case 0:
          done=1;
          break;

          case 1:
          resethiscores(screen);
          drawtitles(screen, level);
          break;

          case 2:
          /* losesounds called atexit()*/
          Wimp_CommandWindow(-1);
          done=1;
          returntodesk=true;
          break;
        }
      }
    }
  }
  return level;
}


void resethiscores(int *screen)
{
  int loop, loop2;
  char n;
  const char yes='y', no='n';

  sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.titles, 7), screen, 96, 117);

  do n=getch(); while (tolower(n)!=no && tolower(n)!=yes);

  if (tolower(n)==yes)
  {
    for (loop=0; loop<5; loop++) for (loop2=0; loop2<10; loop2++)
    strcpy(scoretab.entry[loop][loop2].name, "no name"),
      scoretab.entry[loop][loop2].score=0;
  }
}


void savehiscores(void)
{
  #ifndef DEMO
  sat_savefile("Stib:data.5x10", (int*)&scoretab, sizeof(hst));
  #endif
/*  losesounds(); can't do this here */
}


void losesounds(void)
{
  flexisound_remove(sounds.whoosh);
  flexisound_remove(sounds.wedied);
  flexisound_remove(sounds.transport);
  flexisound_remove(sounds.lockin);
  flexisound_remove(sounds.fire);
  flexisound_remove(sounds.fanfare2);
  flexisound_remove(sounds.fanfare);
  flexisound_remove(sounds.boing);
}
