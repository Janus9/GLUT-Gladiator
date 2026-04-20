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

    delete fpsTimer;
    fpsTimer = nullptr;

    delete interactionTimer;
    interactionTimer = nullptr;    

    delete drawWorldBenchmark;
    drawWorldBenchmark = nullptr;

    delete testSounds;
    testSounds = nullptr;

    delete testUnit;
    testUnit = nullptr;

    delete hud;
    hud = nullptr;

    delete sh;
    sh = nullptr;
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

    // -- CLASS INIT -- //

    inputTimer.reset(); 
    fpsTimer->reset();
    interactionTimer->reset();  
    fireRateTimer.reset();

    player->initPlayer();
    player->hasGun = true;

    // -- HUD -- //

    hud->addHudText("FPS");
    hud->getHudText("FPS")->setFont(GLUT_BITMAP_9_BY_15);
    
    hud->addHudText("PLAYER_POS");
    hud->getHudText("PLAYER_POS")->setFont(GLUT_BITMAP_9_BY_15);

    hud->addHudText("CHUNK_POS");
    hud->getHudText("CHUNK_POS")->setFont(GLUT_BITMAP_9_BY_15);

    hud->addHudText("MOUSE_SCREEN");
    hud->getHudText("MOUSE_SCREEN")->setFont(GLUT_BITMAP_9_BY_15);

    hud->addHudText("MOUSE_WORLD");
    hud->getHudText("MOUSE_WORLD")->setFont(GLUT_BITMAP_9_BY_15);

    hud->addHudText("MOUSE_NORMAL");
    hud->getHudText("MOUSE_NORMAL")->setFont(GLUT_BITMAP_9_BY_15);

    hud->addHudText("TILE_NAME");
    hud->getHudText("TILE_NAME")->setFont(GLUT_BITMAP_9_BY_15);

    hud->addHudText("CELL_HEALTH");
    hud->getHudText("CELL_HEALTH")->setFont(GLUT_BITMAP_9_BY_15);

    hud->addHudText("CHUNK_REDRAW");
    hud->getHudText("CHUNK_REDRAW")->setFont(GLUT_BITMAP_9_BY_15);

    hud->addHudText("PLAYER_HEALTH");
    hud->getHudText("PLAYER_HEALTH")->setFont(GLUT_BITMAP_HELVETICA_18);

    hud->addHudSprite("PROGRESS_BAR");
    hud->getHudSprite("PROGRESS_BAR")->getSprite()->initSprite("images/progress_bar.png",5,1,sprite_direction::RIGHT);
    hud->getHudSprite("PROGRESS_BAR")->getSprite()->scale = {1.5f, 1.5f};
    hud->getHudSprite("PROGRESS_BAR")->getSprite()->createSpriteAction(sprite_action("DEFAULT",0,0,4));
    hud->getHudSprite("PROGRESS_BAR")->getSprite()->loadSpriteAction("DEFAULT");
    hud->getHudSprite("PROGRESS_BAR")->getSprite()->setIdleFrame(0,0);
    
    myLight->setLight(GL_LIGHT0); // The light onto the object from the pointer is set to be the instantiated light from before
    myWorld->initWorld(); // Initialize the world
    
    debugTimer.reset();     
   
    drawWorldBenchmark->startBenchmark();

    enemyManager->initEnemyManager(player.get(),myWorld,bulletManager.get(),&turret_bullet);

    //testSounds->playSounds("sounds/level_transition.mp3");

    // -- SHADERS -- //
    // sh->initShader("shaders/V.vs","shaders/F.fs");
    // glUseProgram(sh->program);

    bulletManager->initBulletManager("images/test_bullet.png", myWorld,player.get(),enemyManager.get());
    player_bullet.amount = 1;
    player_bullet.speed = 512.0f;
    player_bullet.width = 15.0f;
    player_bullet.height = 3.0f;
    player_bullet.lifespan = 3.0f;
    player_bullet.angleOffset = 2.0f;

    turret_bullet.amount = 1;
    turret_bullet.speed = 400.0f;
    turret_bullet.width = 20.0f;
    turret_bullet.height = 4.0f;
    turret_bullet.lifespan = 3.0f;
    turret_bullet.angleOffset = 7.0f;

    player->setHealth(1000.0f);

    return true;
}

