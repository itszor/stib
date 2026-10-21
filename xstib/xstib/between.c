#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include <SDL/SDL_mixer.h>

/*#include "kernel.h"*/
/*#include "swis.h"*/

#include "sat.h"
#include "fsound.h"
#include "fade.h"
#include "play.h"
#include "between.h"
#include "levels.h"
#include "global.h"

/*
#include "DeskLib:swi.h"
#include "DeskLib:sprite.h"
#include "DeskLib:screen.h"
#include "DeskLib:wimpswis.h"
#include "DeskLib:kbd.h"
*/


void gameloop(void)
{
  int *coltab;
  SDL_Surface* screen = vidscreen;
  int levelnumber=0;
/*  sprite_area ptr;*/
  int screenmem;
  int loop;
  struct timeval currenttime;
  
  gettimeofday(&currenttime, 0);

  returntodesk=false;

  srand(currenttime.tv_sec);

  /* try to ensure 160k of screen memory */
/*  SWI(1, 2, SWI_OS_ReadDynamicArea, 2, NULL, &screenmem);
  if (screenmem<160*1024) SWI(2, 0, SWI_OS_ChangeDynamicArea, 2, 160*1024-screenmem); */
  /* If a problem occurs, just sod it & crash, probably */

/*  sat_screenmode(15);
  sat_screenmode(13);
  vdustr(5,23,0,10,16,0,0,0,0,0,0,-1);*/ /* turn off cursor */
  sat_initialise();
/*  sat_resetscreen(sat_data.screenbase); */  /* change back to vdu_1, hardware_1 and cls */
  sat_data.screenbuffer=1;

//  sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.titles, 9), screen, 87, 53);

  sat_loadsprites(&spriteareas.titles, "data/titles");
  sat_loadsprites(&spriteareas.background, "data/32x32");
  sat_loadsprites(&spriteareas.blocks,     "data/24x24");
  sat_loadsprites(&spriteareas.shooter,    "data/16x16");
  sat_loadsprites(&spriteareas.shootbase,  "data/64x32");
  sat_loadsprites(&spriteareas.danger,     "data/bar");
  sat_loadsprites(&spriteareas.gamefont,   "data/gamefont");
  sat_loadsprites(&spriteareas.smaller,    "data/smaller");
  sat_loadsprites(&spriteareas.lakeoffire, "data/32x32m");
  sat_loadsprites(&spriteareas.megahack,   "data/12x24");
  sat_loadsprites(&coltab,                 "data/fadecol"); /* actually, not sprites */

  sat_makearbsurfaces(spriteareas.titles, sdlsurfaces.titles, 10, 1);
  sat_makesurfaces(spriteareas.background, sdlsurfaces.background, 32, 32, 8, 
    0);
  sat_makesurfaces(spriteareas.blocks, sdlsurfaces.blocks, 24, 24, 42, 1);
  sat_makesurfaces(spriteareas.smaller, sdlsurfaces.smaller, 16, 16, 42, 1);
  sat_makesurfaces(spriteareas.shooter, sdlsurfaces.shooter, 16, 16, 2, 1);
  sat_makesurfaces(spriteareas.shootbase, sdlsurfaces.shootbase, 64, 32, 2, 1);
  sat_makearbsurfaces(spriteareas.danger, sdlsurfaces.danger, 10, 1);
  sat_makearbsurfaces(spriteareas.gamefont, sdlsurfaces.gamefont, 58, 1);
  sat_makesurfaces(spriteareas.lakeoffire, sdlsurfaces.lakeoffire, 32, 32, 8, 
    1);
  sat_makesurfaces(spriteareas.megahack, sdlsurfaces.megahack, 12, 24, 1, 0);

  sat_blit(sdlsurfaces.titles[9], screen, 87, 53);

/*  ptr=Sprite_LoadFile("Stib:data.ptr");*/

  if (Mix_OpenAudio(8000, MIX_DEFAULT_FORMAT, 2, 128) < 0)
  {
    fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
    exit(1);
  }
  else
  {
    int audio_rate, channels;
    short audio_format;
    Mix_QuerySpec(&audio_rate, &audio_format, &channels);
    fprintf(stderr, "Opened audio at %d Hz %d bit %s\n", audio_rate,
      (audio_format&0xff), (channels>1) ? "stereo" : "mono");
  }

#define TESTWAV(X) if ((X)==0) { \
  fprintf(stderr, "Couldn't do %s: %s\n", \
    #X, SDL_GetError()); \
  exit(1); \
  }
    

  TESTWAV(sounds.boing     = Mix_LoadWAV("data/better_boing.wav"))
  TESTWAV(sounds.fanfare   = Mix_LoadWAV("data/glass1.wav"))
  TESTWAV(sounds.fanfare2  = Mix_LoadWAV("data/glass2.wav"))
  TESTWAV(sounds.fire	     = Mix_LoadWAV("data/missle.wav"))
  TESTWAV(sounds.lockin	   = Mix_LoadWAV("data/lock.wav"))
  TESTWAV(sounds.transport = Mix_LoadWAV("data/explo.wav"))
  TESTWAV(sounds.wedied	   = Mix_LoadWAV("data/frying.wav"))
  TESTWAV(sounds.whoosh	   = Mix_LoadWAV("data/fall.wav"))

  Mix_SetPanning(0, 128, 128);

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

    drawtitles(buf2(screen), levelnumber);
    fade_zoom(buf1(screen), buf2(screen));
    /* turn on pointer */
/*    SWI(8,0,SWI_OS_SpriteOp,256+36,ptr,"titleptr",2,1,1,0,0); */ 
    setlevelinfo(levelnumber=titleloop(vidscreen, backscreen));
  /*  SWI(2,0,0x400D8,0,-1);  */              /* turn off pointer */
    if (returntodesk) continue;           /* drop out of loop if rtdesk */
    initplayfield();
    initgame();

    /* xfade in */
    drawgamescreen(buf2(screen), 0, 0);

    sat_blit(sdlsurfaces.titles[6], buf1(screen), 95, 104);
    sat_blit(sdlsurfaces.titles[6], buf2(screen), 95, 104);

    fade_xfade(buf1(screen), buf2(screen), coltab);

    SDL_ShowCursor(SDL_DISABLE);
    playagame();
    SDL_ShowCursor(SDL_ENABLE);

    /* change back to vdu_1, hardware_1 and cls */
//    sat_resetscreen(sat_data.screenbase);   
    sat_data.screenbuffer=1;
    sat_plot_backdrop(backdroptile, backscreen);

    sat_printf(sdlsurfaces.gamefont, backscreen, 160, 64, centre, "score: %d", 
      score.future);

    if (game==won)
    {
      flexisound_play(sounds.fanfare, 2, 0x17f, 0x1700);
      sat_printf(sdlsurfaces.gamefont, backscreen, 160, 32, centre,
        "level %d completed!", levelnumber+1);
      sat_printf(sdlsurfaces.gamefont, backscreen, 160, 80, centre,
        "level completion bonus: %d", (levelnumber+1)*1000);
      score.future+=(levelnumber+1)*1000;
      sat_printf(sdlsurfaces.gamefont, backscreen, 160, 96, centre,
        "stib use bonus: %d", lev.stibuse);
      score.future+=lev.stibuse;
      sat_printf(sdlsurfaces.gamefont, backscreen, 160, 112, centre,
        "final score: %d", score.future);
    }
    else
    {
      sat_printf(sdlsurfaces.gamefont, backscreen, 160, 32, centre,
        "level incomplete!");
      sat_printf(sdlsurfaces.gamefont, backscreen, 160, 112, centre,
        "no bonus");
    }
    
    sat_refresh(vidscreen, backscreen);

    addhiscore(levelnumber, score.future, vidscreen, backscreen);
  }

  Mix_FreeChunk(sounds.boing);
  Mix_FreeChunk(sounds.fanfare);
  Mix_FreeChunk(sounds.fanfare2);
  Mix_FreeChunk(sounds.fire);
  Mix_FreeChunk(sounds.lockin);
  Mix_FreeChunk(sounds.transport);
  Mix_FreeChunk(sounds.wedied);
  Mix_FreeChunk(sounds.whoosh);
  
  Mix_CloseAudio();

  /* free all stuff before return to desktop */
  free(costab);
  free(sintab);
