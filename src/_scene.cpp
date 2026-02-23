#include "_scene.h"

_scene::_scene()
{
    //ctor
}

_scene::~_scene()
{
    //dtor
    delete myLight;
    myLight = nullptr;

    delete myModel;
    myModel = nullptr;

    delete myWorld;
    myWorld = nullptr;

    delete test_player;
    test_player = nullptr;

    delete fpsTimer;
    fpsTimer = nullptr;

    delete drawBenchTimer;
    drawBenchTimer = nullptr;

    delete updateBenchTimer;
    updateBenchTimer = nullptr;
}


GLint _scene::initGL()
{
    glewInit();
    glClearColor(0.0,0.0,0.0,1.0); //Intended to change the background color. 0001 is black
    glClearDepth(1.0); //Gives depth to the environment by having color both in the front and back. Depth-test value
    glEnable(GL_DEPTH_TEST); //Will ensure the depth of the z-coordinate is accurate through enabling depth-test
    glDepthFunc(GL_LEQUAL); //Depth-test is less than or equal to--> true for less or equal

    // glEnable(GL_LIGHTING); //Enable lighting within the scene to be initialized (8 total lights)
    // glEnable(GL_LIGHT0); //Light is instantiated at this module
    // glEnable(GL_COLOR_MATERIAL); //To theoretically instantiate a base color on a 3D object (likely won't be used)

    // its 2D w/ pixels so no need for lighting
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // CLASS INIT //

    inputTimer.reset(); 
    fpsTimer->reset();  
    // tester player
    test_player->spriteInit("images/test_player.png", 1, 1); 
    
    test_player->pos = {width/2.0f, height/2.0f}; // Start player in the center of the screen

    myLight->setLight(GL_LIGHT0); // The light onto the object from the pointer is set to be the instantiated light from before
    myModel->initModel(); // The model is initialized from the pointer to the model class
    myWorld->initWorld(); // Initialize the world
    
    debugTimer.reset();     
    drawBenchTimer->reset(); 
    updateBenchTimer->reset();
    
    myWorld->SET_DisplayChunkBorders(displayChunkBorders); // Set initial chunk border display state
    
    vbo1.vboInit();
    texture1.loadTexture("images/wood.png");

    return true;
}

void _scene::reSize(GLint width, GLint height)
{
    this->width = width;
    this->height = height;
    Logger.LogInfo("Resizing window to width: " + std::to_string(width) + " and height: " + std::to_string(height), LOG_BOTH);
    GLfloat aspectRatio = (GLfloat) width/ (GLfloat) height; //Intended to keep track of window resize
    glViewport(0,0,width,height); // Integer values taken in to take in view. Setting Viewport
    glMatrixMode(GL_PROJECTION); // Turns the projection into a matrix. Initiate the projection
    glLoadIdentity(); // Keep value's axes (matrix * identity matrix = matrix). Initialize the matrix with identity matrix
    
    test_player->pos = {width/2.0f, height/2.0f}; // Start player in the center of the screen
    
    applyCamera();

    glMatrixMode(GL_MODELVIEW); // Inputs into matrix depend on the placement of objects in the matrix overview. Initiate model and view matrix

    glLoadIdentity();// Keep value's axes (matrix * identity matrix = matrix). Initialize the matrix with identity matrix
}

//Scene is running in a loop
void _scene::drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    //clear buffers
    glLoadIdentity(); //Whichever state the scene is in it will stay there

    applyCamera(); // Apply camera transformations

    double startTime = drawBenchTimer->getTicks(); 
    myWorld->drawWorld(); // Draw the world
    double endTime = drawBenchTimer->getTicks();
    double deltaTime = endTime - startTime;
    drawBenchmark.benchmarkTicks += deltaTime;
    drawBenchmark.numIterations++;

    test_player->drawSprite(); // Draw the test player sprite

    // For FPS measuring
    frameCount++;

    if(fpsTimer->getTicks() > fpsPrintInterval) {
        debugPrintFPS(); 
    }
}