void _scene::reSize(GLint width, GLint height)
{
    this->width = width;
    this->height = height;

    // These should be moved to main at some point
    _hud::setHudViewportDimensions(width,height);

    float offset = 40.0f;
    float spacing = 20.0f;

    if (hud->getHudText("FPS")) hud->getHudText("FPS")->position = {20.0f, height-offset};
    if (hud->getHudText("FPS")) offset += spacing;
    if (hud->getHudText("PLAYER_POS")) hud->getHudText("PLAYER_POS")->position = {20.0f, height-offset};
    if (hud->getHudText("PLAYER_POS")) offset += spacing;
    if (hud->getHudText("CHUNK_POS")) hud->getHudText("CHUNK_POS")->position = {20.0f, height-offset};
    if (hud->getHudText("CHUNK_POS")) offset += spacing;
    if (hud->getHudText("MOUSE_SCREEN")) hud->getHudText("MOUSE_SCREEN")->position = {20.0f, height-offset};
    if (hud->getHudText("MOUSE_SCREEN")) offset += spacing;
    if (hud->getHudText("MOUSE_WORLD")) hud->getHudText("MOUSE_WORLD")->position = {20.0f, height-offset};
    if (hud->getHudText("MOUSE_WORLD")) offset += spacing;
    if (hud->getHudText("MOUSE_NORMAL")) hud->getHudText("MOUSE_NORMAL")->position = {20.0f, height-offset};
    if (hud->getHudText("MOUSE_NORMAL")) offset += spacing;
    if (hud->getHudText("TILE_NAME")) hud->getHudText("TILE_NAME")->position = {20.0f, height-offset};
    if (hud->getHudText("TILE_NAME")) offset += spacing;
    if (hud->getHudText("CELL_HEALTH")) hud->getHudText("CELL_HEALTH")->position = {20.0f, height-offset};
    if (hud->getHudText("CELL_HEALTH")) offset += spacing;
    if (hud->getHudText("CHUNK_REDRAW")) hud->getHudText("CHUNK_REDRAW")->position = {20.0f, height-offset};
    if (hud->getHudText("CHUNK_REDRAW")) offset += spacing;
    if (hud->getHudText("PLAYER_HEALTH")) hud->getHudText("PLAYER_HEALTH")->position = {20.0f, 0 + 40.0f};
    if (hud->getHudText("PLAYER_HEALTH")) offset += spacing;
    if (hud->getHudSprite("PROGRESS_BAR")) hud->getHudSprite("PROGRESS_BAR")->position = {width/2.0, 100.0f};

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

    bulletManager->drawBulletManager();

    enemyManager->drawEnemies();
    
    player->drawPlayer();

    hud->drawHud();

    // For FPS measuring
    frameCount++;

    if(fpsTimer->getMilliseconds() > fpsPrintInterval) {
        debugPrintFPS(); 
    }

}

