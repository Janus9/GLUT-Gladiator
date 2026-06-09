// include/_loggerSDL.h
#pragma once

#include <SDL3/SDL_log.h>

enum GameLogCategory {
    LOG_MAIN = SDL_LOG_CATEGORY_APPLICATION,
    LOG_SCENE,
    LOG_MENU_MANAGER,
    LOG_PARTICLE_ENGINE,
    LOG_BULLET_ENGINE
};

void initSDLLogger();