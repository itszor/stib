/* The boundary between the game's C, compiled natively, and the translated
 * code - the assembler, DeskLib and the C start-up from !RunImage, run on
 * libroger (hyrt.h) - and the C library calls hybrid.h sends to the
 * emulated machine. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <roger.h>
#include "hyrt.h"
#include "inc/kernel.h"

#define MEMSIZE 0x4000000u

/* the image's Shared C Library stub vector (chunk 2, at &E99C): the
 * entries libroger's C library serves, in the library's order */
#define CLIB_RAND    0xEA9Cu
#define CLIB_SRAND   0xEAA0u
#define CLIB_CALLOC  0xEAA4u
#define CLIB_FREE    0xEAA8u
#define CLIB_MALLOC  0xEAACu
#define CLIB_REALLOC 0xEAB0u
#define CLIB_ATEXIT  0xEAB8u
#define CLIB_EXIT    0xEABCu

uint32_t hy_arm(const void *p)
{
    if (!p) return 0;
    const uint8_t *q = p;
    if (q < MEM || q >= MEM + MEMSIZE) {
        fprintf(stderr, "hybrid: %p is not in the emulated memory\n", p);
        roger_finish("native pointer passed to the translated code");
    }
    return (uint32_t)(q - MEM);
}

void *hy_host(uint32_t a) { return a ? MEM + (a & (MEMSIZE - 1)) : NULL; }
uint32_t hy_reg(int k) { return R[k]; }

int hy_inmem(const void *p) { return (const uint8_t *)p >= MEM && (const uint8_t *)p < MEM + MEMSIZE; }

void hy_enter(struct hy_frame *f)
{
    memcpy(f->save, R, sizeof f->save);
    f->flags = NF << 3 | ZF << 2 | CF << 1 | VF;
    f->nback = 0;
}

static uint32_t push(const void *p, uint32_t size)
{
    R[13] = (R[13] - size) & ~7u;
    memcpy(MEM + R[13], p, size);
    return R[13];
}

uint32_t hy_in(struct hy_frame *f, const void *p, uint32_t size)
{
    (void)f;
    if (!p) return 0;
    return hy_inmem(p) ? hy_arm(p) : push(p, size);
}

uint32_t hy_io(struct hy_frame *f, void *p, uint32_t size)
{
    if (!p) return 0;
    if (hy_inmem(p)) return hy_arm(p);
    uint32_t a = push(p, size);
    if (f->nback == (int)(sizeof f->back / sizeof f->back[0])) roger_finish("hybrid: too many arguments copied back");
    f->back[f->nback].host = p;
    f->back[f->nback].arm = a;
    f->back[f->nback].size = size;
    f->nback++;
    return a;
}

uint32_t hy_str(struct hy_frame *f, const char *s) { return hy_in(f, s, s ? (uint32_t)strlen(s) + 1 : 0); }

static void leave(struct hy_frame *f)
{
    for (int k = 0; k < f->nback; k++)
        if (memcmp(f->back[k].host, MEM + f->back[k].arm, f->back[k].size))
            memcpy(f->back[k].host, MEM + f->back[k].arm, f->back[k].size);
    memcpy(R, f->save, sizeof f->save);
    NF = f->flags >> 3 & 1; ZF = f->flags >> 2 & 1; CF = f->flags >> 1 & 1; VF = f->flags & 1;
}

uint32_t hy_call(struct hy_frame *f, uint32_t addr, int n, const uint32_t *args)
{
    if (n > 4) {                               /* the fifth argument on, at [sp], [sp+4]... */
        R[13] = (R[13] - 4 * (uint32_t)(n - 4)) & ~7u;
        for (int k = 4; k < n; k++) WR32(R[13] + 4 * (uint32_t)(k - 4), args[k]);
    }
    for (int k = 0; k < 4; k++) R[k] = k < n ? args[k] : 0;
    roger_call_program(addr, ROGER_HANDLER_RET);
    uint32_t r = R[0];
    leave(f);
    return r;
}

