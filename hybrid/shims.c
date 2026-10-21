/* The translated code, as the native C sees it: each assembler routine the
 * C calls (s.satcode, s.fadecode, s.fsndcode) and each DeskLib function,
 * at its address in !RunImage, called with its arguments in R0-R3 and then
 * on the ARM stack (hyrt.h), returning R0. And the other way: the native
 * functions the translated code calls - main (from the C start-up), the
 * Wimp event handlers (from DeskLib's Event_Poll) and savehiscores (from
 * the library's exit) - by their addresses in !RunImage.
 *
 * Pointer arguments are to the emulated memory (the heap, the screen) or,
 * for what the C passes from its stack or static data (the mouse position,
 * the hiscore table, the squiggle table, the pointer block), copied there.
 * The one structure of pointers the assembler reads, bminfo, is rebuilt
 * with 32-bit addresses. Strings for DeskLib are given a lasting copy in
 * the heap, as it may keep the pointer (Event_Initialise's task name). */
#include <string.h>
#include "sat.h"
#include "fsound.h"
#include "fade.h"
#include "play.h"
#include "between.h"
#include "global.h"
#include "userint.h"
#include "desklib.h"
#include "hyrt.h"

#define A(p) hy_arm(p)
#define IN(p) hy_in(&f, (p), sizeof *(p))
#define IO(p) hy_io(&f, (p), sizeof *(p))
#define S(s) (hy_inmem(s) ? hy_arm(s) : hy_strdup(s))
#define CALL(addr, a0, a1, a2, a3) hy_run(&f, (addr), (uint32_t)(a0), (uint32_t)(a1), (uint32_t)(a2), (uint32_t)(a3))
#define CALLN(addr, ...) hy_call(&f, (addr), sizeof((uint32_t[]){__VA_ARGS__}) / 4, (const uint32_t[]){__VA_ARGS__})
#define SHIM(...) struct hy_frame f; hy_enter(&f); __VA_ARGS__

/* ------------------------------------------------------------ s.fadecode */
void fade_zoom(int *dest, int *source) { SHIM(CALL(0x0EC80, A(dest), A(source), 0, 0);) }
void fade_xfade(int *dest, int *source, int *coltab) { SHIM(CALL(0x0EE04, A(dest), A(source), A(coltab), 0);) }

/* ------------------------------------------------------------ s.fsndcode */
int flexisound_add(char *name) { SHIM(return (int)CALL(0x0EE7C, hy_str(&f, name), 0, 0, 0);) }
void flexisound_remove(int n) { SHIM(CALL(0x0EE84, n, 0, 0, 0);) }
void flexisound_play(int n, int ch, int vol, int pitch) { SHIM(CALL(0x0EE8C, n, ch, vol, pitch);) }

/* ------------------------------------------------------------ s.satcode */
/* a load into the C's static data (the hiscore table): copied back, the
 * file's length */
void sat_loadfile(char *name, int *to)
{
    if (hy_inmem(to)) { SHIM(CALL(0x0EE94, hy_str(&f, name), A(to), 0, 0);) return; }
    int size = File_Size(name);
    SHIM(CALL(0x0EE94, hy_str(&f, name), hy_io(&f, to, (uint32_t)size), 0, 0);)
}
void sat_savefile(char *name, int *base, int size)
{
    SHIM(CALL(0x0EEB0, hy_str(&f, name), hy_in(&f, base, (uint32_t)size), size, 0);)
}
void sat_screenmode(int mode) { SHIM(CALL(0x0EEE0, mode, 0, 0, 0);) }
int *sat_getscreenbase(void) { SHIM(return hy_host(CALL(0x0EEF8, 0, 0, 0, 0));) }
int sat_swapbanks(int bank) { SHIM(return (int)CALL(0x0EF1C, bank, 0, 0, 0);) }
void sat_resetscreen(int *screen) { SHIM(CALL(0x0EF4C, A(screen), 0, 0, 0);) }
void sat_mouseop(int *x, int *y, int *b) { SHIM(CALL(0x0EF80, IO(x), IO(y), IO(b), 0);) }
int sat_timeop(void) { SHIM(return (int)CALL(0x0EFA4, 0, 0, 0, 0);) }

#define PLOTTER(name, addr) \
    void name(int *sprite, int *screen, int x, int y) { SHIM(CALL(addr, A(sprite), A(screen), x, y);) }
