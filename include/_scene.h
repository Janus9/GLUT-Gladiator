#ifndef _SCENE_H
#define _SCENE_H

// INCLUDES //
#include <windows.h>

#include<_common.h> 

#include<_texture.h>
#include<_lightSettings.h>
#include<_model.h>
#include<_timerPlusPlus.h>  
#include<_sprite.h>
#include<_vbo.h>
#include<_quad.h>
#include<_benchmark.h>
#include<_unit.h>
#include<_sounds.h>
#include<_world.h>
#include<_hud.h>
#include<_particleManager.h>
#include<_bulletManager.h>
#include<_shader.h>
#include<_player.h>
#include<_enemyManager.h>

class _scene
{
    public:

        _scene();
        virtual ~_scene();

        // Initialization of the scene openGL content (runs every time scene is reopened from menu)
        GLint initGL(); 

        // Initialization of scene objects
        void initScene();

        // Runs and handles a window resize event
        void reSize(GLint width, GLint height);
        // Draws the scene using a double-buffer. Runs as fast as the loop will let it. No time-based events should ever be added in here. Those are for the updateScene()
        void drawScene(); 
        // Updates the scene based on time of (60fps ~16.67ms per update). Time-based events should be added in here, such as movement and other time-based changes to the scene
        void updateScene(double dt, bool* keysArray);
        // Handles input messsages send from windows -- used for controls etc
        int winMsg(HWND	hWnd, UINT uMsg, WPARAM	wParam, LPARAM lParam);

        // CLASSES //
    protected:
    private:
        // Window dimensions
        int width, height; 
        
        // (x,y) position of the mouse in world coordinates (adjusted to camera position)
        Vec2f mouseWorldPos;
        // (x,y) position of the mouse in screen coordinates
        Vec2f mouseScreenPos;
        // (x,y) position of the mouse normalized for screen coordinates (0-1)
        Vec2f mouseNormalPos;

        // Coordinates for the view-window. These are set in applyCamera.
        float left = 0.0f;
        float right = 0.0f;
        float bottom = 0.0f;
        float top = 0.0f;

        // Player
        unique_ptr<_player> player = make_unique<_player>();                        
        player_action action = PLAYER_ACTION_NULL;
        player_face face = PLAYER_FACE_NULL;

        // Enemies
        unique_ptr<_enemyManager> enemyManager = make_unique<_enemyManager>();
        
        // Bullets (Projectiles)
        unique_ptr<_bulletManager> bulletManager = make_unique<_bulletManager>();
        _bullet_config player_bullet;
        _bullet_config turret_bullet;

        _sounds* testSounds = new _sounds();

        _lightSettings *myLight = new _lightSettings();     
        _model* myModel = new _model();
        _world* myWorld = new _world();
        

        _unit* testUnit = new _unit();

        _quad* myQuad = new _quad();

        _vbo vbo1 = _vbo();

        _hud* hud = new _hud();

        _texture texture1;

        _timerPlusPlus inputTimer; // Timer to regulate toggle keys (ensures a key only pressed once)
        _timerPlusPlus* interactionTimer = new _timerPlusPlus(); // Timer for interactive events
        _timerPlusPlus fireRateTimer;

        _shader* sh = new _shader();

        _cell* hoveredCell = nullptr;
        _chunk* hoveredChunk = nullptr;

        // Cell + Chunk player is occupying
        _cell* playerCell = nullptr;
        _chunk* playerChunk = nullptr;

        // Position of the player in chunk coordinates
        Vec2i playerChunkPos = {0, 0};

        float cameraX = 0.0f;
        float cameraY = 0.0f;
        float playerSpeed = 128.0f; // Units per second
        float cameraSpeed = 1024.0f; // Units per second
        float cameraZoom = 1.0f;

        float miningSpeed = 2.5f; // In seconds

        bool *keysPtr = nullptr;    // Points to bool array of keys passed from main

        bool LMB = false;

        // -- FUNCTIONS -- //
        void mouseMove(LPARAM lParam);
        // Function that runs every [debugPrintInterval] ms for dubuggin purposes
        void debugPrint();
        // Function that runs every [fpsPrintInterval] ms to calculate and print the FPS of the scene
        void debugPrintFPS();
        // Handles keyboard inputs for toggle keys. Function regulated by a timer
        void keyboardHandler(WPARAM wParam); 
        // Applies camera position zoom, etc
        void applyCamera();

        // -- DEBUGGING -- //
        _timerPlusPlus debugTimer;          // Timer to track time between updates for the scene
        _timerPlusPlus* fpsTimer = new _timerPlusPlus(); // Timer to track time between frames for FPS calculation

        _benchmark* drawWorldBenchmark = new _benchmark();

        bool debugEnabled = true;           // Enables specific debugging information for the scene
        bool inputDebugEnabled = false;     // Enables debug info for inputs (keyboard keys + mouse inputs)
        bool cameraFree = false;            // If true camera will not track player and can be moved freely

        double debugPrintInterval = 5000;   // ms between debug prints
        double fpsPrintInterval = 1000;     // ms between FPS prints
        double sceneFPS = 0.0;              // Current FPS of the scene (updated every [fpsPrintInterval] ms)
        int frameCount = 0;                 // Number of frames between FPS prints
};

#endif // _SCENE_H
