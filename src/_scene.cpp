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

    delete testPlayer;
    testPlayer = nullptr;

    delete fpsTimer;
    fpsTimer = nullptr;

    delete drawWorldBenchmark;
    drawWorldBenchmark = nullptr;

    delete testSounds;
    testSounds = nullptr;

    delete testUnit;
    testUnit = nullptr;

    delete fpsText;
    fpsText = nullptr;

    delete posText;
    posText = nullptr;

    delete chunkText;
    chunkText = nullptr;

    delete mouseScreenText;
    mouseScreenText = nullptr;

    delete mouseWorldText;
    mouseWorldText = nullptr;

    delete testText;
    testText = nullptr;
}   


GLint _scene::initGL()
{
    glewInit();
    glClearColor(0.0,0.0,0.0,1.0); //Intended to change the background color. 0001 is black
    glClearDepth(1.0); //Gives depth to the environment by having color both in the front and back. Depth-test value
    glEnable(GL_DEPTH_TEST); //Will ensure the depth of the z-coordinate is accurate through enabling depth-test
    glDepthFunc(GL_LEQUAL); //Depth-test is less than or equal to--> true for less or equal

    // its 2D w/ pixels so no need for lighting
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // CLASS INIT //

    inputTimer.reset(); 
    fpsTimer->reset();  

    // Tester Player //

    testPlayer->setupSprite("WALK");
    testPlayer->getSprite("WALK")->initSprite("images/walk.png", 8, 6, sprite_direction::LEFT,12); // No natural direction due to top down

    testPlayer->scale = {0.8f, 0.8f};
    testPlayer->pos = {0.0f, 0.0f}; // Start player in the center of the screen

    testPlayer->getSprite("WALK")->createSpriteAction(sprite_action("WALK_DOWN",0,0,7));
    testPlayer->getSprite("WALK")->createSpriteAction(sprite_action("WALK_DOWN_LEFT",1,0,7));
    testPlayer->getSprite("WALK")->createSpriteAction(sprite_action("WALK_UP_LEFT",2,0,7));
    testPlayer->getSprite("WALK")->createSpriteAction(sprite_action("WALK_UP",3,0,7));
    testPlayer->getSprite("WALK")->createSpriteAction(sprite_action("WALK_UP_RIGHT",4,0,7));
    testPlayer->getSprite("WALK")->createSpriteAction(sprite_action("WALK_DOWN_RIGHT",5,0,7));

    // Tester Unit
    // testUnit->initUnit();
    // testUnit->spriteInit("images/m2_50.png",4,1);
    
    // testUnit->pos = {0.0f, 0.0f};

    fpsText->initText("FPS: ",{10.0f,height - 80.0f}, {1.0f,1.0f});
    fpsText->color = {1.0f,1.0f,1.0f}; 

    posText->initText("POS",{10.0f,height - 120.0f}, {1.0f,1.0f});
    posText->color = {1.0f,1.0f,1.0f}; 

    chunkText->initText("CHUNK",{10.0f,height - 160.0f}, {1.0f,1.0f});
    chunkText->color = {1.0f,1.0f,1.0f}; 

    mouseScreenText->initText("MOUSES",{10.0f,height - 180.0f}, {1.0f,1.0f});
    mouseScreenText->color = {1.0f,1.0f,1.0f}; 

    mouseWorldText->initText("MOUSEW",{10.0f,height - 200.0f}, {1.0f,1.0f});
    mouseWorldText->color = {1.0f,1.0f,1.0f}; 

    testText->initText("TEST",{10.0f,height - 220.0f}, {1.0f,1.0f});
    testText->color = {1.0f,1.0f,1.0f}; 

    myLight->setLight(GL_LIGHT0); // The light onto the object from the pointer is set to be the instantiated light from before
    myModel->initModel(); // The model is initialized from the pointer to the model class
    myWorld->initWorld(); // Initialize the world
    
    debugTimer.reset();     
    
    vbo1.vboInit();
    texture1.loadTexture("images/wood.png");
   
    drawWorldBenchmark->startBenchmark();

    //testSounds->playSounds("sounds/level_transition.mp3");

    return true;
}

