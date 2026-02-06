#ifndef _TIMER_PLUS_PLUS_H
#define _TIMER_PLUS_PLUS_H

#include<_common.h>

class _timerPlusPlus
{
    public:
        _timerPlusPlus();
        virtual ~_timerPlusPlus();
        // Reset the timer to zero
        void reset();
        // Returns the number of ticks since last reset (in milliseconds)
        double getTicks();

    protected:

    private:
        chrono::high_resolution_clock::time_point startTime;
};

#endif // _TIMER_PLUS_PLUS_H
