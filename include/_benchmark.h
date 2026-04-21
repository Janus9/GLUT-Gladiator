#ifndef _BENCHMARK_H
#define _BENCHMARK_H

#include <_common.h>

class _benchmark 
{
    public:
        _benchmark();
        virtual ~_benchmark();

        // Starts the benchmark timing. There is no "stop" benchmark since we never really want to stop. There is an option to "reset" however.
        void startBenchmark();
        // "Clicks" the timemark to mark a new iteration. Doesn't stop the timer.
        void clickBenchmark();
        void resetBenchmark();

        // Returns the average time elapsed (in ms)
        double getAverageResult();
    protected:
    private:
        chrono::high_resolution_clock::time_point startTime;

        double totalTime = 0.0;    // Total time (in ms) ellapsed
        int64_t clicks = 0;         // Number of checkpoint clicks. Used for finding the average.
        double averageTime = 0.0;  // Average time the benchmark has found
};

#endif // _BENCHMARK_H