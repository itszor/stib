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
#include "global.h"

#include "DeskLib:sprite.h"
#include "DeskLib:swi.h"
#include "DeskLib:screen.h"
#include "DeskLib:window.h"
#include "DeskLib:wimpswis.h"
#include "DeskLib:kbd.h"

void playagame(void)
{
  int regulator=1;
  int xm=0, ym=0, bm=0;
  int *screen;

  while (game==alive || game==soontodie || game==soontowin)
  {
    static int ntime, otime;

    sat_mouseop(&xm,&ym,&bm);
    stibshooter.angle=(xm-640)/2;
    if (stibshooter.angle<0) stibshooter.angle+=360*4;

    sat_data.screenbuffer=sat_swapbanks(sat_data.screenbuffer);
    if (sat_data.screenbuffer==2) screen=buf1(sat_data.screenbase);
                             else screen=buf2(sat_data.screenbase);

    /* Check for fire initialised */
    if (bm>0 && !stibshooter.inuse && game!=soontodie && game!=soontowin &&
        !playfield.exploding && !playfield.burning)
    {
      initfire(stibshooter.angle);
    }

    /* Draw playing screen */
    drawgamescreen(screen, stibshooter.angle, regulator);

    /* Update any states, etc. which need changing */
    if (stibshooter.inuse) dofire(screen, regulator);
    if (playfield.detaching) dodetach(screen);
    if (playfield.exploding) doexplode();
    if (playfield.burning) doburn(regulator);

    otime=ntime;
    ntime=sat_timeop();
    regulator=(ntime-otime)/2; /* 50hz internal clock */
    if (regulator<1) regulator=1;
    else if (regulator>4) regulator=4; /* speed compensate within reason */

    updatestate(regulator);

    /* escape pressed? */
    if (Kbd_KeyDown(-113) || Kbd_KeyDown(-30)) playfield.burning=true;
  }
}


void drawgamescreen(int *screen, int angle, int regulator)
{
  int test1, test2;

  if (playfield.shuffling)
  {
    drawplayarea(screen, 1, shuffletimeindex);

    test1=(int)(shuffletimeindex/lev.blocksize.y);
    shuffletimeindex+=regulator*2;
    test2=(int)(shuffletimeindex/lev.blocksize.y);

    if (test1!=test2) flexisound_play(sounds.whoosh, 3, 0x17f, 0x26c0);

    if (shuffletimeindex>lev.gridsize.y*(lev.blocksize.y+2)/2)
    {
      playfield.shuffling=false;
      down.rows++;
      down.pixels.current+=lev.blocksize.y;
      burntest();
    }
  }
  else drawplayarea(screen, 0, 0);

  drawdevice(screen, (int)angle);
}


