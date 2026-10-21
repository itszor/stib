#!/usr/bin/env python3
"""Compare two headless runs' screenshots (make compare): the hybrid's and
the full recompilation's, taken at the same emulated times.

    compare.py DIR1 DIR2

For each screenshot-N.bmp in both, the number of pixels that differ; the
runs agree if none do (the same game, drawn the same, at the same times)."""
import os
import sys
from PIL import Image, ImageChops

a, b = sys.argv[1], sys.argv[2]
n = 1
same = diff = 0
while os.path.exists(os.path.join(a, 'screenshot-%d.bmp' % n)) and \
        os.path.exists(os.path.join(b, 'screenshot-%d.bmp' % n)):
    ia = Image.open(os.path.join(a, 'screenshot-%d.bmp' % n)).convert('RGB')
    ib = Image.open(os.path.join(b, 'screenshot-%d.bmp' % n)).convert('RGB')
    if ia.size != ib.size:
        print('screenshot-%d: sizes differ, %s and %s' % (n, ia.size, ib.size))
        diff += 1
    else:
        d = ImageChops.difference(ia, ib).convert('L').point(lambda v: 255 if v else 0)
        k = d.histogram()[255]
        if k:
            print('screenshot-%d: %d pixels differ, in %s' % (n, k, d.getbbox()))
            diff += 1
        else:
            same += 1
    n += 1
print('%d screenshots the same, %d different' % (same, diff))
sys.exit(1 if diff else 0)
