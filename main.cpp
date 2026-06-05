// INCLUDES //

#include <SDL3/SDL.h>

#include <_common.h>         // For common headers
#include <_scene.h>          // My scene context
#include <_timerPlusPlus.h>   // For the timer class
#include <_menuManager.h>
#include <_sounds.h>         // Shared audio engine (owned here, not by _scene)

// DEFINES //

constexpr double UPDATE_DELAY = (1.0 / 60.0); // Delay in seconds for 60 updates per second

// GLOBAL VARIABLES //

const int windowSpawnWidth = 800;
const int windowSpawnHeight = 600;

bool active = true;			// Foreground-focus flag. False while the window is not the user's active window.
bool minimized = false;		// True while the window is minimized. Combined with !active to drive the suspended state.
bool fullscreen = true;		// True while application is fullscreened

int wWidth;					// Window width
int wHeight;				// Window height

uint64_t updatePreviousTime;
uint64_t inputPreviousTime;

InputState inputState;

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

	gameScene->reSize(wWidth, wHeight);
    
    _menuManager::setWindowDimensions({windowSpawnWidth, windowSpawnHeight});

	glViewport(0, 0, drawableW, drawableH);
}

// MOUSE MOVE HANDLER //
void handleMouseMove(const SDL_Event &event) {
	inputState.mouseScreenPos = {event.motion.x, event.motion.y};
	inputState.mouseScreenClipPos = {inputState.mouseScreenPos.x / wWidth, 1.0f - (inputState.mouseScreenPos.y / wHeight)};
}

// MOUSE BUTTON HANDLER //
void handleMouseButton(const SDL_Event &event, bool buttonDown) {
	switch (event.button.button) {
		case SDL_BUTTON_LEFT:
			inputState.LMB = buttonDown;
			break;
		case SDL_BUTTON_RIGHT:
			inputState.RMB = buttonDown;
			break;
	}
}

void handleDraw(SDL_Window* window) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (menuManager->getLoadedMenu() == MENU_GAME) {
		// Draw Game
		gameScene->drawScene();
	} else {
		// Draw Menu
		menuManager->drawMenuManager();
	}


	SDL_GL_SwapWindow(window);			// Double buffering - Swap buffer
}

void handleUpdate() {
	const uint64_t currentTime = SDL_GetTicksNS();
	const double dt = static_cast<double>(currentTime - updatePreviousTime) / 1000000000.0;	// Delta time (s)

	if (dt >= UPDATE_DELAY) {
		if (menuManager->getLoadedMenu() == MENU_GAME) {
			// Update Game
			if (menuManager->loadGame) {
				SDL_LogInfo(LOG_MAIN, "Entering Game State");
				gameScene->reSize(wWidth, wHeight);
				menuManager->loadGame = false;
			}
			gameScene->updateScene(dt, inputState);
		} else {
			// Update Menu
			menuManager->updateMenuManager(dt, inputState);
		}

		updatePreviousTime = currentTime;
	}
}

// MAIN ENTRY POINT //
int main(int argc, char *argv[])
{
	// Initialization //
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		const std::string errorMessage = std::string("ERROR: SDL_Init failed") + SDL_GetError(); 
		SDL_LogError(LOG_MAIN, errorMessage.c_str());
		return EXIT_FAILURE;
	}

	// Window Creation //
	SDL_Window* window = SDL_CreateWindow("GLUT Gladiator", windowSpawnWidth, windowSpawnHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!window) {
		const std::string errorMessage = std::string("ERROR: Window failed") + SDL_GetError(); 
		SDL_LogError(LOG_MAIN, errorMessage.c_str());
		SDL_Quit();
		return EXIT_FAILURE;
	}

	// Context Setting //
	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	if (!glContext) {
		const std::string errorMessage = std::string("ERROR: GL Context failed") + SDL_GetError(); 
		SDL_LogError(LOG_MAIN, errorMessage.c_str());
		SDL_Quit();
		return EXIT_FAILURE;
	}

	SDL_GL_MakeCurrent(window, glContext);		// Tells OpenGL our window is the given OpenGL context (to apply API calls into)
    SDL_GL_SetSwapInterval(1);					// Enable VSync

	// OpenGL Glut Initialization // -- Depricate at some point
	int argc2 = 1;
	char appName[] = "GLUT Gladiator";
	char* argv2[] = { appName, nullptr };

	glutInit(&argc2, argv2);

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

	menuManager = std::make_unique<_menuManager>();
	menuManager->initMenuManager(nullptr, gameScene.get());
	menuManager->loadMenu(MENU_HOME);

	updatePreviousTime = SDL_GetTicksNS();
	inputPreviousTime = SDL_GetTicks();

	bool running = true;
	while (running) {
		SDL_Event event;
		inputState.mouseWheelY = 0.0f;	// Reset mouse scroll event

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
				case SDL_EVENT_MOUSE_MOTION:
					handleMouseMove(event);
					break;
				case SDL_EVENT_MOUSE_BUTTON_DOWN:
					handleMouseButton(event,true);
					break;
				case SDL_EVENT_MOUSE_BUTTON_UP:
					handleMouseButton(event,false);
					break;
				case SDL_EVENT_MOUSE_WHEEL:
					inputState.mouseWheelY = event.wheel.y;
					gameScene->mouseScrollEvent(inputState);
					break;
				case SDL_EVENT_KEY_DOWN: {
					inputState.keys[event.key.scancode] = true;
					const uint64_t inputCurrentTime = SDL_GetTicks();
					const uint64_t dt = (inputCurrentTime - inputPreviousTime);
					if (dt > 100) {
						// Pause Event //
						if (inputState.keys[SDL_SCANCODE_ESCAPE]) {
							if (menuManager->getLoadedMenu() == MENU_GAME) {
								// In game - pause
								SDL_LogInfo(LOG_MAIN, "Pause game event");
								menuManager->loadMenu(MENU_PAUSE);
							} else {
								// In menu - unpause (if game loaded)
								if (gameScene->isInitialized()) {
									SDL_LogInfo(LOG_MAIN, "Unpause game event");
									menuManager->loadMenu(MENU_GAME);
								}
							}
						}
						gameScene->keyboardHandler(inputState);
						inputPreviousTime = inputCurrentTime;
					}
					break;
				}
				case SDL_EVENT_KEY_UP:
					inputState.keys[event.key.scancode] = false;
					break;
				default:
					break;
			}
		}

		// UPDATE //
		handleUpdate();
		
		// DRAW //
		handleDraw(window);
    }

	SDL_Quit();

	return EXIT_SUCCESS;
}