void drawplayarea(int *screen, int fallingflag, int timeindex)
{
  register int xloop, yloop;
  int *sprite;
  static int shift[playgrid_xsize][playgrid_ysize];
  int time=(sat_timeop()/16)%4;
  int *lake=maskindex(spriteareas.lakeoffire, time, 32, 32);
  int *top=nomaskarb(spriteareas.danger, 5);
  int *side=nomaskarb(spriteareas.danger, 4);

  /* draw patterned background */
  sat_plot_backdrop(backdroptile, screen);

  if (!fallingflag)
  {
    for (yloop=0; yloop<lev.gridsize.y; yloop++)
    {
      for (xloop=0; xloop<lev.gridsize.x; xloop++)
      {
        if (array[xloop][yloop].info.full)
        {
          if (!array[xloop][yloop].info.exploding)
            sprite=(int*)((int)lev.sparea+(array[xloop][yloop].info.sprite
              +8*array[xloop][yloop].info.diff)*lev.sep);
          else
            sprite=(int*)((int)lev.sparea+(16+(array[xloop][yloop].info.explodesp/2))
              *lev.sep);

          (*lev.wordplotter)(sprite, screen,
             cposx(xloop,yloop), cposy(yloop,down.pixels.current));
        }
      }
    }
  }
  else
  {
    /* Set up shuffle table */
    for (yloop=0; yloop<lev.gridsize.y; yloop++)
    {
      for (xloop=0; xloop<lev.gridsize.x; xloop++)
      {
        float angl=(float)(10*(xloop-(lev.gridsize.x>>1)))/(float)lev.gridsize.x;
        shift[xloop][yloop]=timeindex-(((lev.gridsize.y-yloop)*lev.blocksize.y)>>1)
          -(int)(angl*angl)+lev.blocksize.y;
        if (shift[xloop][yloop]<0) shift[xloop][yloop]=0;
        if (shift[xloop][yloop]>lev.blocksize.y) shift[xloop][yloop]=lev.blocksize.y;
      }
    }

    /* Draw shuffled */
    for (xloop=0; xloop<lev.gridsize.x; xloop++)
    {
      for (yloop=0; yloop<lev.gridsize.y; yloop++)
      {
        if (array[xloop][yloop].info.full)
        {
          if (!array[xloop][yloop].info.exploding)
            sprite=(int*)((int)lev.sparea+(array[xloop][yloop].info.sprite
              +8*array[xloop][yloop].info.diff)*lev.sep);
          else
            sprite=(int*)((int)lev.sparea+(16+(array[xloop][yloop].info.explodesp)/2)
              *lev.sep);

          (*lev.wordplotter)(sprite, screen,
             cposx(xloop,yloop),
             cposy(yloop,down.pixels.current+shift[xloop][yloop]));
        }
      }
    }
  }

  /* level 3 hack */
  if (lev.blocksize.x==24)
  {
    for (yloop=0; yloop<down.rows; yloop++)
    {
      for (xloop=0; xloop<lev.gridsize.x; xloop++)
      {
        sat_plot_24x24_noclip_unmask_word(lev.bricksprite, screen,
          xloop*24+lev.leftedge, yloop*lev.blocksize.y+lev.indent.y);
      }
      sat_plot_12x24_noclip_unmask_word(spriteareas.megahack, screen,
        xloop*24+lev.leftedge, yloop*lev.blocksize.y+lev.indent.y);
    }
  }
  else
  {
    for (yloop=0; yloop<down.rows; yloop++)
    {
      for (xloop=0; xloop<=lev.gridsize.x; xloop++)
      {
        (*lev.wordplotter)(lev.bricksprite, screen,
          xloop*lev.blocksize.x+lev.leftedge-4,
          yloop*lev.blocksize.y+lev.indent.y);
      }
    }
  }

  for (xloop=lev.leftedge; xloop<lev.rightedge+lev.blocksize.x-8; xloop+=16)
  {
    sat_plot_XxY_noclip_mask_byte(top, screen, xloop, 4);
  }

  if (((lev.rightedge+lev.blocksize.x)-xloop)>=8)
  {
    sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.danger, 8), screen, xloop, 4);
  }

  for (yloop=8; yloop<224; yloop+=16)
  {
    sat_plot_XxY_noclip_mask_byte(side, screen, lev.leftedge-4, yloop);
    sat_plot_XxY_noclip_mask_byte(side, screen, lev.rightedge+lev.blocksize.x, yloop);
  }

  sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.danger, 6), screen, lev.leftedge-4, 4);
  sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.danger, 7), screen,
    lev.rightedge+lev.blocksize.x, 4);

  for (xloop=0; xloop<320; xloop+=32)
  {
    sat_plot_32x32_noclip_mask_word(lake, screen, xloop, 224);
  }

  sat_printf(spriteareas.gamefont, screen, 188, 243, left, "score: %d", score.current);
}


void drawdevice(int *screen, int angle)
{
  int loop;
  fpoint part, diff;
  bminfo mid;
  int length=danger.current;
  int *base;

  sat_plot_64x32_noclip_mask_word(spriteareas.shootbase, screen, 128, 224);
  part.x=160.+(float)(sintab[angle]*24/256);
  part.y=244.-(float)(costab[angle]*24/256);
  diff.x=(float)sintab[angle]*12/256;
  diff.y=-(float)costab[angle]*12/256;
  for (loop=0; loop<4; loop++)
  {
    if (loop!=3) base=nomaskindex(spriteareas.shooter, 1, 16, 16);
      else base=spriteareas.shooter;
    sat_plot_16x16_noclip_mask_byte(base, screen, (int)part.x-8, (int)part.y-8);
    part.x+=diff.x;
    part.y+=diff.y;
  }

  sat_plot_24x24_noclip_mask_word(maskindex(spriteareas.blocks, nxshp.cell.future,
    24, 24), screen, 292, 232);

  switch (nxshp.state)
  {
    case stable:
      sat_plot_24x24_noclip_mask_word(maskindex(spriteareas.blocks, nxshp.cell.current,
        24, 24), screen, 148, 232);
    break;

    case moving:
      drawnext(screen, nxshp.pos);
  }

  sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.danger, 0), screen, 4, 244);
  if (length>2)
  {
    sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.danger, 1), screen, 5, 245);
    mid.sprite=nomaskarb(spriteareas.danger, 2);
    mid.screen=screen;
    mid.xpos=7;
    mid.ypos=245;
    mid.length=length;
    sat_plot_barmiddle(&mid);
    sat_plot_XxY_noclip_mask_byte(nomaskarb(spriteareas.danger, 3), screen, length+7, 245);
  }
}


