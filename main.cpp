
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

const int windowSpawnWidth = 800;
const int windowSpawnHeight = 600;

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
std::unique_ptr<_scene> gameScene; 												// Singleton Scene
// std::unique_ptr<_timerPlusPlus> timer = std::make_unique<_timerPlusPlus>();  // Wont likely be used
std::unique_ptr<_menuManager> menuManager;										// Singleton Menu Manager
// std::unique_ptr<_sounds> sharedSounds = std::make_unique<_sounds>();			// DEPRICATED -- Delete later

// SCREEN RESIZE HANDLER //
void handleWindowResize(SDL_Window* window) {
	int newWindowW = 0;
    int newWindowH = 0;

    int drawableW = 0;
    int drawableH = 0;

    SDL_GetWindowSize(window, &newWindowW, &newWindowH);
    SDL_GetWindowSizeInPixels(window, &drawableW, &drawableH);

    if (newWindowW <= 0 || newWindowH <= 0) {
        return;
    }

    if (drawableW <= 0 || drawableH <= 0) {
        return;
    }

    wWidth = newWindowW;
    wHeight = newWindowH;

    glViewport(0, 0, drawableW, drawableH);

    _menuManager::setWindowDimensions({windowSpawnWidth, windowSpawnHeight});
}

// MAIN ENTRY POINT //
int main(int argc, char *argv[])
{
	// Initialization //
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		const std::string errorMessage = std::string("ERROR: SDL_Init failed") + SDL_GetError(); 
		SDL_Log(errorMessage.c_str());
		return EXIT_FAILURE;
	}

	// Window Creation //
	SDL_Window* window = SDL_CreateWindow("GLUT Gladiator", windowSpawnWidth, windowSpawnHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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

	gameScene = std::make_unique<_scene>();
	gameScene->initGL();

	handleWindowResize(window);	// Force resize event to sit window dimension parameters + OpenGL window params				

	gameScene->reSize(wWidth, wHeight);
	
	menuManager = std::make_unique<_menuManager>();
	menuManager->initMenuManager(nullptr, gameScene.get());
	menuManager->loadMenu(MENU_HOME);

	bool running = true;
	while (running) {
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				// Exit Game //
				case SDL_EVENT_QUIT:
					running = false;
					break;
				// Window Resize //
				case SDL_EVENT_WINDOW_RESIZED:
					handleWindowResize(window);
					break;
				default:
					break;
			}
		}

		// DRAW //
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		menuManager->drawMenuManager();

		SDL_GL_SwapWindow(window);			// Double buffering - Swap buffer
    }

	SDL_Quit();

	return EXIT_SUCCESS;
}
