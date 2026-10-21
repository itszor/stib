Stib: a hybrid build
====================

Stib (Monster Software, 1996; version 1.20, 2002) on libroger, built the
way Cylon Attack is (~/games/cylon-attack): the game's own C compiled
natively, the rest of the original !RunImage translated by SuperSpider and
run on libroger. The C runs as host code; the assembler (the sprite
plotters, the zoom and cross-fade, the FlexiSound veneers), the DeskLib
Wimp library the desktop front end is built on, and the C start-up run as
translated ARM, as does the author's FlexiSound module.

The RISC OS source of the C is lost. source/ is a reconstruction of it,
made from the SDL port (xstib, which keeps most of the RISC OS lines in
comments) and checked against !RunImage function by function; a full
recompilation of the original draws the same game frame for frame (see
Verification).

Building
--------

Needs libroger (its Makefile builds libroger.a; SDL3 via pkg-config) and
gcc. The translation is included (hybrid/gen), so SuperSpider is only
needed to make it again.

    cd hybrid && make ROGER=/path/to/libroger && ./stib

    make regen      analyse and translate the images again (../tools/images.py;
                    SS=/path/to/superspider)
    make test       a headless game, seeded (build/test/run: screenshots)
    make full       stib-full: the whole of !RunImage translated, the C too
    make compare    the test game with both, screenshot by screenshot

ROGER and SS default to ~/stuff/libroger and ~/stuff/superspider. Run
./stib from hybrid/: the game's files are found at ../stib.

Running
-------

Stib is a Wimp task: an icon on the icon bar, Select to play, Menu for
Info, Play and Quit. With `--desktop` the desktop is shown and that's up
to you; otherwise the program clicks its own icon (the game starts), and
when you leave the game with its "desktop" button it quits from the icon's
menu. On the title screen click a level, then Play; the mouse aims and any
button fires; Escape or F12 gives up the game.

    --seed=N          the game's random numbers seeded with N, not the time
    --frame-lock      headless: OS_Byte 19 waits for the vsync (see below)

The hiscores are saved as the program exits, as on RISC OS - and also if
the window is closed, which the original couldn't know about (written to
libroger's save directory, savedata/, and read back from there).

Layout
------

    stib/!Stib/          the game as released (stib_final.zip, unpacked):
                         !RunImage, FlexiSound, data, Templates, !Sprites
    xstib/               the SDL port, as it was (not modified; its .o files and
                         binary, and stib_final.zip, not committed)
    source/              the reconstructed RISC OS source of the C (below)
    tools/images.py      the translated images and their ranges: analyse, gen
    tools/asmsyms.py     finds each EXPORTed asm routine of xstib/xstib/asm in !RunImage
    tools/funcmap.py     !RunImage's functions with the names Acorn C embeds
    tools/compare.py     two runs' screenshots compared (make compare)
    analysis/            SuperSpider's analyses
    hybrid/              the build: Makefile, game.c (roger_config), shims.c and
                         hyrt.c (the boundary), hybrid.h, inc/ (kernel.h, swis.h,
                         desklib.h), stib.patch, gen/ (translated)

!RunImage
---------

The C language description block at &F3F0 gives the C code's extent; the
rest was placed by asmsyms.py and the SCL stub tables:

    &8000-&8080   AIF header, zero-initialisation                translated
    &8080-&BE08   c.play, c.between, c.sat, c.levels              native
    &BE08-&BE78   the stubs' C$$code: the language's
                  initialisation, the call of _main              translated
    &BE78-&C110   c.userint: main, playgame, handleibar           native
    &C110-&E390   DeskLib (C compiled without function names)     translated
    &E390-&E72C   DeskLib's SWI veneers                           translated
    &E72C-&EC80   the stubs: start-up, library vectors            translated
    &EC80-&F3C8   s.fadecode, s.fsndcode, s.satcode               translated
    &F3C8         global_reguser (an AOF data object), data

Functions Acorn C compiled without a frame carry no name (countnum,
testarea, countunattached, doexplode, burntest, setlevelinfo,
savehiscores); DeskLib's were found by their SWIs and their callers.

How it fits together
--------------------

As in Cylon Attack (its README has the details of the boundary):

