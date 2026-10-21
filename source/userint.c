#include <stdlib.h>

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

#include "sat.h"
#include "play.h"
#include "between.h"
#include "global.h"
#include "userint.h"

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

  /* hiscore handling */
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