void _scene::reSize(GLint width, GLint height)
{
    this->width = width;
    this->height = height;
    fpsText->setScreenDimensions(width,height);
    posText->setScreenDimensions(width,height);
    chunkText->setScreenDimensions(width,height);
    mouseScreenText->setScreenDimensions(width,height);
    mouseWorldText->setScreenDimensions(width,height);
    testText->setScreenDimensions(width,height);
    Logger.LogInfo("Resizing window to width: " + std::to_string(width) + " and height: " + std::to_string(height), LOG_BOTH);
    GLfloat aspectRatio = (GLfloat) width/ (GLfloat) height; //Intended to keep track of window resize
    glViewport(0,0,width,height); // Integer values taken in to take in view. Setting Viewport
    glMatrixMode(GL_PROJECTION); // Turns the projection into a matrix. Initiate the projection
    glLoadIdentity(); // Keep value's axes (matrix * identity matrix = matrix). Initialize the matrix with identity matrix
    
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

    //myQuad->drawQuad();

    drawWorldBenchmark->startBenchmark();
        myWorld->drawWorld(left,right,top,bottom); // Draw the world
    drawWorldBenchmark->clickBenchmark();

    testPlayer->drawUnit();
    //testUnit->drawSprite();

    chunkText->drawText();
    posText->drawText();
    fpsText->drawText();
    mouseScreenText->drawText();
    mouseWorldText->drawText();
    testText->drawText();

    // For FPS measuring
    frameCount++;

    if(fpsTimer->getMilliseconds() > fpsPrintInterval) {
        debugPrintFPS(); 
    }
}

