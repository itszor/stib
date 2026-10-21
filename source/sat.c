#include "kernel.h"
#include "swis.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "DeskLib:file.h"

#include "sat.h"
#include "play.h"
#include "global.h"

satinfo sat_data;

void sat_initialise(void)
{
  sat_data.screenbase=sat_getscreenbase();
  sat_data.screenbuffer=1;
}

void sat_loadsprites(int** area, char *name)
{
  int size;
  size=File_Size(name);
  if (size==0) exit(1);
  *area=malloc(size);
  if ((int)*area<0x8000) exit(1);
  sat_loadfile(name, *area);
}

#define charaddr(N) ((int*)((int)spriteareas.gamefont+forw[string[(N)]]))

void sat_printf(int *font, int *screen, int x, int y, align ment, char *format, ...)
{
  va_list arg;
  char string[256];
  static const char back[58]=
    {' ','!','"','#',39,'(',')','*','+',',','-','.','/',
     '0','1','2','3','4','5','6','7','8','9',':',';','?',
     'a','b','c','d','e','f','g','h','i','j','k','l','m','n',
     'o','p','q','r','s','t','u','v','w','x','y','z','\x8f',
     '\xe4','\xeb','\xef','\xf6','\xfc'
    };
  static int forw[256];
  static int hasbeendone=0;
  int index, leng, xsize=0;

  va_start(arg, format);
  leng=vsprintf(string, format, arg);
  va_end(arg);

  if (!hasbeendone)
  {
    for (index=0; index<256; forw[index++]=0);
    for (index=0; index<58; index++) forw[back[index]]=spriteareas.gamefont[index];
    hasbeendone=1;
  }

  if (leng==0) return;

  for (index=0; index<leng; xsize+=*charaddr(index++));

  switch (ment)
  {
    case right: x-=xsize; break;
    case centre: x-=xsize/2; break;
  }

  for (index=0; index<leng; index++)
  {
    sat_plot_XxY_noclip_mask_byte(charaddr(index), screen, x, y);
    x+=*charaddr(index);
  }
}

void vdustr(int a, ...)
{
  va_list ap;
  int b=0;

  writec(a);

  va_start(ap, a);
  while ((b=va_arg(ap, int))!=-1) writec(b);
  va_end(ap);
}

void writec(int n)
{
  _kernel_swi_regs in;
  in.r[0]=n;
  _kernel_swi(OS_WriteC, &in, &in);
}
