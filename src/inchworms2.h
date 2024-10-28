#ifndef INCHWORMS2_H
#define INCHWORMS2_H

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <ncurses.h>
#include <time.h>

void *updateScr();
int tooCloseY( int i, int yMax);
int tooCloseX( int i, int xMax );
bool closeCorner( int headYX[], int yMax, int xMax);
void redisplay( bool state, int coords[][2], int thread);
void *wormFunc(void *arg);
int main();



#endif