/* range of x 148-292 */
/* mid point 220 */
void drawnext(int *screen, int pos)
{
  squiggleinfo sq;
  register int loop;
  static struct {
    int offset;
    int scale;
  } stat[24];
  static int done=false;
  float mult;

  if (!done)
  {
    done=true;
    for (loop=0; loop<24; loop++)
    {
      stat[loop].scale=(rand()%40000);
      stat[loop].offset=-rand()%12;
    }
  }

  sq.x=pos;
  sq.y=232;
  mult=(pos-220.0)/72.0;
  if (mult<0) mult=-mult;
  mult=1-mult;
  for (loop=0; loop<24; loop++)
  {
    sq.tab[loop].offset=(int)(stat[loop].offset*mult);
    sq.tab[loop].scale=(int)(65536-stat[loop].scale*mult);
  }

  sat_plot_squiggled(maskindex(spriteareas.blocks, nxshp.cell.current, 24, 24),
    screen, &sq);
}


void alterasprite(void)
{
  int x=rand()%lev.gridsize.x, y=rand()%lev.gridsize.y;
  array[x][y].info.diff=1-array[x][y].info.diff;
}


int locktogrid(int x, int y, int *xgrid, int *ygrid)
{
  register int xloop, yloop;
  int xpos, ypos, onehere=false;

  for (yloop=lev.gridsize.y-1; yloop>=0 && !onehere; yloop--)
  {
    for (xloop=0; xloop<lev.gridsize.x && !onehere; xloop++)
    {
      xpos=cposx(xloop,yloop);
      ypos=cposy(yloop,down.pixels.current);
      if ((abs(xpos-x)<lev.proximity && abs(ypos-y)<lev.proximity) ||
          y<down.pixels.current)
      {
        *xgrid=xloop;
        *ygrid=yloop;
        if (array[xloop][yloop].info.full) onehere=true;
      }
    }
  }
  return onehere;
}


int countnum(int startx, int starty)
{
  register int xloop, yloop, loop;
  int done, sprite, count=1;
  ipoint test;

  /* unset all 'counted' + test for burning */

  for (xloop=0; xloop<lev.gridsize.x; xloop++)
    for (yloop=0; yloop<lev.gridsize.y; yloop++)
      array[xloop][yloop].info.counted=0;

  array[startx][starty].info.counted=1;
  sprite=array[startx][starty].info.sprite;

  do {
    done=1;
    for (xloop=0; xloop<lev.gridsize.x; xloop++)
    {
      for (yloop=0; yloop<lev.gridsize.y; yloop++)
      {
        for (loop=0; loop<6; loop++)
        {
          if ((yloop&1)==0)
          {
            test.x=xloop+alt1[loop].x;
            test.y=yloop+alt1[loop].y;
          }
          else
          {
            test.x=xloop+alt2[loop].x, test.y=yloop+alt2[loop].y;
          }

          if ((test.x>=0) && (test.x<lev.gridsize.x)&&(test.y>=0)
              && (test.y<lev.gridsize.y))
          {
            if (array[test.x][test.y].info.counted &&
               !array[xloop][yloop].info.counted &&
                array[xloop][yloop].info.full &&
                sprite==array[xloop][yloop].info.sprite)
            {
              count++;
              array[xloop][yloop].info.counted=1;
              done=0;
            }
          }
        }
      }
    }
  } while (!done);

  return count;
}