// Runs in loop 60 times per second. dt is in ms.
void _scene::updateScene(double dt)
{
    // Update mouse 
    float mouseScreenYInverted = height - mouseScreenPos.y;
    // Bottom-left is (0,0) and top right is (1,1)
    mouseNormalPos = {mouseScreenPos.x/width,mouseScreenYInverted/height};

    //cout << "Mouse Normal Pos: " << mouseNormalPos.toString() << "\n";

    float worldWidth = right-left;
    float worldHeight = top-bottom;

    // Where the mouse is in world position (for selecting tiles etc)
    mouseWorldPos = {mouseNormalPos.x * worldWidth + left, mouseNormalPos.y * worldHeight + bottom};
    
    bool walking = false;

    /**
     * 0 0 0
     * 0 P 0
     * 0 0 0
     * 
     * P = player
     * 0 = tile (0 = no collision and 1 = collision)
     */
    bool collisionTable[9] = {false};
    for (int i = 0; i < 9; i++) {
        // Establish tiles right around player
        int x = i % 3 - 1; 
        int y = i / 3 - 1;
        Vec2f offset(16 * x, -16 * y);
        collisionTable[i] = myWorld->getTileAtWorld(testPlayer->pos + offset)->hasCollision;
    }

    // Debugs collision table
    // cout << "Collision Table:" <<
    //     "\n(" << collisionTable[0] << ", " << collisionTable[1] << ", " << collisionTable[2] << ")\n" <<
    //     "\n(" << collisionTable[3] << ", " << collisionTable[4] << ", " << collisionTable[5] << ")\n" <<
    //     "\n(" << collisionTable[6] << ", " << collisionTable[7] << ", " << collisionTable[8] << ")\n";

    if (!cameraFree) {
        // Double input -- diagonol checks //
        if (W && A) {
            testPlayer->getSprite("WALK")->loadSpriteAction("WALK_UP_LEFT");
            testPlayer->getSprite("WALK")->setIdleFrame(0,2);
            walking = true;
        }
        
        if (W && D) {
            testPlayer->getSprite("WALK")->loadSpriteAction("WALK_UP_RIGHT");
            testPlayer->getSprite("WALK")->setIdleFrame(0,4);
            walking = true;
        }
        
        if (S && A) {
            testPlayer->getSprite("WALK")->loadSpriteAction("WALK_DOWN_LEFT");
            testPlayer->getSprite("WALK")->setIdleFrame(0,1);
            walking = true;
        }
    
        if (S && D) {
            testPlayer->getSprite("WALK")->loadSpriteAction("WALK_DOWN_RIGHT");
            testPlayer->getSprite("WALK")->setIdleFrame(0,5);
            walking = true;
        }

        // Prevents single inputs when were already doing a double input
        if (!walking) {
            // Sigle input checks //
            if (W && !collisionTable[1]) {
                testPlayer->getSprite("WALK")->loadSpriteAction("WALK_UP");
                testPlayer->getSprite("WALK")->setIdleFrame(0,3);
                walking = true;
            }
            
            if (A && !collisionTable[3]) {
                testPlayer->getSprite("WALK")->loadSpriteAction("WALK_DOWN_LEFT");
                testPlayer->getSprite("WALK")->setIdleFrame(0,1);
                walking = true;
            }
            
            if (S && !collisionTable[7]) {
                testPlayer->getSprite("WALK")->loadSpriteAction("WALK_DOWN");
                testPlayer->getSprite("WALK")->setIdleFrame(0,0);
                walking = true;
            }
        
            if (D && !collisionTable[5]) {
                testPlayer->getSprite("WALK")->loadSpriteAction("WALK_DOWN_RIGHT");
                testPlayer->getSprite("WALK")->setIdleFrame(0,5);
                walking = true;
            }
        }
    }

    if (walking) {
        testPlayer->getSprite("WALK")->startAnimation();
    } else {
        testPlayer->getSprite("WALK")->stopAnimation();
    }

    dt = dt / 1000.0; // Convert dt to seconds for easier calculations
    if (cameraFree) {
        if(W) cameraY += cameraSpeed*dt;
        if(A) cameraX -= cameraSpeed*dt; 
        if(S) cameraY -= cameraSpeed*dt;
        if(D) cameraX += cameraSpeed*dt; 
    } else {
        if(W && !collisionTable[1]) testPlayer->pos.y += playerSpeed*dt;
        if(A && !collisionTable[3]) testPlayer->pos.x -= playerSpeed*dt; 
        if(S && !collisionTable[7]) testPlayer->pos.y -= playerSpeed*dt;
        if(D && !collisionTable[5]) testPlayer->pos.x += playerSpeed*dt;
    }

    // // Chunk generation only works during world setup (for now TODO)
    // bool inLoadedChunk = myWorld->isChunkLoaded(playerChunkPos.x, playerChunkPos.y);
    // if (!inLoadedChunk) {
    //     //myWorld->generateChunk(playerChunkPos.x, playerChunkPos.y);
    // }

    if (debugTimer.getMilliseconds() > debugPrintInterval) 
    {
        debugPrint(); 
        debugTimer.reset(); 
    }

    playerChunkPos = myWorld->worldToChunkPos(testPlayer->pos);

    fpsText->text = "FPS: " + to_string(sceneFPS);
    posText->text = "Player Position: (" + to_string(testPlayer->pos.x) + ", " + to_string(testPlayer->pos.y) + ")"; 
    chunkText->text = "Chunk Position: (" + to_string(playerChunkPos.x) + ", " + to_string(playerChunkPos.y) + ")"; 
    mouseScreenText->text = "Mouse Screen Position: " + mouseScreenPos.toString(); 
    mouseWorldText->text = "Mouse World Position: " + mouseWorldPos.toString(); 
    //const _tile* tile = myWorld->getTileAtWorld(testPlayer->pos);

    const _tile* tile = myWorld->getTileAtWorld(Vec2f(mouseWorldPos.x,mouseWorldPos.y));
    _cell* cell = myWorld->getCellAtWorld(Vec2f(mouseWorldPos.x,mouseWorldPos.y));
    _chunk* chunk = myWorld->getChunkAtWorld(Vec2f(mouseWorldPos.x,mouseWorldPos.y));

    if (cell != hoveredCell) {
        if (hoveredCell && hoveredChunk) {
            hoveredCell->outlined = false;
            hoveredChunk->vboDirty = true;
        }

        hoveredCell = cell;
        hoveredChunk = chunk;
        
        hoveredCell->outlined = true;
        hoveredChunk->vboDirty = true;
    }

    if (tile) {
        //testText->text = "Tile Collision: " + to_string(tile->hasCollision);
        testText->text = "Tile Name: " + tile->name + " -- Tile Collision: " + (tile->hasCollision ? "TRUE" : "FALSE");
    } else {
        testText->text = "Tile Name: nullptr";
    }
}

void _scene::debugPrint()
{
    Logger.LogDebug("Debug Print: W=" + to_string(W) + " A=" + to_string(A) + " S=" + to_string(S) + " D=" + to_string(D), LOG_CONSOLE);
    Logger.LogDebug("Camera Position: (" + to_string(cameraX) + ", " + to_string(cameraY) + ") Zoom: " + to_string(cameraZoom), LOG_CONSOLE);
    Logger.LogDebug("Player Position: (" + to_string(testPlayer->pos.x) + ", " + to_string(testPlayer->pos.y) + ")", LOG_CONSOLE);
    Logger.LogDebug("Player is in chunk: (" + to_string(playerChunkPos.x) + ", " + to_string(playerChunkPos.y) + ")", LOG_CONSOLE);
    
    Logger.LogDebug("World drawing took: " + to_string(drawWorldBenchmark->getAverageResult()) + "ms");

    Logger.LogDebug("Distance to test unit: " + to_string(GetDistance(testPlayer->pos,testUnit->pos)));

    bool inLoadedChunk = myWorld->isChunkLoaded(playerChunkPos.x, playerChunkPos.y);
    //Logger.LogDebug("Player is in loaded chunk: " + (inLoadedChunk ? "YES" : "NO"), LOG_CONSOLE);

    Logger.LogDebug("Left: " + to_string(left) + "\n Right: " + to_string(right) + "\n Top: " + to_string(top) + "\n Bottom: " + to_string(bottom));

    myWorld->debugPrint();
}

