#ifndef _LOGGER_H_
#define _LOGGER_H_


/* -- COLOR DEFINITIONS -- */
#define ESC "\033["         
//#define RESET "\033[m"
#define RESET "\033[m"
// TEXT COLORS //
#define BLACK_TXT "30;"
#define RED_TXT "31;"
#define GREEN_TXT "32;"
#define YELLOW_TXT "33;"
#define BLUE_TXT "34;"
#define PURPLE_TXT "35;"
#define CYAN_TXT "36;"
#define WHITE_TXT "37;"
// BACKGROUNDS //
#define BLACK_BKG "40m"
#define RED_BKG "41m"
#define GREEN_BKG "42m"
#define YELLOW_BKG "43m"
#define BLUE_BKG "44m"
#define PURPLE_BKG "45m"
#define CYAN_BKG "46m"
#define WHITE_BKG "47m"

#include <_common.h>

// Enum for where the message will be logged to. Console, file, or both
enum LogPool {
    LOG_FILE = 0,
    LOG_CONSOLE = 1,
    LOG_BOTH = 2
};

// Enum for benchmark units to use on output. Auto will select the most appropriate unit based on the time taken
enum BenchmarkUnit {
    BENCH_SECOND = 0,
    BENCH_MILLISECOND = 1,
    BENCH_MICROSECOND = 2,
    BENCH_NANOSECOND = 3,
    // Automatically selects a unit based on which is the first in line to be non-zero
    // This may not be the most precise unit but will give a readable output
    BENCH_AUTO = 4,
};

// Logger class to handle logging messages to the console and/or a file, as well as benchmarking code execution time. Benchmarks are logged as info messages with the benchmark name and time taken.
class _logger {
    private:
        std::chrono::high_resolution_clock::time_point benchmarkStartTime;
        std::string GetTimestamp(bool includeDate);
        std::ofstream logFileStream;
        // Filepath to the primary log file. There can me multiple log files but this is used by default.
        std::string PrimaryLogFilePath = "";
        // How many log files are attached to the logger
        int numLogFiles = 0;
    protected:
    public:
        // Closes the logger and any open log files. Automatically called in destructor.
        void CloseLogger();
        // Clears the contents of the primary log file. Ran automatically on InitLogger unless specified.
        void ClearLogger();
        _logger();
        ~_logger();
        // Sets up the logger to log to a file at the given filepath.
        void InitLogger(const std::string& filepath, bool clearExisting = true);

        /* Logging Functions */

        void LogError(std::string message,LogPool pool = LOG_CONSOLE);
        void LogWarning(std::string message,LogPool pool = LOG_CONSOLE);
        void LogInfo(std::string message,LogPool pool = LOG_CONSOLE);
        void LogDebug(std::string message,LogPool pool = LOG_CONSOLE);

        /* Benchmark Functions 
        * The benchmarker allows performance metrics to be logged and recorded
        */
        
        // Starts the benchmark timer for a given benchmark name. This is used before running the function/code to bench.
        void StartBenchmark();
        // Ends the benchmark timer for a given benchmark name and logs the result.
        void EndBenchmark(const std::string& benchmarkName);
        void EndBenchmark(const std::string& benchmarkName, BenchmarkUnit unit);
        void EndBenchmark(const std::string& benchmarkName, LogPool pool);
        void EndBenchmark(const std::string& benchmarkName, LogPool pool, BenchmarkUnit);
};

extern _logger Logger;

#endif // _LOGGER_H_