/* Set exploding flags for all counted stibs, and increase score accordingly */
void testarea(void)
{
  register int xloop, yloop;

  playfield.exploding=true;

  for (xloop=0; xloop<lev.gridsize.x; xloop++)
  {
    for (yloop=0; yloop<lev.gridsize.y; yloop++)
    {
      if (array[xloop][yloop].info.counted==1)
      {
        array[xloop][yloop].info.exploding=1;
        array[xloop][yloop].info.explodesp=0;
        score.future+=30;
      }
    }
  }
}


void countunattached(void)
{
  register int xloop, yloop, loop;
  int done;
  ipoint test;

  for (xloop=0; xloop<lev.gridsize.x; xloop++)
    for (yloop=0; yloop<lev.gridsize.y; yloop++)
      array[xloop][yloop].info.counted=false;

  for (xloop=0; xloop<lev.gridsize.x; xloop++)
    if (array[xloop][0].info.full && !array[xloop][0].info.exploding)
      array[xloop][0].info.counted=true;

  do {
    done=1;
    for (xloop=0; xloop<lev.gridsize.x; xloop++)
    {
      for (yloop=0; yloop<lev.gridsize.y; yloop++)
      {
        for (loop=0; loop<6 && !array[xloop][yloop].info.counted; loop++)
        {
          if ((yloop&1)==0)
          {
            test.x=xloop+alt1[loop].x;
            test.y=yloop+alt1[loop].y;
          }
          else
          {
            test.x=xloop+alt2[loop].x;
            test.y=yloop+alt2[loop].y;
          }

          if ((test.x>=0) && (test.x<lev.gridsize.x) && (test.y>=0) &&
              (test.y<lev.gridsize.y))
          {
            if (array[test.x][test.y].info.counted &&
               !array[xloop][yloop].info.counted &&
                array[xloop][yloop].info.full &&
               !array[xloop][yloop].info.exploding)
            {
              array[xloop][yloop].info.counted=1;
              done=0;
            }
          }
        }
      }
    }
  } while (!done);
}


/* Destroy any Stibs unattached, and fill them into the dropping array */
void dropunattached(void)
{
  register int xloop, yloop;
  int dangersub=false;

  drop.number=0;

  for (xloop=0; xloop<lev.gridsize.x; xloop++)
  {
    for (yloop=0; yloop<lev.gridsize.y; yloop++)
    {
      if (!array[xloop][yloop].info.counted &&
          !array[xloop][yloop].info.exploding &&
           array[xloop][yloop].info.full)
      {
        drop.drop[drop.number].pos.x=(float)(cposx(xloop,yloop));
        drop.drop[drop.number].pos.y=(float)(cposy(yloop,down.pixels.current));
        drop.drop[drop.number].dir.x=(float)(rand()%20-10)/2;
        drop.drop[drop.number].dir.y=0;
        drop.drop[drop.number].sprite=array[xloop][yloop].info.sprite;
        drop.number++;
        array[xloop][yloop].info.full=0;
        playfield.detaching=true; /* ~+ implicit gamestate=exploding */
        danger.future-=lev.dangerinc;
        dangersub=true;
        score.future+=60;
      }
    }
  }
  if (!dangersub)
  {
    danger.future+=lev.dangerinc;
    flexisound_play(sounds.transport, 2, 0x17f, 0x26c0);
  }
  else
  {
    flexisound_play(sounds.fanfare, 2, 0x17f, 0x1700);
    bigfanfare.timeout=sat_timeop(); /* Set time of start */
  }
}


void evaluatetypes(void)
{
  register int xloop, yloop, loop;
  int sprite, inlist;

  numtypes=0;

  for (xloop=0; xloop<lev.gridsize.x; xloop++)
  {
    for (yloop=0; yloop<lev.gridsize.y; yloop++)
    {
      if (array[xloop][yloop].info.full &&
         !array[xloop][yloop].info.exploding &&
          array[xloop][yloop].info.sprite!=7)
      {
        sprite=array[xloop][yloop].info.sprite;
        inlist=false;
        for (loop=0; loop<numtypes; loop++)
          if (sprite==types[loop])
            inlist=true;
        if (!inlist) types[numtypes++]=sprite;
      }
    }
  }
  if (numtypes==0) game=soontowin, pendgstimeout=sat_timeop();
}


