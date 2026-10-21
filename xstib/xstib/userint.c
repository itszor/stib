#include <stdlib.h>

/*
#include "DeskLib:wimpswis.h"
#include "DeskLib:core.h"
#include "DeskLib:resource.h"
#include "DeskLib:wimp.h"
#include "DeskLib:window.h"
#include "DeskLib:event.h"
#include "DeskLib:eventmsg.h"
#include "DeskLib:menu.h"
#include "DeskLib:icon.h"
#include "DeskLib:screen.h"
#include "DeskLib:template.h"
#include "DeskLib:handler.h"
#include "DeskLib:error.h"
*/

#include "sat.h"
#include "play.h"
#include "between.h"
#include "global.h"
#include "userint.h"

#include <SDL/SDL.h>

int main(int argc, char* argv[])
{
  SDL_Color pal[256];
  int vidcpalette[16] =
  {
    0,    273,  546,  819,  4,    277,  550,  823,
    1024, 1297, 1570, 1843, 1028, 1301, 1574, 1847
  };
  int c, d, i;
  int sizemult = 1;
  int enablesound = 1;
  int fullscreen = 0;

  if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)==-1)
  {
    fprintf(stderr, "Could not initialise SDL: %s.\n", SDL_GetError());
    exit(1);
  }

  atexit(SDL_Quit);

  for (i=1; i<argc; i++)
  {
    if (strcmp(argv[i], "-2")==0 || strcmp(argv[i], "--double")==0)
    {
      sizemult = 2;
    }
    else if (strcmp(argv[i], "-3")==0 || strcmp(argv[i], "--triple")==0)
    {
      sizemult = 3;
    }
    else if (strcmp(argv[i], "-4")==0 || strcmp(argv[i], "--quad")==0)
    {
      sizemult = 4;
    }
    else if (strcmp(argv[i], "-ns")==0 || strcmp(argv[i], "--nosound")==0)
    {
      enablesound = 0;
    }
    else if (strcmp(argv[i], "-fs")==0 || strcmp(argv[i], "--fullscreen")==0)
    {
      fullscreen = 1;
    }
    else
    {
      fprintf(stderr, "Unknown option %s\n", argv[i]);
      exit(1);
    }
  }

  windowzoom = sizemult;

  // here be monsters  
  for (c=0; c<256; c++)
  {
    int palentry = c & 15;
    int L4 = (c>>4) & 1;
    int L65 = (c>>5) & 3;
    int L7 = (c>>7) & 1;
    pal[c].r = ((vidcpalette[palentry] & 7) | (L4<<3))<<4;
    pal[c].g = (((vidcpalette[palentry] >> 4) & 3) | (L65<<2))<<4;
    pal[c].b = (((vidcpalette[palentry] >> 8) & 7) | (L7<<3))<<4;
  }
  
  vidscreen = SDL_SetVideoMode(320*sizemult, 256*sizemult, 8, SDL_HWSURFACE
    | (fullscreen ? SDL_FULLSCREEN : 0));

  if (vidscreen == NULL)
  {
    fprintf(stderr, "Couldn't set 320x256x8 video mode: %s.\n",
      SDL_GetError());
    exit(1);
  }
  
  SDL_WM_SetCaption("Stib", NULL);
  
  backscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 256, 8, 0, 0, 0, 0);
  
  SDL_SetColors(vidscreen, (SDL_Color*)pal, 0, 256);
  SDL_SetColors(backscreen, (SDL_Color*)pal, 0, 256);

/*  
  for (d=0; d<100; d++)
  {
    SDL_Event event;
    
    SDL_LockSurface(backscreen);
    for (c=0; c<320*256; c++)
    {
      ((char*)backscreen->pixels)[c] = c+d*200;
    }
    SDL_UnlockSurface(backscreen);
    SDL_BlitSurface(backscreen, 0, vidscreen, 0);
    SDL_UpdateRect(vidscreen, 0, 0, 0, 0);
  }
*/
  
  SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);
  SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_ENABLE);
  SDL_EventState(SDL_QUIT, SDL_ENABLE);
  SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);
  SDL_EventState(SDL_KEYUP, SDL_ENABLE);
  SDL_EnableUNICODE(1);
  
  gameloop();
}

void transformmouse(int* xdest, int* ydest, int xsrc, int ysrc)
{
  *xdest = xsrc / windowzoom;
  *ydest = ysrc / windowzoom;
  if (*xdest < 0) *xdest = 0;
  if (*ydest < 0) *ydest = 0;
  if (*xdest > 319) *xdest = 319;
  if (*ydest > 255) *ydest = 255;
}

/*
int main(void)
{
  static window_handle ws_proginfo;
  static icon_handle ws_iconbaricon;
  static menu_ptr ws_iconbarmenu;

  Resource_InitialisePath("Stib");
  Event_Initialise("Stib");
  EventMsg_Initialise();
  Screen_CacheModeInfo();
  Template_Initialise();
  Template_LoadFile("Templates");

  ws_proginfo=Window_Create("proginfo", 0);

  #ifdef GERMAN
    #ifdef DEMO
    Icon_SetText(ws_proginfo, 9, "Freeware demo version");
    #else
    Icon_SetText(ws_proginfo, 9, "Registered Shareware");
    #endif
  #else
    #ifdef DEMO
    Icon_SetText(ws_proginfo, 9, "Freeware demo version");
    #else
    Icon_SetText(ws_proginfo, 9, global_reguser);
    #endif
  #endif

  // hiscore handling 
  sat_loadfile("Stib:data.5x10", (int*)&scoretab);
  atexit(savehiscores);

  Event_Claim(event_CLOSE, ws_proginfo, event_ANY, Handler_CloseWindow, NULL);
  Event_Claim(event_OPEN, ws_proginfo, event_ANY, Handler_OpenWindow, NULL);

  #ifdef GERMAN
  ws_iconbarmenu=Menu_New("Stib", "Info,Spiel,Ende");
  #else
  ws_iconbarmenu=Menu_New("Stib", "Info,Play,Quit");
  #endif

  Menu_AddSubMenu(ws_iconbarmenu, 0, (menu_ptr)ws_proginfo);

  ws_iconbaricon=Icon_BarIcon("!stib", iconbar_RIGHT);

  Event_Claim(event_CLICK, window_ICONBAR, ws_iconbaricon, playgame, ws_iconbarmenu);

  ws_desktopquit=false;

  while (!ws_desktopquit)
    Event_Poll();

  Event_CloseDown();

  return 0;
}

BOOL playgame(event_pollblock *e, void *menuh)
{
  switch (e->data.mouse.button.value & 7)
  {
    case button_SELECT:
    gameloop();
    break;

    case button_MENU:
    Menu_Show(menuh, e->data.mouse.pos.x, -1);
    Event_Claim(event_MENU, event_ANY, event_ANY, handleibar, NULL);
    break;
  }
  return TRUE;
}

BOOL handleibar(event_pollblock *e, void *ref)
{
  mouse_block pointr;

  UNUSED(ref);

  switch (e->data.selection[0])
  {
    case 1:
    gameloop();
    break;

    case 2:
    ws_desktopquit=true;
    break;
  }
  Event_Release(event_MENU, event_ANY, event_ANY, handleibar, NULL);

  Wimp_GetPointerInfo(&pointr);
  if ((pointr.button.value&7)==button_ADJUST) Menu_ShowLast();

  return TRUE;
}
*/
