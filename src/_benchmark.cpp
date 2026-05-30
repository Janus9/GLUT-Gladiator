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
    startTime = std::chrono::steady_clock::now();
}

void _benchmark::clickBenchmark()
{
    auto currentTime = std::chrono::steady_clock::now();
    double dt = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime-startTime).count();
    totalTime += dt;
    clicks++;  
}

void _benchmark::resetBenchmark()
{
    startTime = std::chrono::steady_clock::now();
    totalTime = 0.0;
    clicks = 0;
    averageTime = 0.0;
}

double _benchmark::getAverageResult() 
{
    if (clicks == 0) {
        std::cerr << "Divide by zero error -- benchmark iterations is 0. Make sure to use clickBenchmark at least once prior to getting the result";
        return 0.0;
    }
    averageTime = totalTime / clicks;
    return averageTime;
}