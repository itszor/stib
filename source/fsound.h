/* header file for flexisound */
/* (c) Julian Brown, Saturday   */

#ifndef FSOUND_H
#define FSOUND_H 1

extern int flexisound_add(char *name);
extern void flexisound_remove(int number);
extern void flexisound_play(int number, int channel, int volume, int pitch);

#endif
