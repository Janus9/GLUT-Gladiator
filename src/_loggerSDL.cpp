// src/_loggerSDL.cpp
#include <_loggerSDL.h>

#include <iostream>

static const char* getPriorityName(SDL_LogPriority priority) {
    switch (priority) {
        case SDL_LOG_PRIORITY_VERBOSE:
            return "VERBOSE";
        case SDL_LOG_PRIORITY_DEBUG:
            return "DEBUG";
        case SDL_LOG_PRIORITY_INFO:
            return "INFO";
        case SDL_LOG_PRIORITY_WARN:
            return "WARN";
        case SDL_LOG_PRIORITY_ERROR:
            return "ERROR";
        case SDL_LOG_PRIORITY_CRITICAL:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}

static const char* getCategoryName(int category) {
    switch (category) {
        case LOG_MAIN:
            return "MAIN";
        case LOG_SCENE:
            return "SCENE";
        case LOG_MENU_MANAGER:
            return "MENU_MANAGER";
        default:
            return "UNKNOWN";
    }
}

// Re-routes the logger function calls here to add pretty-printing
static void SDLCALL gameLogOutput(void* userdata, int category, SDL_LogPriority priority, const char* message) {
    const char* priorityName = getPriorityName(priority);
    const char* categoryName = getCategoryName(category);

    std::cout << "[" << priorityName << "] [" << categoryName << "] " << message << "\n";
}

void initSDLLogger() {
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
    SDL_SetLogOutputFunction(gameLogOutput, nullptr);

    SDL_LogInfo(LOG_MAIN, "SDL logger initialized");
}