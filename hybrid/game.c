/* Stib (Monster Software, 1996; 1.20, 2002): a hybrid build. The game's C
 * is compiled natively from ../source - a reconstruction of the RISC OS
 * source, from the author's SDL port (xstib) and !RunImage itself -
 * patched for a 64-bit host by stib.patch. The rest of !RunImage - its
 * assembler (the sprite plotters, the fades, the FlexiSound veneers), the
 * DeskLib Wimp library it's built on and the C start-up - is translated
 * (../tools/images.py) and runs on libroger, which also runs the author's
 * FlexiSound module, translated likewise. shims.c and hyrt.c join the two.
 *
 * !Boot (run by the Filer when it first sees the application): Set
 * Stib$Path <Obey$Dir>. ; Iconsprites Stib:!Sprites
 * !Run: Set Stib$Path <Obey$Dir>. ; RMEnsure FlexiSound 1.00 RMLoad
 * Stib:FlexiSound ; Wimpslot -min 320k -max 320k ; Run Stib:!RunImage
 *
 * The program starts as the original does: the AIF header, then the
 * Shared C Library stubs, whose start-up initialises libroger's C library
 * (the heap the native C allocates from too: hybrid.h) and calls main -
 * which is the native main, as are the Wimp event handlers DeskLib calls
 * and the exit handler the library calls (hy_native).
 *
 * Stib is a Wimp task: an icon on the icon bar, Select to play, Menu for
 * Info and Quit. On the desktop (--desktop) that's up to the user;
 * otherwise wimp_idle stands in: Select on the icon starts the game, and
 * when it's left (its "desktop" button) Quit from the icon's menu ends the
 * program, as the user would - saving the hiscores, as its exit does. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <roger.h>

#ifndef FULL
#include "hyrt.h"
extern const struct roger_image runimage_image;
#define IMAGE runimage_image
#define TITLE "Stib (hybrid)"
#else
/* make full: the whole of !RunImage translated, its C too - the original
 * as a full recompilation, to compare the reconstructed C with (make
 * compare) */
extern const struct roger_image full_image;
#define IMAGE full_image
#define TITLE "Stib (translated)"
#endif
extern const struct roger_image flexisound_image;

static const struct roger_module modules[] = {
    {"../stib/!Stib/FlexiSound", &flexisound_image, 0, NULL},
    {NULL, NULL, 0, NULL},
};

static const char *const variables[] = {
    "Stib$Path", "!Stib.",
    NULL,
};

/* what !Boot did: the icon bar icon's sprite in the Wimp's pool */
static void init(void) { roger_cli("IconSprites Stib:!Sprites"); }

#ifndef FULL
/* the run ending: the hiscores saved, if the program's exit didn't (shims.c) */
static void finish(void) { hy_finish(); }
#endif

/* --seed=N: the C library's srand (the stub vector's entry, &EAA0) is
 * given N rather than the time the game seeds it with, so that the same
 * game is dealt every run */
static int seed_set;
static uint32_t seed;

/* --frame-lock: in a headless run, OS_Byte 19 waits for the next vsync,
 * as the machine does (libroger's headless OS_Byte 19 returns at once, a
 * centisecond on): the game then plays a frame a vsync whatever its code
 * costs, so that two builds' runs can be compared frame by frame */
static int frame_lock;

static int swi_hook(uint32_t n, int x)
{
    (void)x;
    if (!frame_lock || roger_interactive() || n != 0x06 || R[0] != 19) return 0;
    uint32_t save[16];
    memcpy(save, R, sizeof save);
    R[0] = 176; R[1] = 0; R[2] = 255;                  /* the vsync counter */
    roger_swi(0x06);
    uint32_t v = R[1];
    do {
        roger_idle(1000);
        R[0] = 176; R[1] = 0; R[2] = 255;
        roger_swi(0x06);
    } while (R[1] == v);
    memcpy(R, save, sizeof save);
    return 1;
}

static int arg(const char *a)
{
    if (!strcmp(a, "--frame-lock")) { frame_lock = 1; return 1; }
    if (strncmp(a, "--seed=", 7)) return 0;
    seed = (uint32_t)strtoul(a + 7, NULL, 0);
    seed_set = 1;
    return 1;
}

/* A dispatch to native code: run it as the ARM function it replaces (APCS:
 * R0 the result, R4-R11 and R13 as they were - the native code's own calls
 * into the translated code leave them so - the return to R14 with the
 * caller's flags) */
static int native_call(uint32_t a)
{
    if (a == 0xEAA0 && seed_set) R[0] = seed;
#ifndef FULL
    uint32_t r0, save[16];
    memcpy(save, R, sizeof save);
    if (!hy_native(a, &r0)) return 0;
    memcpy(R, save, sizeof save);
    R[0] = r0;
    SETPC(R[14], 1);
    return 1;
#else
    return 0;
#endif
}

/* Standing in for the user at the desktop: Select on the icon bar icon
 * (the game, until its "desktop" button), then Menu on it and Quit */
static void wimp_idle(void)
{
    static int step;
    switch (step++) {
    case 0: roger_wimp_click(-2, 0, 4); break;
    case 1: roger_wimp_click(-2, 0, 2); break;
    case 2: roger_wimp_menu((const int[]){2, -1}); break;
    default: roger_wimp_quit();
    }
}

/* Headless runs (--limit): Stib reads the mouse with OS_Mouse alone, so a
 * scenario is a --script (click:x:y, release, type:c...): make test plays
 * a little of the game. */

static const struct roger_config config = {
    .title = TITLE,
    .image_path = "../stib/!Stib/!RunImage",
    .image = &IMAGE,
    .modules = modules,
    .appdir = "../stib",
    .variables = variables,
    .command_line = "Stib:!RunImage",
    .initial_mode = 12,
    .wimpslot = 320 * 1024,
    .init = init,
    .call_addr = native_call,
#ifndef FULL
    .finish = finish,
#endif
    .wimp_idle = wimp_idle,
    .swi = swi_hook,
    .arg = arg,
    .usage = "  --seed=N          the game's random numbers seeded with N, not the time\n"
             "  --frame-lock      headless: OS_Byte 19 waits for the vsync (a frame a vsync)\n",
};

int main(int argc, char **argv) { roger_main(argc, argv, &config); }
