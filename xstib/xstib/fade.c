#include "fade.h"

void fade_zoom(SDL_Surface* dest, SDL_Surface* source)
{
  sat_refresh(dest, source);
}

void fade_xfade(SDL_Surface* dest, SDL_Surface* source, int* coltab)
{
  sat_refresh(dest, source);
}