PLOTTER(sat_plot_32x32_noclip_unmask_word, 0x0EFAC)
PLOTTER(sat_plot_24x24_noclip_unmask_word, 0x0F038)
PLOTTER(sat_plot_12x24_noclip_unmask_word, 0x0F064)
PLOTTER(sat_plot_24x24_noclip_mask_word, 0x0F090)
PLOTTER(sat_plot_24x24_noclip_mask_byte, 0x0F10C)
PLOTTER(sat_plot_24x24_clip_mask_byte, 0x0F150)
PLOTTER(sat_plot_16x16_noclip_mask_word, 0x0F154)
PLOTTER(sat_plot_16x16_noclip_mask_byte, 0x0F1C0)
PLOTTER(sat_plot_64x32_noclip_mask_word, 0x0F234)
PLOTTER(sat_plot_32x32_noclip_mask_word, 0x0F290)
PLOTTER(sat_plot_XxY_noclip_mask_byte, 0x0F2EC)

void sat_plot_backdrop(int *sprite, int *screen) { SHIM(CALL(0x0EFD8, A(sprite), A(screen), 0, 0);) }
void sat_plot_barmiddle(bminfo *info)
{
    uint32_t b[5] = {A(info->sprite), A(info->screen), (uint32_t)info->xpos, (uint32_t)info->ypos,
                     (uint32_t)info->length};
    SHIM(CALL(0x0F330, hy_in(&f, b, sizeof b), 0, 0, 0);)
}
void sat_plot_squiggled(int *sprite, int *screen, squiggleinfo *info)
{
    SHIM(CALL(0x0F374, A(sprite), A(screen), IN(info), 0);)
}

/* ------------------------------------------------------------ DeskLib */
/* global_reguser: the "user" AOF data object linked into !RunImage (the
 * registration text), a pointer at &F3C8; DeskLib's screen_mode at &F6C8 */
#define REGUSER 0x0F3C8u
#define SCREEN_MODE 0x0F6C8u
char *global_reguser;
int *hy_desklib_screen_mode(void) { return hy_host(SCREEN_MODE); }

void Resource_InitialisePath(char *respath) { SHIM(CALL(0x0C47C, S(respath), 0, 0, 0);) }
BOOL Event_Initialise(char *taskname) { SHIM(return (BOOL)CALL(0x0CF58, S(taskname), 0, 0, 0);) }
void EventMsg_Initialise(void) { SHIM(CALL(0x0D28C, 0, 0, 0, 0);) }
BOOL Screen_CacheModeInfo(void) { SHIM(return (BOOL)CALL(0x0C244, 0, 0, 0, 0);) }
void Template_Initialise(void) { SHIM(CALL(0x0D2EC, 0, 0, 0, 0);) }
void Template_LoadFile(char *leafname) { SHIM(CALL(0x0D47C, S(leafname), 0, 0, 0);) }
window_handle Window_Create(char *windowname, int maxtitlesize)
{
    SHIM(return (window_handle)CALL(0x0D670, S(windowname), maxtitlesize, 0, 0);)
}
void Icon_SetText(window_handle w, icon_handle i, char *text) { SHIM(CALL(0x0D728, w, i, S(text), 0);) }
icon_handle Icon_BarIcon(char *spritename, window_handle pos)
{
    SHIM(return (icon_handle)CALL(0x0DC28, S(spritename), pos, 0, 0);)
}
void Event_Poll(void) { SHIM(CALL(0x0D048, 0, 0, 0, 0);) }
void Event_CloseDown(void) { SHIM(CALL(0x0D084, 0, 0, 0, 0);) }
/* a handler given to DeskLib: its address in !RunImage (fns, below) */
static uint32_t handler_addr(event_handler handler)
{
    uint32_t a = hy_fnaddr((void (*)(void))handler);
    if (handler && !a) { fprintf(stderr, "hybrid: an event handler not in !RunImage\n"); abort(); }
    return a;
}

