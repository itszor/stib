#include <SDL/SDL_mixer.h>

#include "fsound.h"

int flexisound_add(char* name)
{
  return 0;
}

void flexisound_remove(int x)
{
  return;
}

void flexisound_play(Mix_Chunk* chunk, int channel, int vol, int pitch)
{
  Mix_PlayChannel(channel, chunk, 0);
}
