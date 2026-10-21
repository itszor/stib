#!/usr/bin/env python3
"""The translated images of Stib and the steps that build them.

    images.py analyse   SuperSpider (superspider analyse --full) on each image:
                        analysis/<name>.json (analysis/<name>.roots, if there,
                        adds entry points)
    images.py gen       superspider gen for each image -> hybrid/gen/<name>/

The game is a hybrid: its own C is compiled natively (../source, hybrid/),
and the rest of !RunImage is translated: the C start-up (the AIF header and
the Shared C Library stubs, whose vectors libroger's C library fills),
DeskLib (the Wimp library the front end is built on: C compiled without
function names, and its SWI veneers), and the game's assembler (s.fadecode,
s.fsndcode, s.satcode; found by asmsyms.py). The game's C between them is
not translated: a dispatch to it (main, called by the C start-up; the
Wimp event handlers DeskLib calls; savehiscores, called at exit) goes to
hybrid/game.c's call_addr hook, which runs the native function.

The image's layout (the C language description block at &F3F0 gives the C
code's extent, &8080-&E390):

  &8000-&8080   AIF header, zero-initialisation                 translated
  &8080-&BE08   c.play, c.between, c.sat, c.levels               native
  &BE08-&BE78   the stubs' C$$code: "C", the language's
                initialisation, the call of _main               translated
  &BE78-&C110   c.userint (main, playgame, handleibar)           native
  &C110-&E390   DeskLib's C                                      translated
  &E390-&E72C   DeskLib's SWI veneers                            translated
  &E72C-&EC80   the stubs: start-up, library vectors            translated
  &EC80-&F3C8   s.fadecode, s.fsndcode, s.satcode                translated
  &F3C8-&F458   global_reguser (an AOF data object), the
                language description block, data                -

Images:
  runimage    !Stib.!RunImage at &8000: the ranges below only
  flexisound  !Stib.FlexiSound (the author's own sound module) at &1800000
"""
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOP = os.path.dirname(HERE)
APP = os.path.join(TOP, 'stib', '!Stib')
SS = os.environ.get('SUPERSPIDER', os.path.expanduser('~/stuff/superspider'))

RANGES = [
    (0x08000, 0x08080),    # AIF header, zero-initialisation
    (0x0be08, 0x0be78),    # the stubs' C$$code: language initialisation, _main
    (0x0c110, 0x0f3c8),    # DeskLib, its SWI veneers, the stubs, the assembler
]

IMAGES = [
    # name, file (in !Cylon), base, module
    ('runimage', '!RunImage', 0x8000, False),
    ('flexisound', 'FlexiSound', 0x1800000, True),
]


def run(cmd):
    print('+', ' '.join(cmd), flush=True)
    subprocess.run(cmd, check=True)


def exe():
    subprocess.run(['dune', 'build'], cwd=SS, check=True)
    return os.path.join(SS, '_build/default/bin/main.exe')


def analyse():
    ss = exe()
    an = os.path.join(TOP, 'analysis')
    os.makedirs(an, exist_ok=True)
    for name, path, base, module in IMAGES:
        cmd = [ss, 'analyse', os.path.join(APP, path), '%x' % base, '--full',
               '-o', os.path.join(an, name + '.json')]
        rf = os.path.join(an, name + '.roots')
        if os.path.exists(rf):
            cmd += ['--roots', rf]
        if module:
            cmd += ['--module']
        run(cmd)


def gen():
    ss = exe()
    for name, path, base, module in IMAGES:
        out = os.path.join(TOP, 'hybrid', 'gen', name)
        os.makedirs(out, exist_ok=True)
        for f in os.listdir(out):
            os.remove(os.path.join(out, f))
        cmd = [ss, 'gen', os.path.join(APP, path), '%x' % base,
               os.path.join(TOP, 'analysis', name + '.json'), '--c', out,
               '--prefix', name[0].upper() + name[1:], '--tabprefix', name, '--files', '4']
        if name == 'runimage':
            for lo, hi in RANGES:
                cmd += ['--range', '%x-%x' % (lo, hi)]
        run(cmd)


if __name__ == '__main__':
    what = sys.argv[1] if len(sys.argv) > 1 else ''
    if what == 'analyse':
        analyse()
    elif what == 'gen':
        gen()
    else:
        sys.exit(__doc__)