void initfire(int angle)
{
  ipoint current, nextpos;
  stibshooter.inuse=true;
  tcell.movepos.x=(float)148+lev.add.x;
  tcell.movepos.y=(float)232+lev.add.y;
  tcell.movedir.x=(float)(sintab[angle]*shootspeed)/256.0;
  tcell.movedir.y=(float)(-costab[angle]*shootspeed)/256.0;

  if (lev.stibuse>0)
    lev.stibuse-=lev.stibdec;

  flexisound_play(sounds.fire, 2, 0x17f, 0x26c0);
  nxshp.state=pendingnew;

  /* check 4 instant death */
  nextpos.x=(int)(tcell.movepos.x+tcell.movedir.x*lev.lookahead);
  nextpos.y=(int)(tcell.movepos.y+tcell.movedir.y*lev.lookahead);
  if (nextpos.x<lev.leftedge) nextpos.x+=lev.leftedge-nextpos.x;
  if (nextpos.x>lev.rightedge) nextpos.x-=nextpos.x-lev.rightedge;
  if (locktogrid((int)nextpos.x, (int)nextpos.y, &current.x, &current.y))
    if (tcell.cell.current!=array[current.x][current.y].info.sprite)
      playfield.burning=true;
}


void dodetach(int *screen)
{
  register int del, loop;
  for (loop=0; loop<drop.number;)
  {
    (*lev.byteplotter)(maskindex(lev.sparea, drop.drop[loop].sprite,
      lev.blocksize.x, lev.blocksize.y), screen,
      (int)drop.drop[loop].pos.x, (int)drop.drop[loop].pos.y);
    drop.drop[loop].pos.x+=drop.drop[loop].dir.x;
    drop.drop[loop].pos.y+=drop.drop[loop].dir.y++;

    if (drop.drop[loop].pos.x<lev.leftedge ||
        drop.drop[loop].pos.x>=lev.rightedge ||
        drop.drop[loop].pos.y>=232)
    {
      for (del=loop; del<drop.number-1; del++) drop.drop[del]=drop.drop[del+1];
      drop.number--;
    }
    else loop++; /* so none are skipped inadvertently when removing */
  }
  /* remove 'detaching' from 'tasklist' */
  if (drop.number==0) playfield.detaching=false;
}


void doexplode(void)
{
  int someblownup=0;
  register int xloop, yloop;

  for (xloop=0; xloop<lev.gridsize.x; xloop++)
  {
    for (yloop=0; yloop<lev.gridsize.y; yloop++)
    {
      if (array[xloop][yloop].info.full && array[xloop][yloop].info.exploding)
      {
        array[xloop][yloop].info.explodesp++;
        someblownup=1;
        if (array[xloop][yloop].info.explodesp==0)
        {
          array[xloop][yloop].info.exploding=0;
          array[xloop][yloop].info.full=0;
          playfield.exploding=false;
        }
      }
    }
  }
  if (!someblownup && playfield.exploding) playfield.exploding=false;
}


