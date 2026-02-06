#include "_timerPlusPlus.h"

_timerPlusPlus::_timerPlusPlus()
{
    //ctor
    reset();
}

_timerPlusPlus::~_timerPlusPlus()
{
    //dtor
}

void _timerPlusPlus::reset()
{
    startTime = chrono::high_resolution_clock::now();
}

double _timerPlusPlus::getTicks()
{
    auto currentTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(currentTime - startTime);
    return duration.count();
}
