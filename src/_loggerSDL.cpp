// src/_loggerSDL.cpp
#include <_loggerSDL.h>

#include <fstream>
#include <iostream>

static std::ofstream logFile;
static bool logToFile = true; // False means console.

/* -- COLOR DEFINITIONS -- */
#define LOG_RESET "\033[0m"

// TEXT COLORS
#define LOG_BLACK_TXT  "\033[30m"
#define LOG_RED_TXT    "\033[31m"
#define LOG_GREEN_TXT  "\033[32m"
#define LOG_YELLOW_TXT "\033[33m"
#define LOG_BLUE_TXT   "\033[34m"
#define LOG_PURPLE_TXT "\033[35m"
#define LOG_CYAN_TXT   "\033[36m"
#define LOG_WHITE_TXT  "\033[37m"

// BRIGHT TEXT COLORS
#define LOG_GRAY_TXT        "\033[90m"
#define LOG_BRIGHT_RED_TXT  "\033[91m"
#define LOG_BRIGHT_GREEN_TXT "\033[92m"
#define LOG_BRIGHT_YELLOW_TXT "\033[93m"
#define LOG_BRIGHT_BLUE_TXT "\033[94m"
#define LOG_BRIGHT_PURPLE_TXT "\033[95m"
#define LOG_BRIGHT_CYAN_TXT "\033[96m"
#define LOG_BRIGHT_WHITE_TXT "\033[97m"

// BACKGROUNDS
#define LOG_BLACK_BKG  "\033[40m"
#define LOG_RED_BKG    "\033[41m"
#define LOG_GREEN_BKG  "\033[42m"
#define LOG_YELLOW_BKG "\033[43m"
#define LOG_BLUE_BKG   "\033[44m"
#define LOG_PURPLE_BKG "\033[45m"
#define LOG_CYAN_BKG   "\033[46m"
#define LOG_WHITE_BKG  "\033[47m"

static const char* getPriorityName(bool prettyPrinting, SDL_LogPriority priority) {
    switch (priority) {
        case SDL_LOG_PRIORITY_VERBOSE: {
            if (prettyPrinting) {
                return LOG_WHITE_TXT "[VERBOSE]";
            } else {
                return "[VERBOSE]";
            }
        }
        case SDL_LOG_PRIORITY_DEBUG: {
            if (prettyPrinting) {
                return LOG_GREEN_TXT "[DEBUG]";
            } else {
                return "[DEBUG]";
            }
        }
        case SDL_LOG_PRIORITY_INFO: {
            if (prettyPrinting) {
                return LOG_BLUE_TXT "[INFO]";
            } else {
                return "[INFO]";
            }
        }
        case SDL_LOG_PRIORITY_WARN: {
            if (prettyPrinting) {
                return LOG_YELLOW_TXT "[WARNING]";
            } else {
                return "[WARNING]";
            }
        }
        case SDL_LOG_PRIORITY_ERROR: {
            if (prettyPrinting) {
                return LOG_RED_TXT "[ERROR]";
            } else {
                return "[ERROR]";
            }
        }
        case SDL_LOG_PRIORITY_CRITICAL: {
            if (prettyPrinting) {
                return LOG_BRIGHT_PURPLE_TXT "[CRITICAL]";
            } else {
                return "[CRITICAL]";
            }
        }
        default: {
            if (prettyPrinting) {
                return LOG_WHITE_TXT "[UNKNOWN]";
            } else {
                return "[UNKNOWN]";
            }
        }
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
        case LOG_MENU_PAGE:
            return "MENU_PAGE";
        case LOG_MENU_RENDER_OBJECT:
            return "MENU_RENDER_OBJECT";
        case LOG_PARTICLE_ENGINE:
            return "PARTICLE_ENGINE";
        case LOG_BULLET_ENGINE:
            return "BULLET_MANAGER";
        case LOG_WORLD:
            return "WORLD";
        case LOG_ENEMY_MANAGER:
            return "ENEMY_MANAGER";
        case LOG_PICKUPS:
            return "PICKUP_MANAGER";
        case LOG_SOUND:
            return "SOUND_ENGINE";
        default:
            return "UNKNOWN";
    }
}

// Re-routes the logger function calls here to add pretty-printing
static void SDLCALL gameLogOutput([[maybe_unused]] void* userdata, int category, SDL_LogPriority priority, const char* message) {
    const char* categoryName = getCategoryName(category);

    if (logToFile) {
        logFile << getPriorityName(false, priority) << " [" << categoryName << "] " << message << "\n";
    } else {
        std::cout << getPriorityName(true, priority) << " [" << categoryName << "] " << message << LOG_RESET << "\n";
    }

}

void initSDLLogger() {
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
    SDL_SetLogOutputFunction(gameLogOutput, nullptr);

    if (logToFile) {
        logFile.open("logs/game.log", std::ios::trunc | std::ios::out);
    }

    GG_LOG_INFO(LOG_MAIN, "SDL logger initialized");
}