void dofire(int *screen, int regulator)
{
  static ipoint current={0,0}, old={0,0}, nextpos={0,0};
  register int xloop, yloop;
  int min;
  int *previous=(int*)((int)lev.sparea+tcell.cell.current*lev.sep);

  do {
    nextpos.x=(int)(tcell.movepos.x+tcell.movedir.x*lev.lookahead);
    nextpos.y=(int)(tcell.movepos.y+tcell.movedir.y*lev.lookahead);

    if (nextpos.x<lev.leftedge) nextpos.x+=lev.leftedge-nextpos.x;
    if (nextpos.x>lev.rightedge) nextpos.x-=nextpos.x-lev.rightedge;

    tcell.movepos.x+=tcell.movedir.x;
    tcell.movepos.y+=tcell.movedir.y;

    if (tcell.movepos.x<lev.leftedge || tcell.movepos.x>lev.rightedge)
    {
      tcell.movedir.x=-tcell.movedir.x;
      if (tcell.movepos.x<lev.leftedge)
        tcell.movepos.x+=lev.leftedge-tcell.movepos.x;
      if (tcell.movepos.x>lev.rightedge)
        tcell.movepos.x-=tcell.movepos.x-lev.rightedge;
      flexisound_play(sounds.boing, 2, 0x17f, 0x1700);
    }

    old=current;
    if (locktogrid((int)nextpos.x, (int)nextpos.y, &current.x, &current.y))
    {
      stibshooter.inuse=false;

      array[old.x][old.y].info.full=true;
      array[old.x][old.y].info.sprite=tcell.cell.current;

      if (countnum(old.x, old.y)<3)
      {
        danger.future+=lev.dangerinc;
        flexisound_play(sounds.lockin, 2, 0x17f, 0x26c0);
      }
      else
      {
        testarea();
        countunattached();
        dropunattached();
        evaluatetypes();
      }

      score.future+=10;

      tcell.cell.current=nxshp.cell.current;
      tcell.cell.future=nxshp.cell.future;

      if (game==won) break;

      if (danger.future<0)
      {
        danger.future=0;
        bigfanfare.delaysfx=true;
      } else bigfanfare.delaysfx=false;

      if (danger.future>124)
      {
        int found=false;

        danger.future=0;

        for (yloop=lev.gridsize.y-1; yloop>=0 && !found; yloop--)
          for (xloop=0; xloop<lev.gridsize.x && !found; xloop++)
            if (array[xloop][yloop].info.full) min=yloop, found=true;

        if (found)
          shuffletimeindex=((lev.gridsize.y-min)*lev.blocksize.y/2)
            -lev.blocksize.y;
        else
          shuffletimeindex=0;

        playfield.shuffling=true;
      }

      burntest();

      regulator=0;
    } /* end of hit test */
  } while (--regulator>=0);

  (*lev.byteplotter)(previous, screen, (int)tcell.movepos.x, (int)tcell.movepos.y);
}


void burntest(void)
{
  register int xloop, yloop;

  for (xloop=0; xloop<lev.gridsize.x; xloop++)
    for (yloop=0; yloop<lev.gridsize.y; yloop++)
      if (array[xloop][yloop].info.full &&
          !array[xloop][yloop].info.exploding &&
          (yloop+down.rows)>lev.gridsize.y-2)
        playfield.burning=true;
}


void doburn(int regulator)
{
  ipoint pos;
  int xloop, yloop, max=0;

  if (burningstate==-1)
  {
    flexisound_play(sounds.wedied, 2, 0x17f, 0x1700);
    for (yloop=0; yloop<lev.gridsize.y; yloop++)
      for (xloop=0; xloop<lev.gridsize.x; xloop++)
        if (array[xloop][yloop].info.full && max<yloop) max=yloop;
    burningstate=lev.gridsize.x*(max+1)-1;
  }

  do {
    pos.y=(int)burningstate/lev.gridsize.x;
    pos.x=(int)burningstate%lev.gridsize.x;
    if (pos.y%2) pos.x=lev.gridsize.x-pos.x-1;
    if (array[pos.x][pos.y].info.full) array[pos.x][pos.y].info.sprite=7;

    burningstate--;

    if (burningstate==-1)
    {
      game=soontodie;
      playfield.burning=false;
      pendgstimeout=sat_timeop();
    }
  } while (burningstate>-1 && --regulator>=0);
}


void updatestate(int regulator)
{
  alterasprite();
  if (danger.current<danger.future) danger.current+=2;
  if (danger.current>danger.future) danger.current-=2;
  if (abs(danger.current-danger.future)<2) danger.current=danger.future;
  if (score.current<score.future) score.current+=5;

  switch (nxshp.state)
  {
    case pendingnew:
    nxshp.cell.current=tcell.cell.future;
    nxshp.cell.future=types[rand()%numtypes];
    nxshp.pos=292;
    nxshp.state=moving;
    break;

    case moving:
    nxshp.pos-=regulator*4;
    if (nxshp.pos<=148) nxshp.state=stable;
  }
  /* Play extended fanfare if danger zeroed */
  if (bigfanfare.delaysfx && (sat_timeop()-bigfanfare.timeout)>75)
  {
    flexisound_play(sounds.fanfare2, 3, 0x17f, 0x1700);
    bigfanfare.delaysfx=false;
  }
  /* now a NEW class 'B' hack */
  if (pendgstimeout!=-1 && (sat_timeop()-pendgstimeout)>100)
  {
    switch (game)
    {
      case soontodie: game=died; break;
      case soontowin: game=won; break;
    }
  }
}