/*  free(ptr);*/
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

  /* put screen memory back to what it were before */
/*  if (screenmem<160*1024) SWI(2, 0, SWI_OS_ChangeDynamicArea, 2, screenmem-160*1024);*/
/*  sat_screenmode(screen_mode);*/ /* screen_mode a global from DeskLib */

  /* return screen, pointer to previous states */
/*  SWI(8,0,0x400E9,256+36,0,"ptr_default",1+16,0,0,0,0);*/ /* turn on pointer */
/*  Window_ForceRedraw(-1, 0, 0, screen_size.x, screen_size.y);*/
/*  Wimp_SetMode(screen_mode);*/
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
      else array[xloop][yloop].info.full=false;
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
  backdroptile=sdlsurfaces.background[rand()&7];
/*
  gamestate=waiting;
  pendingstate=waiting;*/
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

int waitkey(void)
{
  SDL_Event event;
  int inp;

  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
      case SDL_QUIT:
      exit(0);
      break;

      case SDL_KEYUP:
      {
      }
      break;

      case SDL_KEYDOWN:
      {
        inp = event.key.keysym.unicode & 0xff;
//        fprintf(stderr, "Got key! %d\n", inp);
        return inp;
      }
      break;

      case SDL_MOUSEBUTTONDOWN:
      {
        return '\n';
      }
      break;
    }
  }
}


