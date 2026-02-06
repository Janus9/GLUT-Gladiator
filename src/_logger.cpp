#include <_logger.h>

void _logger::CloseLogger() {
    if (logFileStream.is_open()) {
        logFileStream.close();
    }
}

void _logger::ClearLogger() {
    if (logFileStream.is_open()) {
        logFileStream.close();
    }
    logFileStream.open(PrimaryLogFilePath, std::ios::out | std::ios::trunc);
    if (!logFileStream.is_open()) {
        LogError("Failed to clear log file: " + PrimaryLogFilePath, LOG_CONSOLE);
    }
}

_logger::_logger() {
    // ctor
}

_logger::~_logger() {
    // dtor
    CloseLogger();
}

std::string _logger::GetTimestamp(bool includeDate) {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);

    std::ostringstream oss;
    if (includeDate) {
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    } else {
        oss << std::put_time(&tm, "%H:%M:%S")
            << '.' << std::setfill('0') << std::setw(3) << ms.count();
    }
    return oss.str();
}

void _logger::InitLogger(const std::string& filepath, bool clearExisting) {
    PrimaryLogFilePath = filepath;
    numLogFiles = 1; 
    
    logFileStream.open(PrimaryLogFilePath, std::ios::out | std::ios::app);
    if (!logFileStream.is_open()) {
        LogError("Failed to open log file: " + PrimaryLogFilePath, LOG_CONSOLE);
        return;
    }
    if (clearExisting) {
        ClearLogger();
    }

    logFileStream << "----- Logger Initialized at " << GetTimestamp(true) << " -----" << std::endl;
}

void _logger::LogError(std::string message, LogPool pool) {
    if (pool == LOG_FILE || pool == LOG_BOTH) {
        if (logFileStream.is_open()) {
            logFileStream << "[ERROR " << GetTimestamp(false) << "] " << message << std::endl;
        }
        if(pool != LOG_BOTH) return;
    }
    std::cerr << ESC << RED_TXT << BLACK_BKG << "[ERROR " << GetTimestamp(false) << "] " << message << RESET << std::endl;
}

void _logger::LogWarning(std::string message, LogPool pool) {
    if (pool == LOG_FILE || pool == LOG_BOTH) {
        if (logFileStream.is_open()) {
            logFileStream << "[WARNING " << GetTimestamp(false) << "] " << message << std::endl;
        }
        if(pool != LOG_BOTH) return;
    }
    std::cerr << ESC << YELLOW_TXT << BLACK_BKG << "[WARNING " << GetTimestamp(false) << "] " << message << RESET << std::endl;
}

void _logger::LogInfo(std::string message, LogPool pool) {
    if (pool == LOG_FILE || pool == LOG_BOTH) {
        if (logFileStream.is_open()) {
            logFileStream << "[INFO " << GetTimestamp(false) << "] " << message << std::endl;
        }
        if(pool != LOG_BOTH) return;
    }
    std::cout << ESC << GREEN_TXT << BLACK_BKG << "[INFO " << GetTimestamp(false) << "] " << message << RESET << std::endl;
}

void _logger::LogDebug(std::string message, LogPool pool) {
    if (pool == LOG_FILE || pool == LOG_BOTH) {
        if (logFileStream.is_open()) {
            logFileStream << "[DEBUG " << GetTimestamp(false) << "] " << message << std::endl;
        }
        if(pool != LOG_BOTH) return;
    }
    std::clog << ESC << CYAN_TXT << BLACK_BKG << "[DEBUG " << GetTimestamp(false) << "] " << message << RESET << std::endl;
}

void _logger::StartBenchmark() {
    // Implementation for starting a benchmark timer
    benchmarkStartTime = std::chrono::high_resolution_clock::now();
}

void _logger::EndBenchmark(const std::string& benchmarkName) {
    EndBenchmark(benchmarkName, LOG_CONSOLE, BENCH_MILLISECOND);
}

void _logger::EndBenchmark(const std::string& benchmarkName, LogPool pool) {
    EndBenchmark(benchmarkName, pool, BENCH_MILLISECOND);
}

void _logger::EndBenchmark(const std::string& benchmarkName, BenchmarkUnit unit) {
    EndBenchmark(benchmarkName, LOG_CONSOLE, unit);
}

void _logger::EndBenchmark(const std::string& benchmarkName,LogPool pool, BenchmarkUnit unit) {
    // Implementation for ending a benchmark timer and logging the result
    auto benchmarkEndTime = std::chrono::high_resolution_clock::now();
    int64_t duration;
    std::string endUnit;
    switch(unit) {
        case BENCH_SECOND:
            duration = std::chrono::duration_cast<std::chrono::seconds>(benchmarkEndTime - benchmarkStartTime).count();
            endUnit = "s";
            break;
        case BENCH_MILLISECOND:
            duration = std::chrono::duration_cast<std::chrono::milliseconds>(benchmarkEndTime - benchmarkStartTime).count();
            endUnit = "ms";
            break;
        case BENCH_MICROSECOND:
            duration = std::chrono::duration_cast<std::chrono::microseconds>(benchmarkEndTime - benchmarkStartTime).count();
            endUnit = "us";
            break;
        case BENCH_NANOSECOND:
            duration = std::chrono::duration_cast<std::chrono::nanoseconds>(benchmarkEndTime - benchmarkStartTime).count();
            endUnit = "ns";
            break;
        case BENCH_AUTO:
            // Auto finds the first unit that is non-zero to use and ends at nanoseconds if all else fails
            duration = std::chrono::duration_cast<std::chrono::seconds>(benchmarkEndTime - benchmarkStartTime).count();
            if (duration > 0) {
                endUnit = "s";
                break;
            }
            duration = std::chrono::duration_cast<std::chrono::milliseconds>(benchmarkEndTime - benchmarkStartTime).count();
            if (duration > 0) {
                endUnit = "ms";
                break;
            }
            duration = std::chrono::duration_cast<std::chrono::microseconds>(benchmarkEndTime - benchmarkStartTime).count();
            if (duration > 0) {
                endUnit = "us";
                break;
            }
            duration = std::chrono::duration_cast<std::chrono::nanoseconds>(benchmarkEndTime - benchmarkStartTime).count();
            endUnit = "ns";
            break;
    }
    std::string message = benchmarkName + "' completed in " + std::to_string(duration) + " " + endUnit + ".";
    if (pool == LOG_FILE || pool == LOG_BOTH) {
        if (logFileStream.is_open()) {
            logFileStream << "[BENCHMARK " << GetTimestamp(false) << "] " << message << std::endl;
        }
        if(pool != LOG_BOTH) return;
    }
    std::clog << ESC << WHITE_TXT << BLACK_BKG << "[BENCHMARK " << GetTimestamp(false) << "] " << message << RESET << std::endl;
}