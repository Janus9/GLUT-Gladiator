#ifndef _TIMER_PLUS_PLUS_H
#define _TIMER_PLUS_PLUS_H

#include <_common.h>

class _timerPlusPlus
{
    public:
        _timerPlusPlus();
        virtual ~_timerPlusPlus();
        // Reset the timer to zero
        void reset();

        // Returns the amount of time elapsed in seconds
        double getSeconds() const;
        // Returns the amount of time elapsed in milliseconds
        double getMilliseconds() const;

    protected:
    private:
        chrono::steady_clock::time_point startTime;
};

#endif // _TIMER_PLUS_PLUS_H
