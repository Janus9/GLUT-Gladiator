/**
 * -- Log Priorities -- 
 * 
 * Debug: Devloper oriented details. Very noisy. 
 * Info: Normal major lifecycle events. Meaningful milestones.
 * Warning: Something unexpected happened, but recovery is possible without user noticing.
 * Error: An operation failed, a functionality is broken. Recovery is possible but user may notice.
 * Critical: The application has entered a state that cannot be recovered from. Game will crash. 
 * 
 * ** Trace and Verbose are not implemented at this time **
 * 
 * -- Release Modes --
 * 
 * Debug: Full logging enabled, no optimization.
 * Release: Some logging enabled, either console or file debugging. Same optimization as prod (production simulation)
 * Production: Minimal logging enabled and only to file. Maximum optimzation.
 * 
 *  | Priority |   Debug |      Release | Production |
 *  | -------- | ------- | ------------ | ---------- |
 *  | Debug    | Console |           No |         No |
 *  | Info     | Console | Console/file |         No |
 *  | Warn     | Console | Console/file |       File |
 *  | Error    | Console | Console/file |       File |
 *  | Critical | Console | Console/file |       File |
 *
 */

#pragma once

#include <SDL3/SDL_log.h>

// Protect against multiple build configurations //
#if (defined(GAME_DEBUG) + defined(GAME_RELEASE) + defined(GAME_PRODUCTION)) > 1
    #error Multiple game build configurations defined
#endif

// Pre-compiler ignore debugging operations per build configuration //
#if defined(GAME_DEBUG)

    #define GG_LOG_DEBUG(category, ...) \
        SDL_LogDebug(category, __VA_ARGS__)

    #define GG_LOG_INFO(category, ...) \
        SDL_LogInfo(category, __VA_ARGS__)

    #define GG_LOG_WARN(category, ...) \
        SDL_LogWarn(category, __VA_ARGS__)

    #define GG_LOG_ERROR(category, ...) \
        SDL_LogError(category, __VA_ARGS__)

    #define GG_LOG_CRITICAL(category, ...) \
        SDL_LogCritical(category, __VA_ARGS__)

#elif defined(GAME_RELEASE)

    #define GG_LOG_DEBUG(category, ...) \
        ((void)0)

    #define GG_LOG_INFO(category, ...) \
        SDL_LogInfo(category, __VA_ARGS__)

    #define GG_LOG_WARN(category, ...) \
        SDL_LogWarn(category, __VA_ARGS__)

    #define GG_LOG_ERROR(category, ...) \
        SDL_LogError(category, __VA_ARGS__)

    #define GG_LOG_CRITICAL(category, ...) \
        SDL_LogCritical(category, __VA_ARGS__)

#elif defined(GAME_PRODUCTION)

    #define GG_LOG_DEBUG(category, ...) \
        ((void)0)

    #define GG_LOG_INFO(category, ...) \
        ((void)0)

    #define GG_LOG_WARN(category, ...) \
        SDL_LogWarn(category, __VA_ARGS__)

    #define GG_LOG_ERROR(category, ...) \
        SDL_LogError(category, __VA_ARGS__)

    #define GG_LOG_CRITICAL(category, ...) \
        SDL_LogCritical(category, __VA_ARGS__)

#else

    #error No valid game configuration found

#endif

enum GameLogCategory {
    LOG_MAIN = SDL_LOG_CATEGORY_APPLICATION,
    LOG_SCENE,
    LOG_MENU_MANAGER,
    LOG_MENU_PAGE,
    LOG_MENU_RENDER_OBJECT,
    LOG_PARTICLE_ENGINE,
    LOG_BULLET_ENGINE,
    LOG_WORLD,
    LOG_ENEMY_MANAGER,
    LOG_PICKUPS,
    LOG_SHADER,
    LOG_SOUND
};

void initSDLLogger();