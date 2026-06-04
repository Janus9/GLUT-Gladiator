
#pragma once 

// INCLUDES //

#include <SDL3/SDL.h>

#include <_common.h>         // For common headers
#include <_scene.h>          // My scene context
#include <_timerPlusPlus.h>   // For the timer class
#include <_menuManager.h>
#include <_sounds.h>         // Shared audio engine (owned here, not by _scene)

// DEFINES //

#define UPDATE_DELAY (1000.0f / 60.0f)	// Delay in milliseconds for 60 updates per second

// GLOBAL VARIABLES //

bool keys[256];				// Keyboard input array
bool active = true;			// Foreground-focus flag. False while the window is not the user's active window.
bool minimized = false;		// True while the window is minimized. Combined with !active to drive the suspended state.
bool fullscreen = true;		// True while application is fullscreened

int wWidth;					// Window width
int wHeight;				// Window height
Vec2f mouseScreenClipPos;	// Mouse position (screen coordinates as 0-1 where 0,0 is bottom left)
Vec2f mouseScreenPos;		// Mouse position (screen coordinates as pixels where 0,0 is top left)
bool LMB = false;			// Left mouse button held

// CLASS INSTANCE DECLARATIONS //
_logger Logger; 																// DEPRICATED -- Delete later
// std::unique_ptr<_scene> myScene = std::make_unique<_scene>(); 					// Singleton Scene
// std::unique_ptr<_timerPlusPlus> timer = std::make_unique<_timerPlusPlus>();		// Wont likely be used
// std::unique_ptr<_menuManager> menuManager = std::make_unique<_menuManager>();	// Singleton Menu Manager
// std::unique_ptr<_sounds> sharedSounds = std::make_unique<_sounds>();			// DEPRICATED -- Delete later

int main(int argc, char *argv[])
{
	// Initialization //
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		const std::string errorMessage = std::string("ERROR: SDL_Init failed") + SDL_GetError(); 
		SDL_Log(errorMessage.c_str());
		return EXIT_FAILURE;
	}

	// Window Creation //
	SDL_Window* window = SDL_CreateWindow("GLUT Gladiator", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!window) {
		const std::string errorMessage = std::string("ERROR: Window failed") + SDL_GetError(); 
		SDL_Log(errorMessage.c_str());
		SDL_Quit();
		return EXIT_FAILURE;
	}

	// Context Setting //
	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	if (!glContext) {
		const std::string errorMessage = std::string("ERROR: GL Context failed") + SDL_GetError(); 
		SDL_Log(errorMessage.c_str());
		SDL_Quit();
		return EXIT_FAILURE;
	}

	// Dont know what these do
	SDL_GL_MakeCurrent(window, glContext);		// Tells OpenGL our window is the given OpenGL context (to apply API calls into)
    SDL_GL_SetSwapInterval(1);					// Enable VSync

	// OpenGL Glew Initialization //
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK) {
		SDL_GL_DestroyContext(glContext);
        SDL_DestroyWindow(window);
		SDL_Quit();
		return EXIT_FAILURE;
	}

	bool running = true;
	while (running) {
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			// Exit Game //
			if (event.type == SDL_EVENT_QUIT) {
				running = false;
			}
		}

		SDL_GL_SwapWindow(window);			// Double buffering - Swap buffer
    }

	SDL_Quit();

	return EXIT_SUCCESS;
}