uint32_t hy_run(struct hy_frame *f, uint32_t addr, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3)
{
    const uint32_t args[4] = {r0, r1, r2, r3};
    return hy_call(f, addr, 4, args);
}

/* ------------------------------------------------------------ the C library */
static uint32_t clib(uint32_t entry, uint32_t a0, uint32_t a1)
{
    struct hy_frame f;
    hy_enter(&f);
    return hy_run(&f, entry, a0, a1, 0, 0);
}

void *hy_malloc(size_t n) { return hy_host(clib(CLIB_MALLOC, (uint32_t)n, 0)); }
void *hy_calloc(size_t n, size_t size) { return hy_host(clib(CLIB_CALLOC, (uint32_t)n, (uint32_t)size)); }
void *hy_realloc(void *p, size_t n) { return hy_host(clib(CLIB_REALLOC, hy_arm(p), (uint32_t)n)); }
void hy_free(void *p) { if (p) clib(CLIB_FREE, hy_arm(p), 0); }
int hy_rand(void) { return (int)clib(CLIB_RAND, 0, 0); }
void hy_srand(unsigned seed) { clib(CLIB_SRAND, seed, 0); }

uint32_t hy_strdup(const char *s)
{
    static struct { const char *host; uint32_t arm; } made[32];
    static int n;
    for (int k = 0; k < n; k++)
        if (made[k].host == s) return made[k].arm;
    char *p = hy_malloc(strlen(s) + 1);
    strcpy(p, s);
    if (n < 32) { made[n].host = s; made[n].arm = hy_arm(p); n++; }
    return hy_arm(p);
}

/* exit handlers are the library's, as on RISC OS: registered by the address
 * the native function stands at in !RunImage, which the library's exit
 * calls and game.c's call_addr hook sends to the native function */
int hy_atexit(void (*fn)(void))
{
    uint32_t a = hy_fnaddr(fn);
    if (!a) roger_finish("hybrid: atexit with a function not in !RunImage");
    return (int)clib(CLIB_ATEXIT, a, 0);
}

void hy_exit(int status)
{
    clib(CLIB_EXIT, (uint32_t)status, 0);
    roger_finish("exit");                 /* (the library's exit doesn't return) */
}

/* a file saved as the game's sat_savefile saves one (OS_File 10, type
 * &FFD), from native data: for the run's end, when no translated code may
 * run (game.c's finish hook) - the data is copied below the ARM stack */
void hy_savefile(const char *name, const void *data, uint32_t size)
{
    struct hy_frame f;
    hy_enter(&f);
    uint32_t a = (R[13] - 0x1000 - size) & ~3u;
    memcpy(MEM + a, data, size);
    R[0] = 10; R[1] = roger_scratch(name); R[2] = 0xFFD; R[4] = a; R[5] = a + size;
    roger_swi(0x08 | 0x20000u);
    leave(&f);
}

/* ------------------------------------------------------------ the kernel */
static _kernel_oserror last_error;

_kernel_oserror *_kernel_swi(int no, _kernel_swi_regs *in, _kernel_swi_regs *out)
{
    struct hy_frame f;
    hy_enter(&f);
    for (int r = 0; r < 10; r++) R[r] = (uint32_t)in->r[r];
    roger_swi((uint32_t)no | 0x20000u);
    _kernel_oserror *err = NULL;
    if (VF) {
        last_error.errnum = (int)RD32(R[0]);
        snprintf(last_error.errmess, sizeof last_error.errmess, "%s", (char *)MEM + R[0] + 4);
        err = &last_error;
    }
    uint32_t o[10];
    memcpy(o, R, sizeof o);
    leave(&f);
    if (!err) for (int r = 0; r < 10; r++) out->r[r] = (int)o[r];
    return err;
}

int _kernel_osbyte(int op, int x, int y)
{
    struct hy_frame f;
    hy_enter(&f);
    R[0] = (uint32_t)op; R[1] = (uint32_t)x; R[2] = (uint32_t)y;
    roger_swi(0x06 | 0x20000u);
    int r = VF ? -2 : (int)((R[1] & 0xFF) | (R[2] & 0xFF) << 8);
    leave(&f);
    return r;
}