// Runs in loop 60 times per second. dt is in ms.
void _scene::updateScene(double dt)
{
    // Get chunk position (coordinates)
    playerChunkPos.x = (int)floor(playerPos.x / (16 * TILE_W));
    playerChunkPos.y = (int)floor(playerPos.y / (16 * TILE_H));

    dt = dt / 1000.0; // Convert dt to seconds for easier calculations
    if (cameraFree) {
        if(W) cameraY += cameraSpeed*dt;
        if(A) cameraX -= cameraSpeed*dt; 
        if(S) cameraY -= cameraSpeed*dt;
        if(D) cameraX += cameraSpeed*dt; 
    } else {
        if(W) playerPos.y += cameraSpeed*dt;
        if(A) playerPos.x -= cameraSpeed*dt; 
        if(S) playerPos.y -= cameraSpeed*dt;
        if(D) playerPos.x += cameraSpeed*dt;
        // If camera is not free, it will track the player (centered on player)
        test_player->pos = playerPos; // Update player position based on input

        cameraX = playerPos.x;
        cameraY = playerPos.y;
    }

    bool inLoadedChunk = myWorld->isChunkLoaded(playerChunkPos.x, playerChunkPos.y);
    if (!inLoadedChunk) {
        myWorld->generateChunk(playerChunkPos.x, playerChunkPos.y);
    }

    if (debugTimer.getTicks() > debugPrintInterval) 
    {
        debugPrint(); 
        debugTimer.reset(); 
    }
}

void _scene::debugPrint()
{
    Logger.LogDebug("Debug Print: W=" + std::to_string(W) + " A=" + std::to_string(A) + " S=" + std::to_string(S) + " D=" + std::to_string(D), LOG_CONSOLE);
    Logger.LogDebug("Camera Position: (" + std::to_string(cameraX) + ", " + std::to_string(cameraY) + ") Zoom: " + std::to_string(cameraZoom), LOG_CONSOLE);
    Logger.LogDebug("Player is in chunk: (" + std::to_string(playerChunkPos.x) + ", " + std::to_string(playerChunkPos.y) + ")", LOG_CONSOLE);
    
    bool inLoadedChunk = myWorld->isChunkLoaded(playerChunkPos.x, playerChunkPos.y);
    Logger.LogDebug("Player is in loaded chunk: " + std::string(inLoadedChunk ? "YES" : "NO"), LOG_CONSOLE);

    myWorld->debugPrint();
}

void _scene::debugPrintFPS() {
    sceneFPS = frameCount / (fpsTimer->getTicks() / 1000.0); // Calculate FPS based on frames and time
    Logger.LogInfo("Current FPS: " + std::to_string(sceneFPS), LOG_CONSOLE);
    frameCount = 0; // Reset frame count after printing FPS
    fpsTimer->reset(); // Reset the timer for the next FPS calculation
}

void _scene::keyboardHandler(WPARAM wParam)
{
    if (inputTimer.getTicks() > 200) // 200 ms debounce time for toggle keys
    {
        switch (wParam)
        {
            case 192: // "~"
                commandHandler(); // Enter command mode to input text-based commands
                break;
            case 221: // "]"
                debugEnabled = !debugEnabled; 
                Logger.LogInfo("Toggled debug mode: " + std::string(debugEnabled ? "ON" : "OFF"), LOG_CONSOLE);
                break;
            case 219: // "["
                inputDebugEnabled = !inputDebugEnabled; 
                Logger.LogInfo("Toggled input debug mode: " + std::string(inputDebugEnabled ? "ON" : "OFF"), LOG_CONSOLE);
                break;
            case 220: // "\"
                cameraFree = !cameraFree;
                Logger.LogInfo("Toggled camera free mode: " + std::string(cameraFree ? "ON" : "OFF"), LOG_CONSOLE);
                break;
            case 122: // "F11"
                displayChunkBorders = !displayChunkBorders;
                myWorld->SET_DisplayChunkBorders(displayChunkBorders); // Toggle chunk border display
                Logger.LogInfo("Toggled chunk border display: " + std::string(displayChunkBorders ? "ON" : "OFF"), LOG_CONSOLE);
                break;
        }
        inputTimer.reset(); // Reset the timer after handling a toggle key
    }
}

