#include <_timerPlusPlus.h>

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
    startTime = chrono::steady_clock::now();
}

double _timerPlusPlus::getSeconds() const
{
    auto currentTime = chrono::steady_clock::now();
    chrono::duration<double> duration = currentTime - startTime;
    return duration.count();
}

double _timerPlusPlus::getMilliseconds() const
{
    return getSeconds() * 1000.0;
}

