#ifndef TIMER_H
#define TIMER_H

#include "defs.h"
#include <chrono>

#define TIME_MS std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()

extern void startTimer(S_SEARCHINFO* searchInfo, int timeLeft, int moveIncrement, int movesLeft);
extern void startTimer(S_SEARCHINFO* searchInfo, int moveTime);
extern void stopTimer();

#endif // !TIMER_H