void _scene::debugPrintFPS() {
    sceneFPS = frameCount / (fpsTimer->getMilliseconds() / 1000.0); // Calculate FPS based on frames and time
    //Logger.LogInfo("Current FPS: " + std::to_string(sceneFPS), LOG_CONSOLE);
    frameCount = 0; // Reset frame count after printing FPS
    fpsTimer->reset(); // Reset the timer for the next FPS calculation
}

void _scene::keyboardHandler(WPARAM wParam)
{
    if (inputTimer.getMilliseconds() > 200) // 200 ms debounce time for toggle keys
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
                myWorld->DEBUG_displayChunkBorders = !myWorld->DEBUG_displayChunkBorders;
                Logger.LogInfo("Toggled chunk border display: " + std::string(myWorld->DEBUG_displayChunkBorders ? "ON" : "OFF"), LOG_CONSOLE);
                break;
        }
        inputTimer.reset(); // Reset the timer after handling a toggle key
    }
}

void _scene::mouseMove(LPARAM lParam) {
    mouseScreenPos = {LOWORD(lParam), HIWORD(lParam)};
    if (inputDebugEnabled) Logger.LogDebug("Mouse Move at " + mouseScreenPos.toString("px"), LOG_CONSOLE); //Log the position of the mouse when it moves
    // Adjusted to be -width to +width and +height to -height plus offset of the camera
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
                // WASD //
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
                // OTHER //
                case 32: // spacebar
                    SPACE = true;
                    break;
            }
            break;
        // Key release
        case WM_KEYUP:
            if (inputDebugEnabled) Logger.LogDebug("Key Released: " + std::to_string(wParam), LOG_CONSOLE); //Log the key that was released
            switch (wParam)
            {
                // WASD //
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
                // OTHER //
                case 32: // spacebar
                    SPACE = false;
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
            mouseMove(lParam);
            break;
        // Mouse wheel
        case WM_MOUSEWHEEL:
            if (inputDebugEnabled) Logger.LogDebug("Mouse Wheel: " + std::to_string((short)HIWORD(wParam)) + " at (" + std::to_string(LOWORD(lParam)) + ", " + std::to_string(HIWORD(lParam)) + ")", LOG_CONSOLE); //Log the amount of scroll and position of the mouse when the wheel is scrolled
            if ((short)HIWORD(wParam) > 0) { 
                // Scroll up
                if (cameraZoom < 9.0f) cameraZoom++; // Zoom in by increasing the zoom factor
            } else if ((short)HIWORD(wParam) < 0) {
                // Scroll down
                if (cameraZoom > 1.0f) cameraZoom--; // Zoom out by decreasing the zoom factor
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
    if (!cameraFree) {
        // If camera is not free, it will track the player (centered on player)
        cameraX = testPlayer->pos.x;
        cameraY = testPlayer->pos.y;
    }

    float renderCameraX = floor(cameraX);
    float renderCameraY = floor(cameraY);
    
    float halfWidth = width * 0.5f / cameraZoom; // Adjust half-width based on zoom level
    float halfHeight = height * 0.5f / cameraZoom; // Adjust half-height based on zoom level

    left = renderCameraX - halfWidth;
    right = renderCameraX + halfWidth;
    bottom = renderCameraY - halfHeight;
    top = renderCameraY + halfHeight;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // We use Orthographic projection because its a 2D game and it makes it easy to maintain scale via pixels
    /**
     * This 
     */
    glOrtho(
        left, right,    // left->right = zoom of 1 means 1 world unit (1 pixel) is visible. Zooming in makes the world bigger and zooming out makes the world smaller
        bottom, top,    // top->bottom = zoom of 1 means 1 world unit (1 pixel) is visible. Zooming in makes the world bigger and zooming out makes the world smaller
        -1.0, 1.0       // z index from -1 to 1 is visible (everything else clips and is not visible)
    ); 
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