- The program starts as the original does: the AIF header, then the Shared
  C Library stubs, whose start-up initialises libroger's C library and
  calls main - which game.c's call_addr hook sends to the native main.
  DeskLib's Event_Poll calls the native Wimp handlers (playgame,
  handleibar) the same way, and the library's exit the native savehiscores
  (registered with the library's atexit by its address in !RunImage).
- C → translated: shims.c defines each asm routine and DeskLib function the
  C calls, as a call of its translation at its address in !RunImage, with
  R0-R3 and then the ARM stack (DeskLib's Event_Claim has five arguments;
  its SWI() is variadic: each argument keeps its type, so pointers can be
  passed as addresses). A native handler passed to DeskLib is passed as
  its address in !RunImage.
- Memory: one heap, the C library's in emulated memory (hybrid.h), so a
  pointer to heap data or the screen is a host pointer into MEM, passed as
  its address there. What the C passes from its stack or static data (the
  mouse position, the hiscore table, the squiggle table, the pointer block)
  is copied onto the ARM stack for the call, and back. The one structure
  of pointers the assembler reads, bminfo (the danger bar), is rebuilt
  with 32-bit addresses. Strings for DeskLib get a lasting copy in the
  heap, as it may keep the pointer (Event_Initialise's task name).
- rand and srand are the library's (through the stub vector), so a game
  is dealt as the original deals it; global_reguser and DeskLib's
  screen_mode are read where they are in the image.
- The C is compiled as Acorn C compiled it where it matters: char is
  unsigned (sat_printf indexes its glyph table by the characters, among
  them Latin-1 ones), and float arithmetic is single precision, without
  contraction.
- The C's names are linked into one object that exports only the hooks,
  so that they and libroger's (game, lev...) don't meet.

Changes to the C for the host (hybrid/stib.patch, applied to a copy): the
sprite and screen pointer arithmetic through `(int)` becomes `(intptr_t)`,
and sat_loadsprites' check of malloc's result compares its address in
emulated memory.

The reconstruction
------------------

source/ is the C as it was compiled into 1.20. xstib's C is the RISC OS
game with an SDL layer in place of the screen, sprites, sound and mouse;
most RISC OS lines are still there, commented out, and the binary decided
the rest. Each function was compared with its disassembly (the FPA
instructions decoded); what xstib changed or lost, and the binary has:

- the RISC OS layer: sprite areas and the screen as memory, the
  assembler's plotters (byte and word, masked and unmasked) through
  levelinfo's byteplotter/wordplotter, two screen banks swapped every
  frame, OS units from OS_Mouse, the hiscore name read with OS_ReadC
- c.userint: main, playgame and handleibar on DeskLib, as xstib's comment
  has them; the title pointer (Stib:data.ptr, OS_SpriteOp 36) and
  Wimp_CommandWindow / Wimp_SetMode on leaving
- danger is an int slide, and the levels' dangerinc are 8, 10, 10, 12
  and 16 (xstib: 123/14, 123/12, 123/10, 123/9, 123/8 as doubles)
- the frame time: regulator = (time since the last frame)/2, between 1 and
  4 (xstib: always 1); Escape or F12 burns the playfield
- the next shape: no acceleration (pos from 292 down to 148, 4 a frame
  per regulator; xstib's accn); its squiggle scaled by
  1-|(pos-220)/72|, with drawnext(screen, pos)
- falling Stibs are removed when they leave the play area (x < leftedge,
  x >= rightedge, y >= 232; xstib: off the screen)
- the backdrop is rand()%8 (xstib: rand()&7); the hiscore name accepts
  what 1.20 does; resethiscores waits for y or n
- sat_printf's glyph table ends "\x8f\xe4\xeb\xef\xf6\xfc" (Latin-1)

What xstib lost of the game's look and sound, which the hybrid has from
the original: the zoom onto the title screen and the cross-fade into play
(fade_zoom, fade_xfade: copies in xstib); the sprite masks (xstib takes
colour 0 as transparent, but the masks draw 77 black pixels of the 24x24
Stibs and 41 of the 16x16 ones - in three of the faces and the four
explosion frames); the FlexiSound samples (xstib has WAV replacements); and
the desktop front end.

Verification
------------

`make compare` plays the same seeded game with the hybrid and with
stib-full, the whole of !RunImage translated - the original's own compiled
C - taking a screenshot every half second, and compares them. With the
default scenario (Practise: Play, three shots, Escape, a hiscore,
Desktop) all 80 are identical, and so are the saved hiscore tables. A
screenshot taken during the zoom back to the title may differ by a few
rows: fade_zoom doesn't wait for the vsync, and starts a few hundred
instructions later in stib-full, whose C takes emulated time (the
hybrid's takes none); the default scenario's times avoid it.

Longer games, sixty shots across the playfield (explosions, falling
Stibs, the playfield moving down, the burn, a hiscore), were identical
throughout: 208 of 208 screenshots on Medium (seed 7) and on Evil (seed
42). The scenario (a level, Play, then a shot every 1.2s):

    S=9000:click:1120:272,9200:release,9600:click:160:284,9800:release
    for i in $(seq 0 59); do t=$((14000 + 1200 * i)); x=$((200 + i * 347 % 880))
        S=$S,$t:click:$x:512,$((t + 100)):release; done
    make compare SCRIPT=$S,96000:type:115,96200:type:13,104000:click:160:124,104200:release \
        LENGTH=106000 SEED=7

(Evil: the level's click at y 112 rather than 272. In zsh, write ${t} and
${x}: $t:c is a modifier.)

The comparison runs with --frame-lock: in a headless run libroger's
OS_Byte 19 doesn't wait for the vsync, so without it the game runs as
fast as its code allows, and the two builds play different numbers of
frames. stib-full needs libroger's FPEmulator (src/fpa.c), as the C's
float arithmetic is FPA instructions.

Notes
-----

- Headless runs (`--limit`) are deterministic. Stib reads the mouse only
  with OS_Mouse, so a scenario is a --script (click, release, type...);
  roger_config's clicks would only reach the mouse event.
- Emulated time advances only with ARM instructions: the native C takes
  none (see Verification).
