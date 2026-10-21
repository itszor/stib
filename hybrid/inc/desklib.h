/* DeskLib, as the native C sees it: the types, constants and functions the
 * game uses, with DeskLib's names and values. The library itself is the
 * translation of the one linked into !RunImage; shims.c calls it there.
 * Included for each "DeskLib:xxx.h" the source names. */
#ifndef HY_DESKLIB_H
#define HY_DESKLIB_H

typedef int BOOL;
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#define UNUSED(x) ((x) = (x))

typedef struct { int errnum; char errmess[252]; } os_error;

typedef int window_handle;
typedef int icon_handle;
typedef int event_type;
typedef struct menu_block *menu_ptr;            /* a menu built by Menu_New (emulated memory) */
typedef struct sprite_areainfo *sprite_area;    /* a sprite area (emulated memory) */

typedef struct { int x, y; } wimp_point;
typedef union {
    unsigned int value;
    struct { unsigned int adjust : 1, menu : 1, select : 1; } data;
} button_state;

typedef struct {
    wimp_point pos;
    button_state button;
    window_handle window;
    icon_handle icon;
} mouse_block;

typedef struct {
    event_type type;
    union {
        mouse_block mouse;
        int selection[10];
        int bytes[63];
    } data;
} event_pollblock;

typedef BOOL (*event_handler)(event_pollblock *event, void *reference);

#define event_ANY      (-99)
#define event_OPEN     2
#define event_CLOSE    3
#define event_CLICK    6
#define event_MENU     9

#define window_ICONBAR (-2)
#define iconbar_RIGHT  (-1)

#define button_ADJUST  1
#define button_MENU    2
#define button_SELECT  4

#define SWI_OS_ChangeDynamicArea 0x2A
#define SWI_OS_SpriteOp          0x2E
#define SWI_OS_ReadDynamicArea   0x5C

/* Screen */
#define screen_mode (*(int *)hy_desklib_screen_mode())
int *hy_desklib_screen_mode(void);
BOOL Screen_CacheModeInfo(void);

/* Resource, Event, EventMsg, Template, Window, Icon, Menu, Handler */
void Resource_InitialisePath(char *respath);
BOOL Event_Initialise(char *taskname);
void Event_Poll(void);
void Event_CloseDown(void);
BOOL Event_Claim(event_type type, window_handle window, icon_handle icon,
                 event_handler handler, void *reference);
BOOL Event_Release(event_type type, window_handle window, icon_handle icon,
                   event_handler handler, void *reference);
void EventMsg_Initialise(void);
void Template_Initialise(void);
void Template_LoadFile(char *leafname);
window_handle Window_Create(char *windowname, int maxtitlesize);
void Icon_SetText(window_handle w, icon_handle i, char *text);
icon_handle Icon_BarIcon(char *spritename, window_handle pos);
menu_ptr Menu_New(char *title, char *description);
void Menu_AddSubMenu(menu_ptr menu, int entry, menu_ptr submenu);
void Menu_Show(menu_ptr menu, int x, int y);
void Menu_ShowLast(void);
BOOL Handler_CloseWindow(event_pollblock *event, void *reference);
BOOL Handler_OpenWindow(event_pollblock *event, void *reference);

/* Kbd, File, Sprite */
BOOL Kbd_KeyDown(int keynum);
int File_Size(char *filename);
sprite_area Sprite_LoadFile(char *filename);

/* the Wimp's SWI veneers */
os_error *Wimp_GetPointerInfo(mouse_block *ptrinfo);
os_error *Wimp_CommandWindow(int open);
os_error *Wimp_SetMode(int mode);

/* SWI: DeskLib's general SWI veneer (numregsin inputs, then numregsout
 * pointers to receive R0..: NULL to ignore one). Each argument keeps its
 * type (hy_swiarg), so a pointer can be passed as an address in the
 * emulated memory. */
#include <stddef.h>
#include <stdint.h>
typedef struct { uint64_t v; int kind; } hy_swiarg;    /* kind: 0 int, 1 string, 2 int *, 3 other pointer */
#define HY_KIND(x) _Generic((x), char *: 1, const char *: 1, int *: 2, \
                            sprite_area: 3, void *: 3, default: 0)
#define HY_SA(x) ((hy_swiarg){(uint64_t)(uintptr_t)(x), HY_KIND(x)})
#define HY_M1(f, a) f(a)
#define HY_M2(f, a, ...) f(a), HY_M1(f, __VA_ARGS__)
#define HY_M3(f, a, ...) f(a), HY_M2(f, __VA_ARGS__)
#define HY_M4(f, a, ...) f(a), HY_M3(f, __VA_ARGS__)
#define HY_M5(f, a, ...) f(a), HY_M4(f, __VA_ARGS__)
#define HY_M6(f, a, ...) f(a), HY_M5(f, __VA_ARGS__)
#define HY_M7(f, a, ...) f(a), HY_M6(f, __VA_ARGS__)
#define HY_M8(f, a, ...) f(a), HY_M7(f, __VA_ARGS__)
#define HY_M9(f, a, ...) f(a), HY_M8(f, __VA_ARGS__)
#define HY_M10(f, a, ...) f(a), HY_M9(f, __VA_ARGS__)
#define HY_MN(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define HY_MAP(f, ...) HY_MN(__VA_ARGS__, HY_M10, HY_M9, HY_M8, HY_M7, HY_M6, HY_M5, \
                             HY_M4, HY_M3, HY_M2, HY_M1, 0)(f, __VA_ARGS__)
os_error *hy_SWI(int numregsin, int numregsout, int swicode, int nargs, const hy_swiarg *args);
#define SWI(in, out, swi, ...) \
    hy_SWI((in), (out), (swi), sizeof((hy_swiarg[]){HY_MAP(HY_SA, __VA_ARGS__)}) / sizeof(hy_swiarg), \
           (const hy_swiarg[]){HY_MAP(HY_SA, __VA_ARGS__)})

#endif