void addhiscore(int lev, int cscore, SDL_Surface *displaysurf,
  SDL_Surface* backsurf)
{
  static char newname[22]=""; /* just initialise to start with */
  static unsigned char inp;
  int pos, shuf;
  const int xp=75, yp=152, xf=75;
  int mx, my, mb;

  /* Flush keyboard buffer */
/*  _kernel_osbyte(15,1,0);*/
//  rewind(stdin);

  if (cscore>scoretab.entry[lev][9].score)
  {
    int returnhit = 0;
    
    sat_printf(sdlsurfaces.gamefont, backsurf, 160, 215, centre,
      "enter name and press return");
    sat_blit(sdlsurfaces.titles[5], backsurf, xp, yp);
//    sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.titles, 5), screen, xp, yp);
    sat_printf(sdlsurfaces.gamefont, backsurf, xf+5, yp+17, left, "%s", 
      newname);

    while (!returnhit)
    {
      int oldlen=strlen(newname);

      inp = waitkey();
            
      if (inp==13)
      {
        returnhit = 1;
        continue;
      }

      inp = tolower(inp);

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
      
      sat_blit(sdlsurfaces.titles[5], backsurf, xp, yp);
//      sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.titles, 5), screen, xp, yp);
      sat_printf(sdlsurfaces.gamefont, backsurf, xf+5, yp+17, left, "%s", 
        newname);

      sat_refresh(displaysurf, backsurf);
    }
    for (pos=0; scoretab.entry[lev][pos].score>=cscore && pos<10; pos++);
    for (shuf=8; shuf>=pos; shuf--)
      scoretab.entry[lev][shuf+1]=scoretab.entry[lev][shuf];
    strcpy(scoretab.entry[lev][pos].name, newname);
    scoretab.entry[lev][pos].score=cscore;
  }
  else
  {
    sat_printf(sdlsurfaces.gamefont, backsurf, 160, 215, centre,
      "click mouse to continue");
    
    sat_refresh(displaysurf, backsurf);
/*    sat_mouseop(&mx, &my, &mb);
    while (mb!=0) sat_mouseop(&mx, &my, &mb);
    while (mb==0) sat_mouseop(&mx, &my, &mb); */
  }
}


