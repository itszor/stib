/* The native C's side of the boundary with the translated code (hyrt.c):
 * the assembler, DeskLib and the C start-up. Kept apart from roger.h, so
 * that the game's headers and libroger's don't meet.
 *
 * A call into the translated code (shims.c) opens a frame, passes its
 * pointer arguments, calls, and returns R0:
 *
 *     struct hy_frame f;
 *     hy_enter(&f);
 *     return hy_run(&f, ADDR, hy_in(&f, pos, sizeof *pos), n, 0, 0);
 *
 * The emulated machine's memory (MEM) holds the one heap (hybrid.h), so a
 * pointer into it is passed as its address there. A pointer to anything
 * else (the native stack, the C's static data) is copied onto the ARM
 * stack for the call: hy_in for what is only read, hy_io for what may also
 * be written (copied back afterwards). The frame keeps the registers as
 * they were, and the ARM stack pointer, and puts them back. hy_call passes
 * any number of arguments: R0-R3, then the ARM stack, as APCS does. */
#ifndef HYRT_H
#define HYRT_H
#include <stdint.h>

struct hy_frame {
    uint32_t save[16], flags;
    int nback;
    struct { void *host; uint32_t arm, size; } back[8];
};

void hy_enter(struct hy_frame *f);
uint32_t hy_in(struct hy_frame *f, const void *p, uint32_t size);
uint32_t hy_io(struct hy_frame *f, void *p, uint32_t size);
uint32_t hy_str(struct hy_frame *f, const char *s);
uint32_t hy_run(struct hy_frame *f, uint32_t addr, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3);
uint32_t hy_call(struct hy_frame *f, uint32_t addr, int n, const uint32_t *args);

uint32_t hy_arm(const void *p);         /* a pointer into MEM: its address there (NULL: 0) */
void *hy_host(uint32_t a);              /* ...and back (0: NULL) */
int hy_inmem(const void *p);            /* is p in MEM? */
uint32_t hy_reg(int k);                 /* a register: arguments of a call into native code */
uint32_t hy_strdup(const char *s);      /* a lasting copy of a string in the heap (for
                                           DeskLib, which may keep the pointer), made once */

void hy_savefile(const char *name, const void *data, uint32_t size);   /* OS_File 10, at the end */

/* native code the ARM code calls (shims.c): 1 if a is one, which has been
 * run with its arguments from R0-R3, and *r0 its result */
int hy_native(uint32_t a, uint32_t *r0);
/* ...and the address a native function stands at in !RunImage (0: none) */
uint32_t hy_fnaddr(void (*fn)(void));
/* the run ending other than by the program's exit (game.c's finish hook) */
void hy_finish(void);

#endif