BOOL Event_Claim(event_type type, window_handle window, icon_handle icon, event_handler handler,
                 void *reference)
{
    SHIM(return (BOOL)CALLN(0x0C6FC, type, window, icon, handler_addr(handler), A(reference));)
}
BOOL Event_Release(event_type type, window_handle window, icon_handle icon, event_handler handler,
                   void *reference)
{
    SHIM(return (BOOL)CALLN(0x0CB00, type, window, icon, handler_addr(handler), A(reference));)
}
menu_ptr Menu_New(char *title, char *description)
{
    SHIM(return hy_host(CALL(0x0DA8C, S(title), S(description), 0, 0));)
}
/* the submenu may be a window handle, cast (a dialogue box) */
void Menu_AddSubMenu(menu_ptr menu, int entry, menu_ptr submenu)
{
    uint32_t sub = hy_inmem(submenu) ? A(submenu) : (uint32_t)(uintptr_t)submenu;
    SHIM(CALL(0x0DC10, A(menu), entry, sub, 0);)
}
void Menu_Show(menu_ptr menu, int x, int y) { SHIM(CALL(0x0DD94, A(menu), x, y, 0);) }
void Menu_ShowLast(void) { SHIM(CALL(0x0DDEC, 0, 0, 0, 0);) }
BOOL Kbd_KeyDown(int keynum) { SHIM(return (BOOL)CALL(0x0E390, keynum, 0, 0, 0);) }
int File_Size(char *filename) { SHIM(return (int)CALL(0x0C438, hy_str(&f, filename), 0, 0, 0);) }
sprite_area Sprite_LoadFile(char *filename) { SHIM(return hy_host(CALL(0x0C1D0, hy_str(&f, filename), 0, 0, 0));) }
os_error *Wimp_GetPointerInfo(mouse_block *ptrinfo) { SHIM(return hy_host(CALL(0x0E3D4, IO(ptrinfo), 0, 0, 0));) }
os_error *Wimp_CommandWindow(int open) { SHIM(return hy_host(CALL(0x0E3C4, open, 0, 0, 0));) }
os_error *Wimp_SetMode(int mode) { SHIM(return hy_host(CALL(0x0E3B4, mode, 0, 0, 0));) }

/* the handlers the program passes DeskLib (by address, below): never
 * called natively */
BOOL Handler_CloseWindow(event_pollblock *event, void *reference) { (void)event; (void)reference; return FALSE; }
BOOL Handler_OpenWindow(event_pollblock *event, void *reference) { (void)event; (void)reference; return FALSE; }

os_error *hy_SWI(int numregsin, int numregsout, int swicode, int nargs, const hy_swiarg *args)
{
    uint32_t a[3 + 16];
    struct hy_frame f;
    hy_enter(&f);
    a[0] = (uint32_t)numregsin; a[1] = (uint32_t)numregsout; a[2] = (uint32_t)swicode;
    for (int k = 0; k < nargs && k < 16; k++) {
        void *p = (void *)(uintptr_t)args[k].v;
        switch (args[k].kind) {
        case 0: a[3 + k] = (uint32_t)args[k].v; break;                 /* an int */
        case 1: a[3 + k] = hy_inmem(p) ? A(p) : hy_str(&f, p); break;   /* a string */
        case 2: a[3 + k] = hy_io(&f, p, sizeof(int)); break;           /* an int to receive */
        default: a[3 + k] = A(p);                                      /* a pointer into MEM */
        }
    }
    return hy_host(hy_call(&f, 0x0C110, 3 + nargs, a));
}

/* ------------------------------------------------------------ native code */
int stib_main(void);

static const struct { void (*fn)(void); uint32_t addr; } fns[] = {
    {(void (*)(void))savehiscores, 0x0B7C4},
    {(void (*)(void))playgame, 0x0C014},
    {(void (*)(void))handleibar, 0x0C094},
    {(void (*)(void))Handler_CloseWindow, 0x0D804},
    {(void (*)(void))Handler_OpenWindow, 0x0D82C},
};

uint32_t hy_fnaddr(void (*fn)(void))
{
    if (!fn) return 0;
    for (unsigned k = 0; k < sizeof fns / sizeof fns[0]; k++)
        if (fns[k].fn == fn) return fns[k].addr;
    return 0;
}

/* The hiscores are saved as the program exits (savehiscores, an atexit
 * handler). A run that ends otherwise - the window closed, the instruction
 * limit - saves them too, once main has loaded them: with the OS's call
 * sat_savefile makes, as no translated code may run then. */
static int started, saved;

void hy_finish(void)
{
    if (started && !saved) hy_savefile("Stib:data.5x10", &scoretab, sizeof scoretab);
}

int hy_native(uint32_t a, uint32_t *r0)
{
    switch (a) {
    case 0x0BE78: {                     /* main, from _main */
        uint32_t p;
        started = 1;
        memcpy(&p, hy_host(REGUSER), 4);
        global_reguser = hy_host(p);
        *r0 = (uint32_t)stib_main();
        return 1;
    }
    case 0x0C014: *r0 = (uint32_t)playgame(hy_host(hy_reg(0)), hy_host(hy_reg(1))); return 1;
    case 0x0C094: *r0 = (uint32_t)handleibar(hy_host(hy_reg(0)), hy_host(hy_reg(1))); return 1;
    case 0x0B7C4: savehiscores(); saved = 1; *r0 = 0; return 1;
    }
    return 0;
}