int _scene::winMsg(HWND	hWnd, UINT uMsg, WPARAM	wParam, LPARAM lParam)
{
    switch (uMsg)								
    {
        // Keypress
        case WM_KEYDOWN:
            if (inputDebugEnabled) Logger.LogDebug("Key Pressed: " + std::to_string(wParam), LOG_CONSOLE); //Log the key that was pressed
            keyboardHandler(wParam);
            switch (wParam)
            {
                case 'W':
                    W = true;
                    break;
                case 'A':
                    A = true;
                    break;
                case 'S':
                    S = true;
                    break;
                case 'D':
                    D = true;
                    break;
            }
            break;
        // Key release
        case WM_KEYUP:
            if (inputDebugEnabled) Logger.LogDebug("Key Released: " + std::to_string(wParam), LOG_CONSOLE); //Log the key that was released
            switch (wParam)
            {
                case 'W':
                    W = false;
                    break;
                case 'A':
                    A = false;
                    break;
                case 'S':
                    S = false;
                    break;
                case 'D':
                    D = false;
                    break;
            }
            break;
        // Left Mouse button down
        case WM_LBUTTONDOWN:
            if (inputDebugEnabled) Logger.LogDebug("Left Mouse Button Down at (" + std::to_string(LOWORD(lParam)) + ", " + std::to_string(HIWORD(lParam)) + ")", LOG_CONSOLE); //Log the position of the mouse when left button is pressed
            break;
        // Right Mouse button down
        case WM_RBUTTONDOWN:
            if (inputDebugEnabled) Logger.LogDebug("Right Mouse Button Down at (" + std::to_string(LOWORD(lParam)) + ", " + std::to_string(HIWORD(lParam)) + ")", LOG_CONSOLE); //Log the position of the mouse when right button is pressed
            break;
        // Left Mouse button up
        case WM_LBUTTONUP:
            if (inputDebugEnabled) Logger.LogDebug("Left Mouse Button Up at (" + std::to_string(LOWORD(lParam)) + ", " + std::to_string(HIWORD(lParam)) + ")", LOG_CONSOLE); //Log the position of the mouse when left button is released
            break;
        // Right Mouse button up
        case WM_RBUTTONUP:
            if (inputDebugEnabled) Logger.LogDebug("Right Mouse Button Up at (" + std::to_string(LOWORD(lParam)) + ", " + std::to_string(HIWORD(lParam)) + ")", LOG_CONSOLE); //Log the position of the mouse when right button is released
            break;
        // Mouse move
        case WM_MOUSEMOVE:
            if (inputDebugEnabled) Logger.LogDebug("Mouse Move at (" + std::to_string(LOWORD(lParam)) + ", " + std::to_string(HIWORD(lParam)) + ")", LOG_CONSOLE); //Log the position of the mouse when it moves
            break;
        // Mouse wheel
        case WM_MOUSEWHEEL:
            if (inputDebugEnabled) Logger.LogDebug("Mouse Wheel: " + std::to_string((short)HIWORD(wParam)) + " at (" + std::to_string(LOWORD(lParam)) + ", " + std::to_string(HIWORD(lParam)) + ")", LOG_CONSOLE); //Log the amount of scroll and position of the mouse when the wheel is scrolled
            if ((short)HIWORD(wParam) > 0 && cameraZoom < 9.0f) { 
                // Scroll up
                cameraZoom *= 1.1f; // Zoom in by increasing the zoom factor
            } else if ((short)HIWORD(wParam) < 0) {
                // Scroll down
                cameraZoom /= 1.1f; // Zoom out by decreasing the zoom factor
            }
            break;
    }
    return 0;
}

void _scene::commandHandler() {
    std::string command;
    std::cout << "Enter command: ";
    std::cin >> command;
    // command checking -- hashmap would be better
    if (command == "gg_help") {
        // show all commands
        std::cout << "Available commands:\n";
    } else if (command == "gg_toggle_debug") {
        debugEnabled = !debugEnabled;
        std::cout << "Debug mode: " << (debugEnabled ? "ON" : "OFF") << std::endl;
    } else if (command == "gg_toggle_input_debug") {
        inputDebugEnabled = !inputDebugEnabled;
        std::cout << "Input debug mode: " << (inputDebugEnabled ? "ON" : "OFF") << std::endl;
    } else if (command == "gg_set_camera_speed") {
        float newSpeed;
        std::cout << "Enter new camera speed (default is " << cameraSpeed << "): ";
        std::cin >> newSpeed;
        cameraSpeed = newSpeed;
        std::cout << "Camera speed set to: " << cameraSpeed << std::endl;
    } else {
        std::cout << "Unknown command: " << command << std::endl;
    }
}

void _scene::applyCamera() {
    float halfWidth = width * 0.5f / cameraZoom; // Adjust half-width based on zoom level
    float halfHeight = height * 0.5f / cameraZoom; // Adjust half-height based on zoom level

    float left = cameraX - halfWidth;
    float right = cameraX + halfWidth;
    float bottom = cameraY - halfHeight;
    float top = cameraY + halfHeight;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // We use Orthographic projection because its a 2D game and it makes it easy to maintain scale via pixels
    glOrtho(
        left, right,    // left->right = zoom of 1 means 1 world unit (1 pixel) is visible. Zooming in makes the world bigger and zooming out makes the world smaller
        bottom, top,    // top->bottom = zoom of 1 means 1 world unit (1 pixel) is visible. Zooming in makes the world bigger and zooming out makes the world smaller
        -1.0, 1.0       // z index from -1 to 1 is visible (everything else clips and is not visible)
    ); 
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

