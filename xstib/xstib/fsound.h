/* header file for flexisound */
/* © Julian Brown, Saturday   */

#ifndef FSOUND_H
#define FSOUND_H 1

#include <SDL/SDL_mixer.h>

extern int flexisound_add(char *name);
extern void flexisound_remove(int number);
extern void flexisound_play(Mix_Chunk* number, int channel, int volume,
  int pitch);

#endif
