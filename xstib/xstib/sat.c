/*#include "kernel.h"
#include "swis.h"*/

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>

/*#include "DeskLib:file.h"*/

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
  struct stat stbuf;
  int size;
  if (stat(name, &stbuf))
  {
    perror ("sat_loadsprites");
    exit(1);
  }
  size = stbuf.st_size;
  if (size==0) exit(1);
  *area=malloc(size);
  if ((int)*area<0x8000) exit(1);
  sat_loadfile(name, *area, size);
}

void sat_makesurfaces(int* area, SDL_Surface** into, int xsize, int ysize,
  int num, int transparent)
{
  int i;
  
  for (i=0; i<num; i++)
  {
    into[i] = SDL_CreateRGBSurfaceFrom(&area[xsize*ysize*i/sizeof(int)],
      xsize, ysize, 8, xsize, 0, 0, 0, 0);
    SDL_SetColors(into[i], vidscreen->format->palette->colors, 0, 256);
    if (transparent)
      SDL_SetColorKey(into[i], SDL_SRCCOLORKEY|SDL_RLEACCEL, 0);
  }
}

void sat_makearbsurfaces(int* area, SDL_Surface** into, int num,
  int transparent)
{
  int i, j;
  
  for (i=0; i<num; i++)
  {
    int* base = &area[area[i]/sizeof(int)];
    int xsize = base[0], ysize = base[1];
    SDL_PixelFormat* fmt;
    into[i] = SDL_CreateRGBSurfaceFrom(&base[2], xsize, ysize, 8, xsize,
      0, 0, 0, 0);
    SDL_SetColors(into[i], vidscreen->format->palette->colors, 0, 256);
    if (transparent)
      SDL_SetColorKey(into[i], SDL_SRCCOLORKEY|SDL_RLEACCEL, 0);
  }
}

#define charaddr(N) ((int*)((int)spriteareas.gamefont+forw[string[(N)]]))
#define UNUSED(X) (X)=(X)

void sat_printf(SDL_Surface **font, SDL_Surface *screen, int x, int y,
  align ment, char *format, ...)
{
  va_list arg;
  char string[256];
  static const char back[58]=
    {' ','!','"','#',39,'(',')','*','+',',','-','.','/',
     '0','1','2','3','4','5','6','7','8','9',':',';','?',
     'a','b','c','d','e','f','g','h','i','j','k','l','m','n',
     'o','p','q','r','s','t','u','v','w','x','y','z','è',
     '‰','Î','Ô','ˆ','¸'
    };
  static SDL_Surface* forw[256];
  static int hasbeendone=0;
  int index, leng, xsize=0;

  UNUSED(font);

  va_start(arg, format);
  leng=vsprintf(string, format, arg);
  va_end(arg);

  if (!hasbeendone)
  {
    for (index=0; index<256; forw[index++]=0);
    for (index=0; index<58; index++)
      forw[back[index]] = sdlsurfaces.gamefont[index];
    hasbeendone=1;
  }

  if (leng==0) return;

  for (index=0; index<leng; xsize+=forw[string[index++]]->w);

  switch (ment)
  {
    case right: x-=xsize; break;
    case centre: x-=xsize/2; break;
  }

  for (index=0; index<leng; index++)
  {
    sat_blit(forw[string[index]], screen, x, y);
//    sat_plot_XxY_noclip_mask_byte(charaddr(index), screen, x, y);
    x+=forw[string[index]]->w;
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
 /* _kernel_swi_regs in;
    in.r[0]=n;
  _kernel_swi(OS_WriteC, &in, &in);*/
  putc(n, stdout);
}

int sat_timeop(void)
{
  return SDL_GetTicks();
}

void sat_loadfile(char* name, int* to, int size)
{
  FILE* f;
  
  f = fopen(name, "r");
  
  if (!f) {
    fprintf(stderr, "Can't open file %s, giving up\n", name);
    exit(1);
  }
  
  fread(to, sizeof(char), size, f);
  fclose(f);
}

void sat_savefile(char* name, int* base, int size)
{
  FILE* f;
  
  f = fopen(name, "w");
  
  if (!f) {
    fprintf(stderr, "Can't open file %s for writing\n", name);
    return;
  }
  
  fwrite(base, sizeof(char), size, f);
  fclose(f);
}

void sat_mouseop(int* x, int* y, int* z)
{
  fprintf(stderr, "Please don't use sat_mouseop\n");
  exit(1);
}

void sat_blit(SDL_Surface* from, SDL_Surface* to, int xpos, int ypos)
{
  SDL_Rect rect;
  rect.x = xpos;
  rect.y = ypos;
  rect.w = from->w;
  rect.h = from->h;
/*  fprintf(stderr, "Blit %p to %p, rect %d:%d:%d:%d\n", from, to, rect.x,
    rect.y, rect.w, rect.h); */
  SDL_BlitSurface(from, NULL, to, &rect);
}

void sat_resetscreen(int* x)
{
}

void sat_plot_32x32_noclip_unmask_word(int *sprite, int *screen, int x, int y)
{
}

void sat_plot_32x32_noclip_mask_word(int *sprite, int *screen, int x, int y)
{
}

void sat_plot_24x24_noclip_mask_word(int *sprite, int *screen, int x, int y)
{
}

void sat_plot_12x24_noclip_unmask_word(int *sprite, int *screen, int x, int y)
{
}

void sat_plot_24x24_noclip_unmask_word(int *sprite, int *screen, int x, int y)
{
}

void sat_plot_24x24_noclip_mask_byte(int *sprite, int *screen, int x, int y)
{
}

void sat_plot_24x24_clip_mask_byte(int *sprite, int *screen, int x, int y)
{
}

void sat_plot_16x16_noclip_mask_byte(int *sprite, int *screen, int x, int y)
{
}

void sat_plot_16x16_noclip_mask_word(int *sprite, int *screen, int x, int y)
{
}

void sat_plot_64x32_noclip_mask_word(int *sprite, int *screen, int x, int y)
{
}

void sat_plot_backdrop(SDL_Surface *sprite, SDL_Surface *screen)
{
  int x, y;
  
  for (y=0; y<256; y+=32)
  {
    for (x=0; x<320; x+=32)
    {
      sat_blit(sprite, screen, x, y);
    }
  }
}

void sat_plot_XxY_noclip_mask_byte(int *sprite, int *screen, int x, int y)
{
}

void sat_plot_barmiddle(SDL_Surface* sprite, SDL_Surface* screen, int xpos,
  int ypos, int length)
{
  int i, j;

  if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);

  for (j=0; j<sprite->h; j++)
  {
    int col = ((char*)sprite->pixels)[j*4];
    char* rowstart = &((char*)screen->pixels)[(ypos+j)*320+xpos];
    
    for (i=0; i<length; i++)
    {
      rowstart[i] = col;
    }
  }

  if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
}

