#include "timer.h"

#include <thread>

#define SLEEP_TIME 3

static std::thread *threadTimer;
static bool isTiming;

void timerInThread(S_SEARCHINFO *info, int t0, int movetime) {
    while (isTiming) {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
        int t1 = TIME_MS;
        if (t1 - t0 >= movetime) {
            info->stopped = true;
            break;
        }
    }
}

void stopTimer() {
    if (isTiming) {
        isTiming = false;
        threadTimer->join();
        delete(threadTimer);
    }
}

void startTimer(S_SEARCHINFO *info, int moveTime) {
    int t0 = TIME_MS;

    isTiming = true;
    threadTimer = new std::thread(timerInThread, info, t0, moveTime);
}

void startTimer(S_SEARCHINFO *info, int timeLeft, int moveIncrement, int movesLeft) {
    int timeTotal = timeLeft + moveIncrement * movesLeft;
    int movetime = timeTotal / movesLeft;
    printf("info starting timer with %d ms\n", movetime);
    startTimer(info, movetime);
}