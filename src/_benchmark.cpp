#include <_benchmark.h>

_benchmark::_benchmark() 
{
    // ctor
}

_benchmark::~_benchmark() 
{
    // dtor
}

void _benchmark::startBenchmark() 
{
    startTime = chrono::high_resolution_clock::now();
}

void _benchmark::clickBenchmark()
{
    auto currentTime = chrono::high_resolution_clock::now();
    double dt = chrono::duration_cast<chrono::milliseconds>(currentTime-startTime).count();
    totalTime += dt;
    clicks++;  
}

void _benchmark::resetBenchmark()
{
    startTime = chrono::high_resolution_clock::now();
    totalTime = 0.0;
    clicks = 0;
    averageTime = 0.0;
}

double _benchmark::getAverageResult() 
{
    if (clicks == 0) {
        cerr << "Divide by zero error -- benchmark iterations is 0. Make sure to use clickBenchmark at least once prior to getting the result";
        return 0.0;
    }
    averageTime = totalTime / clicks;
    return averageTime;
}