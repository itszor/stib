#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "kernel.h"

#include "sat.h"
#include "fsound.h"
#include "fade.h"
#include "play.h"
#include "between.h"
#include "levels.h"
#include "global.h"


void setlevelinfo(int levelnum)
{
  switch (levelnum)
  {
    case 0:
      lev.blocksize.x=24;
      lev.blocksize.y=24;
      lev.add.x=0;
      lev.add.y=0;
      lev.indent.x=56;
      lev.indent.y=8;
      lev.gridsize.x=8;
      lev.gridsize.y=10;
      lev.leftedge=56;
      lev.rightedge=236;
      lev.halfsizex=12;
      lev.proximity=13;
      lev.startheight=5;
      lev.stibuse=10000;
      lev.stibdec=100;
      lev.dudsin=0;
      lev.dangerinc=8;
      lev.sep=1152;
      lev.lookahead=7;
      lev.bricksprite=maskindex(spriteareas.blocks, 20, 24, 24);
      lev.byteplotter=&sat_plot_24x24_noclip_mask_byte;
      lev.wordplotter=&sat_plot_24x24_noclip_mask_word;
      lev.sparea=spriteareas.blocks;
    break;

#ifndef DEMO
    case 1:
      lev.blocksize.x=24;
      lev.blocksize.y=24;
      lev.indent.x=8;
      lev.indent.y=8;
      lev.gridsize.x=12;
      lev.gridsize.y=10;
      lev.leftedge=8;
      lev.rightedge=284; /* 288 */
      lev.halfsizex=12;
      lev.proximity=13;
      lev.stibuse=12000;
      lev.stibdec=100;
      lev.startheight=5;
      lev.dudsin=0;
      lev.dangerinc=10;
      lev.sep=1152;
      lev.add.x=0;
      lev.add.y=0;
      lev.lookahead=7;
      lev.bricksprite=maskindex(spriteareas.blocks, 20, 24, 24);
      lev.byteplotter=&sat_plot_24x24_noclip_mask_byte;
      lev.wordplotter=&sat_plot_24x24_noclip_mask_word;
      lev.sparea=spriteareas.blocks;
    break;

    case 2:
      lev.blocksize.x=16;
      lev.blocksize.y=16;
      lev.halfsizex=8;
      lev.proximity=9;
      lev.indent.x=44;
      lev.indent.y=8;
      lev.gridsize.x=14;
      lev.gridsize.y=14;
      lev.leftedge=44;
      lev.rightedge=260;
      lev.startheight=7;
      lev.stibuse=19600;
      lev.stibdec=100;
      lev.dudsin=0;
      lev.dangerinc=10;
      lev.sep=512;
      lev.add.x=4;
      lev.add.y=4;
      lev.lookahead=5;
      lev.bricksprite=maskindex(spriteareas.smaller, 20, 16, 16);
      lev.byteplotter=&sat_plot_16x16_noclip_mask_byte;
      lev.wordplotter=&sat_plot_16x16_noclip_mask_word;
      lev.sparea=spriteareas.smaller;
    break;

    case 3:
      lev.blocksize.x=16;
      lev.blocksize.y=16;
      lev.halfsizex=8;
      lev.proximity=9;
      lev.indent.x=12;
      lev.indent.y=8;
      lev.gridsize.x=18;
      lev.gridsize.y=14;
      lev.leftedge=12;
      lev.rightedge=292;
      lev.startheight=7;
      lev.stibuse=31500;
      lev.stibdec=100;
      lev.dudsin=1;
      lev.dangerinc=12;
      lev.sep=512;
      lev.add.x=4;
      lev.add.y=4;
      lev.lookahead=5;
      lev.bricksprite=maskindex(spriteareas.smaller, 20, 16, 16);
      lev.byteplotter=&sat_plot_16x16_noclip_mask_byte;
      lev.wordplotter=&sat_plot_16x16_noclip_mask_word;
      lev.sparea=spriteareas.smaller;
    break;

    case 4:
      lev.blocksize.x=16;
      lev.blocksize.y=16;
      lev.halfsizex=8;
      lev.proximity=9;
      lev.indent.x=12;
      lev.indent.y=8;
      lev.gridsize.x=18;
      lev.gridsize.y=14;
      lev.leftedge=12;
      lev.rightedge=292;
      lev.startheight=8;
      lev.stibuse=36000;
      lev.stibdec=100;
      lev.dudsin=1;
      lev.dangerinc=16;
      lev.sep=512;
      lev.add.x=4;
      lev.add.y=4;
      lev.lookahead=5;
      lev.bricksprite=maskindex(spriteareas.smaller, 20, 16, 16);
      lev.byteplotter=&sat_plot_16x16_noclip_mask_byte;
      lev.wordplotter=&sat_plot_16x16_noclip_mask_word;
      lev.sparea=spriteareas.smaller;
    break;
#endif

  }
}
