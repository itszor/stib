/* Included before every file of the game's C (gcc -include): the C library
 * calls that must reach the emulated machine rather than the host's.
 *
 * - the heap: the assembler reads and writes what the C allocates (sprites,
 *   the fade's colour table, the pointer's sprite area), so there is one
 *   heap, the Shared C Library's in emulated memory (libroger's), reached
 *   through the image's own stub vector. A pointer into it is a host
 *   pointer into MEM.
 * - rand and srand: the library's generator, so that a game is dealt as the
 *   original deals it.
 * - exit and atexit: the library's. An exit handler (savehiscores) is
 *   registered by its address in !RunImage, and runs as the library exits,
 *   as on RISC OS - including after main returns.
 *
 * Everything else (strings, sprintf, maths...) is the host's. */
#ifndef HYBRID_H
#define HYBRID_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

void *hy_malloc(size_t n);
void *hy_calloc(size_t n, size_t size);
void *hy_realloc(void *p, size_t n);
void hy_free(void *p);
int hy_rand(void);
void hy_srand(unsigned seed);
void hy_exit(int status) __attribute__((noreturn));
int hy_atexit(void (*fn)(void));
uint32_t hy_arm(const void *p);

#define malloc(n) hy_malloc(n)
#define calloc(n, s) hy_calloc(n, s)
#define realloc(p, n) hy_realloc(p, n)
#define free(p) hy_free(p)
#define rand() hy_rand()
#define srand(s) hy_srand(s)
#define exit(s) hy_exit(s)
#define atexit(f) hy_atexit(f)

#endif