void drawbutton(int add, SDL_Surface *screen, int x, int y, char *name,
  int state)
{
  #ifdef DEMO
  if (add>=4 && add<=7) state=8;
  #endif
  sat_blit(sdlsurfaces.titles[state], screen, x, y);
/*  sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.titles, state), screen, x, y); */
  #ifdef DEMO
  if (add>=4 && add<=7) state=0;
  #endif
  sat_printf(sdlsurfaces.gamefont, screen, x+32+state, y+4+state, centre, "%s", name);
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


void redrawbutton(SDL_Surface* screen, int num, int state)
{
  if (tainfo.area[num].cstate!=state)
  {
    sat_blit(sdlsurfaces.titles[state], screen, tainfo.area[num].l, 
      tainfo.area[num].t);
/*    sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.titles, state), 
      screen, tainfo.area[num].l, tainfo.area[num].t); */
    sat_printf(sdlsurfaces.gamefont, screen,
      tainfo.area[num].l+32+state, tainfo.area[num].t+4+state,
      centre, "%s", tainfo.area[num].text);
    tainfo.area[num].cstate=state;
  }
}


void drawtitles(SDL_Surface *screen, int level)
{
  register int xloop, yloop;

  for (xloop=0; xloop<320; xloop+=64)
  {
    for (yloop=0; yloop<256; yloop+=64)
    {
      sat_blit(sdlsurfaces.titles[2], screen, xloop, yloop);
/*      sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.titles, 2), screen, xloop, yloop); */
    }
  }

  sat_blit(sdlsurfaces.titles[3], screen, 32, 16);
/*  sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.titles, 3), screen, 32, 16); */
  drawhiscores(screen, level);

  #ifdef DEMO
  sat_printf(sdlsurfaces.gamefont, screen, 160, 4, centre, "demo version");
  #endif

  sat_printf(sdlsurfaces.gamefont, screen, 160, 242, centre, "(c) monster software 1996");
  sat_printf(sdlsurfaces.gamefont, screen, 160, 124, centre, "hiscores");

  drawbutton(0, screen, 8, 180, "play", 0);
  drawbutton(1, screen, 8, 200, "clear", 0);
  drawbutton(2, screen, 8, 220, "desktop", 0);
  drawbutton(3, screen, 248, 140, "practise", level==0);
  drawbutton(4, screen, 248, 160, "easy",     level==1);
  drawbutton(5, screen, 248, 180, "medium",   level==2);
  drawbutton(6, screen, 248, 200, "hard",     level==3);
  drawbutton(7, screen, 248, 220, "evil",     level==4);
}


void drawhiscores(SDL_Surface* screen, int level)
{
  register int loop;
  static const int hleft=80, hright=240, htop=138;

  sat_blit(sdlsurfaces.titles[4], screen, hleft, htop);
/*  sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.titles, 4), screen, hleft, htop); */
  for (loop=0; loop<10; loop++)
  {
    sat_printf(sdlsurfaces.gamefont, screen, hleft+3, htop+4+loop*9, left, "%s",
      scoretab.entry[level][loop].name);
    sat_printf(sdlsurfaces.gamefont, screen, hright-2, htop+4+loop*9, right, 
      "%d", scoretab.entry[level][loop].score);
  }
}


int titleloop(SDL_Surface *displaysurf, SDL_Surface* backsurf)
{
  int loop, mx = 0, my = 0, mb = 0, done=0, hit;
  static int level=0;
  SDL_Event event;

  while (!done)
  {
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
        case SDL_MOUSEMOTION:
        {
          mb = (event.motion.state == SDL_PRESSED);
          transformmouse(&mx, &my, event.motion.x, event.motion.y);
        }
        break;
        
        case SDL_MOUSEBUTTONDOWN:
        {
          mb = 1;
          transformmouse(&mx, &my, event.button.x, event.button.y);
        }
        break;
        
        case SDL_MOUSEBUTTONUP:
        {
          mb = 0;
        }
        break;
        
        case SDL_QUIT:
        {
          exit(0);
        }
        break;
      }
    }
//    sat_mouseop(&mx, &my, &mb);
/*    mx/=4;
    my=255-(my/4); */
    if (mb)
    {
      hit=-1;

      for (loop=0; loop<8; loop++)
        if (mx>tainfo.area[loop].l && mx<tainfo.area[loop].r &&
          my>tainfo.area[loop].t && my<tainfo.area[loop].b) hit=loop;

      if (hit!=-1)
      {
        if (hit>=3)
        {
          #ifndef DEMO
          level=hit-3;
          redrawbutton(backsurf, 3, level==0);
          redrawbutton(backsurf, 4, level==1);
          redrawbutton(backsurf, 5, level==2);
          redrawbutton(backsurf, 6, level==3);
          redrawbutton(backsurf, 7, level==4);
          drawhiscores(backsurf, level);
          #else
          switch (hit-3)
          {
	          case 0:
            {
              level=hit-3;
              redrawbutton(backsurf, 3, level==0);
              drawhiscores(backsurf, level);
            }
            break;
          }
          #endif
        }
        else
        {
          redrawbutton(backsurf, 0, hit==0);
          redrawbutton(backsurf, 1, hit==1);
          redrawbutton(backsurf, 2, hit==2);
        }
     //   while (mb!=0) sat_mouseop(&mx, &my, &mb);
        if (hit<3)
        {
          redrawbutton(backsurf, 0, 0);
          redrawbutton(backsurf, 1, 0);
          redrawbutton(backsurf, 2, 0);
        }

        switch (hit)
        {
          case 0:
          done=1;
          break;

          case 1:
          resethiscores(displaysurf, backsurf);
          drawtitles(backsurf, level);
          mb = 0;
          break;

          case 2:
            /* losesounds called atexit()*/
//            Wimp_CommandWindow(-1);
          done=1;
          returntodesk=true;
          break;
        }
      }
      sat_refresh(displaysurf, backsurf);
      usleep(100);
    }
  }
  return level;
}


void resethiscores(SDL_Surface *displaysurf, SDL_Surface* backsurf)
{
  int loop, loop2;
  char n;
  SDL_Event event;
  const char yes='y', no='n';

  sat_blit(sdlsurfaces.titles[7], backsurf, 96, 117);
/*  sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.titles, 7), screen, 96, 117); */

  sat_refresh(displaysurf, backsurf);

  while (1)
  {
    n = waitkey();
    if (n==yes || n==no || n=='\n') break;
  }

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
  sat_savefile("data/5x10", (int*)&scoretab, sizeof(hst));
  #endif
/*  losesounds(); can't do this here */
}