// Runs in loop 60 times per second. dt is in ms.
void _scene::updateScene(double dt)
{
    dt = dt / 1000.0; // Convert dt to seconds for easier calculations

    enemyManager->updateEnemies(dt);
    bulletManager->updateBulletManager(dt);
    myWorld->updateWorld(dt);
    player->updatePlayer(dt);

    // Check for mouse events
    if (LMB && hoveredCell && hoveredChunk && myWorld->isCellWall(hoveredCell)) {
        if (interactionTimer->getSeconds() > miningSpeed/5.0f) {
            hud->getHudSprite("PROGRESS_BAR")->getSprite()->iterateFrame();
            myWorld->damageCell(hoveredCell,25.0f);
            if (!hoveredCell->isAlive()) {
                cout << "BLOCK HAS BEEN MINED \n";
                hud->getHudSprite("PROGRESS_BAR")->getSprite()->stopAnimation(); // Mining finished, reset progress bar animation
            }
            interactionTimer->reset();
        }
    } else {
        interactionTimer->reset();
    }

    // Update mouse 
    float mouseScreenYInverted = height - mouseScreenPos.y;
    // Bottom-left is (-1,-1) and top right is (1,1)
    mouseNormalPos = {mouseScreenPos.x/(width * 0.5f) - 1,mouseScreenYInverted/(height * 0.5f) - 1};

    //cout << "Mouse Normal Pos: " << mouseNormalPos.toString() << "\n";

    float worldWidth = right-left;
    float worldHeight = top-bottom;

    // Where the mouse is in world position (for selecting tiles etc)
    mouseWorldPos = {mouseScreenPos.x/width * worldWidth + left, mouseScreenYInverted/height * worldHeight + bottom};
    
    _cell* _playerCell = myWorld->getCellAtWorld(player->pos);
    _chunk* _playerChunk = myWorld->getChunkAtWorld(player->pos);

    // if (_playerCell != playerCell) {
    //     if (playerCell && playerChunk) {
    //         playerCell->outlined = false;
    //         playerChunk->vboDirty = true;
    //     }

    //     playerCell = _playerCell;
    //     playerChunk = _playerChunk;
        
    //     playerCell->outlined = true;
    //     playerChunk->vboDirty = true;
    // }

    playerCell = _playerCell;
    playerChunk = _playerChunk;

    /**
     * X X X
     * X P X
     * X X X
     * 
     * P = player
     * X = tile (false means tile has no collision while true mean tile has collision)
     * 
     */
    bool occupationTable[9] = {false};
    /**
     * Used for movement, we check from occupation table IF movement causes issues
     * 
     *   0
     * 1 P 2
     *   3
     * 
     */
    bool collisionTable[4] = {false};
    for (int i = 0; i < 9; i++) {
        // Establish tiles right around player
        int x = i % 3 - 1; 
        int y = i / 3 - 1;
        Vec2f offset(16 * x, -16 * y);
        occupationTable[i] = myWorld->getTileAtWorld(player->pos + offset)->hasCollision;
    }

    if (myWorld->getTileAtWorld(player->pos + Vec2f(0.0f, playerSpeed*dt))->hasCollision) {
        // Next potential movement yields use moving into upper tile
        collisionTable[0] = true;
    }
    if (myWorld->getTileAtWorld(player->pos - Vec2f(0.0f, playerSpeed*dt))->hasCollision) {
        // Next potential movement yields use moving into lower tile
        collisionTable[3] = true;
    }
    if (myWorld->getTileAtWorld(player->pos - Vec2f(playerSpeed*dt, 0.0f))->hasCollision) {
        // Next potential movement yields use moving into left tile
        collisionTable[1] = true;
    }
    if (myWorld->getTileAtWorld(player->pos + Vec2f(playerSpeed*dt, 0.0f))->hasCollision) {
        // Next potential movement yields use moving into right tile
        collisionTable[2] = true;
    }

    //cout << "Collision Table: " << collisionTable[0] << ", " << collisionTable[1] << ", " << collisionTable[2] << ", " << collisionTable[3] << "\n";

    bool W = keys['W'];
    bool A = keys['A'];
    bool S = keys['S'];
    bool D = keys['D'];
    bool SPACE = keys[VK_SPACE];

    if (SPACE) {
        player->isShooting = true;
    }  else {
        player->isShooting = false;
    }

    player->isMoving = false; 
    
    if (!cameraFree && !player->isDead()) {
        // Double input -- diagonol checks //
        if (W && A) {
            face = PLAYER_FACE_NW;
            player->isMoving = true;
        }
        
        if (W && D) {
            face = PLAYER_FACE_NE;
            player->isMoving = true;
        }
        
        if (S && A) {
            face = PLAYER_FACE_SW;
            player->isMoving = true;
        }
    
        if (S && D) {
            face = PLAYER_FACE_SE;
            player->isMoving = true;
        }

        // Prevents single inputs when were already doing a double input
        if (!player->isMoving) {
            // Sigle input checks //
            if (W && !collisionTable[0]) {
                face = PLAYER_FACE_N;
                player->isMoving = true;
            }
            
            if (A && !collisionTable[1]) {
                face = PLAYER_FACE_W;
                player->isMoving = true;
            }
            
            if (S && !collisionTable[3]) {
                face = PLAYER_FACE_S;
                player->isMoving = true;
            }
        
            if (D && !collisionTable[2]) {
                face = PLAYER_FACE_E;
                player->isMoving = true;
            }
        }
    }

    if (player->isMoving && !player->isDead()) {
        // Walking
        if (player->hasGun) {
            if (player->isShooting) {
                action = PLAYER_ACTION_WALK_SHOOT;
            } else {
                action = PLAYER_ACTION_WALK_GUN;
            }
        } else {
            action = PLAYER_ACTION_WALK;
        }
    } else if (!player->isDead()) {
        // Idle
        if (player->hasGun) {
            if (player->isShooting) {
                // Do face check //
                if (mouseNormalPos.y > 0.33f) {
                    // Face Up
                    if (mouseNormalPos.x > -1.0f && mouseNormalPos.x < -0.33f) {
                        // Top Left
                        face = PLAYER_FACE_NW;
                    } else if (mouseNormalPos.x > -0.33f && mouseNormalPos.x < 0.33f) {
                        // Up
                        face = PLAYER_FACE_N;
                    } else {
                        // Top Right
                        face = PLAYER_FACE_NE;
                    }
                } else if (mouseNormalPos.y > -0.33f) {
                    // Middle
                    if (mouseNormalPos.x > 0.0f) {
                        // Right
                        face = PLAYER_FACE_E;
                    } else {
                        // Left
                        face = PLAYER_FACE_W;
                    }
                } else {
                    // Face Down
                    if (mouseNormalPos.x > -1.0f && mouseNormalPos.x < -0.33f) {
                        // Bottom Left
                        face = PLAYER_FACE_SW;
                    } else if (mouseNormalPos.x > -0.33f && mouseNormalPos.x < 0.33f) {
                        // Up
                        face = PLAYER_FACE_S;
                    } else {
                        // Bottom Right
                        face = PLAYER_FACE_SE;
                    }
                }
                action = PLAYER_ACTION_IDLE_SHOOT;
            } else {
                action = PLAYER_ACTION_IDLE_GUN;
            }
        } else {
            action = PLAYER_ACTION_IDLE;
        }
    }

    if (SPACE && !player->isDead()) {
        if (fireRateTimer.getSeconds() > 1.0f/(player->fireRate/60.0f)) {
            player->setAnimationFPS(player->fireRate / 60);
            Vec2f offsetPos;
            if (face == PLAYER_FACE_E) {
                offsetPos = {4.0f,8.0f};
            } else {
                offsetPos = {-4.0f,8.0f};
            }
            bulletManager->spawnBulletEffect(player->pos + offsetPos, mouseWorldPos, _team::FRIENDLY, player_bullet);
            fireRateTimer.reset();
        } else {
            player->setAnimationFPS(12);
        }
    }

    if (player->isDead()) {
        player->handlePlayerDeath(face);
    }

    player->setAction(action,face);

    if (cameraFree) {
        if(W) cameraY += cameraSpeed*dt;
        if(A) cameraX -= cameraSpeed*dt; 
        if(S) cameraY -= cameraSpeed*dt;
        if(D) cameraX += cameraSpeed*dt; 
    } else if (!player->isDead()) {
        if(W && !collisionTable[0]) player->pos.y += playerSpeed*dt;
        if(A && !collisionTable[1]) player->pos.x -= playerSpeed*dt; 
        if(S && !collisionTable[3]) player->pos.y -= playerSpeed*dt;
        if(D && !collisionTable[2]) player->pos.x += playerSpeed*dt;
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

    playerChunkPos = myWorld->worldToChunkPos(player->pos);

    hud->getHudText("FPS")->setText("FPS: " + to_string(sceneFPS));
    hud->getHudText("PLAYER_POS")->setText("Position: " + player->pos.toString());
    hud->getHudText("CHUNK_POS")->setText("Chunk Position: " + playerChunkPos.toString());
    hud->getHudText("MOUSE_SCREEN")->setText("Mouse Screen Coords: " + mouseScreenPos.toString());
    hud->getHudText("MOUSE_WORLD")->setText("Mouse World Coords: " + mouseWorldPos.toString());
    hud->getHudText("MOUSE_NORMAL")->setText("Mouse Normal Coords: " + mouseNormalPos.toString());
    hud->getHudText("PLAYER_HEALTH")->setText("HP: " + to_string(player->health));
    //const _tile* tile = myWorld->getTileAtWorld(player->pos);

    const _tile* tile = myWorld->getTileAtWorld(Vec2f(mouseWorldPos.x,mouseWorldPos.y));
    _cell* cell = myWorld->getCellAtWorld(Vec2f(mouseWorldPos.x,mouseWorldPos.y));
    _chunk* chunk = myWorld->getChunkAtWorld(Vec2f(mouseWorldPos.x,mouseWorldPos.y));

    if (cell != hoveredCell) {
        hoveredCell->setOutline(false);

        hoveredCell = cell;
        hoveredChunk = chunk;

        hoveredCell->setOutline(true);
    }
    if (tile) {
        hud->getHudText("TILE_NAME")->setText("Selected Tile Name: " + tile->name);
    }
    if (hoveredCell) {
        hud->getHudText("CELL_HEALTH")->setText("Selected Cell Health: " + to_string(hoveredCell->getHealth()));
    }
    if (hoveredChunk) {
        string text_main = "Chunk Redraw: ";
        string test_con = hoveredChunk->vboDirty ? "TRUE" : "FALSE";
        hud->getHudText("CHUNK_REDRAW")->setText(text_main + test_con);
    }
}

void _scene::debugPrint()
{
    Logger.LogDebug("World drawing took: " + to_string(drawWorldBenchmark->getAverageResult()) + "ms");
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
                enemyManager->addEnemy(mouseWorldPos);
                break;
            case ' ': // SPACE
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
            keys[wParam] = true;
            keyboardHandler(wParam);
            break;
        // Key release
        case WM_KEYUP:
            if (inputDebugEnabled) Logger.LogDebug("Key Released: " + std::to_string(wParam), LOG_CONSOLE); //Log the key that was released
            keys[wParam] = false;
            break;
        // Left Mouse button down
        case WM_LBUTTONDOWN: {
            if (inputDebugEnabled) Logger.LogDebug("Left Mouse Button Down at (" + std::to_string(LOWORD(lParam)) + ", " + std::to_string(HIWORD(lParam)) + ")", LOG_CONSOLE); //Log the position of the mouse when left button is pressed
            LMB = true;
            interactionTimer->reset();
            break;
        }
        // Right Mouse button down
        case WM_RBUTTONDOWN:
            if (inputDebugEnabled) Logger.LogDebug("Right Mouse Button Down at (" + std::to_string(LOWORD(lParam)) + ", " + std::to_string(HIWORD(lParam)) + ")", LOG_CONSOLE); //Log the position of the mouse when right button is pressed
            break;
        // Left Mouse button up
        case WM_LBUTTONUP:
            if (inputDebugEnabled) Logger.LogDebug("Left Mouse Button Up at (" + std::to_string(LOWORD(lParam)) + ", " + std::to_string(HIWORD(lParam)) + ")", LOG_CONSOLE); //Log the position of the mouse when left button is released
            LMB = false;
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

void _scene::applyCamera() {
    if (!cameraFree) {
        // If camera is not free, it will track the player (centered on player)
        cameraX = player->pos.x;
        cameraY = player->pos.y;
    }
    
    float renderCameraX = floor(cameraX);
    float renderCameraY = floor(cameraY);
    
    float halfWidth = width * 0.5f / cameraZoom; // Adjust half-width based on zoom level
    float halfHeight = height * 0.5f / cameraZoom; // Adjust half-height based on zoom level

    left = renderCameraX - halfWidth;
    right = renderCameraX + halfWidth;
    bottom = renderCameraY - halfHeight;
    top = renderCameraY + halfHeight;

    _bulletManager::setViewportDimensions(left, right, top, bottom);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // This setup does not use a View matrix and it should -- this should be changed later

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