void sat_plot_squiggled(SDL_Surface *sprite, SDL_Surface *screen,
  squiggleinfo *info)
{
  int x, y;
  char* srcpix = sprite->pixels;
  
  if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);
  
  for (y=0; y<24; y++)
  {
    char* rowstart = &((char*)screen->pixels)[(info->y+y)*320+info->x];
    int offset = info->tab[y].offset;
    int scale = info->tab[y].scale;
    int i = 0;
    
    rowstart += offset;
    
    for (x=0; x<24*65536; x+=scale, i++)
    {
      int col = srcpix[y*24+(x>>16)];
      if (col) rowstart[i] = col;
    }
  }
  
  if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
}

// um, attempt to minimise memory traffic at expense of complexity?
// dunno if this is any use.
void sat_stretch(SDL_Surface* dest, SDL_Surface* src, int factor)
{
  int y;
  unsigned char* srcptr = src->pixels;
  unsigned char* destptr = dest->pixels;
  int reloady = factor-1, yptr = 0;

  SDL_LockSurface(dest);

  for (y=0; y<256*factor; y++)
  {
    register int x;
    register unsigned char* rowptr = &srcptr[yptr*320];
    register int byte = *rowptr, reloadx = factor-1, xptr = 0;

    for (x=0; x<320*factor; x++)
    {
      if (--reloadx<0)
      {
        byte = rowptr[xptr++];
        reloadx = factor-1;
      }
      *destptr++ = byte;
    }

    if (--reloady<0)
    {
      yptr++;
      reloady = factor-1;
    }
  }

  SDL_UnlockSurface(dest);
}

void sat_refresh(SDL_Surface* dest, SDL_Surface* src)
{
  if (windowzoom>1)
  {
    sat_stretch(dest, src, windowzoom);
  }
  else
  {
    SDL_BlitSurface(src, 0, dest, 0);
  }
  SDL_UpdateRect(dest, 0, 0, 0, 0);
}

int sat_swapbanks(int q)
{
}

int* sat_getscreenbase(void)
{
  return 0;
}
