#ifndef _SCENE_H
#define _SCENE_H

// INCLUDES //

#include<_common.h> 

#include<_texture.h>
#include<_lightSettings.h>
#include<_model.h>
#include<_timerPlusPlus.h>  
#include<_sprite.h>
#include<_vbo.h>

#include<_world.h>

class _scene
{
    public:

        _scene();
        virtual ~_scene();

        // Initialization of the scene
        GLint initGL(); 

        // Runs and handles a window resize event
        void reSize(GLint width, GLint height);
        // Draws the scene using a double-buffer. Runs as fast as the loop will let it. No time-based events should ever be added in here. Those are for the updateScene()
        void drawScene(); 
        // Updates the scene based on time of (60fps ~16.67ms per update). Time-based events should be added in here, such as movement and other time-based changes to the scene
        void updateScene(double dt);
        // Handles input messsages send from windows -- used for controls etc
        int winMsg(HWND	hWnd, UINT uMsg, WPARAM	wParam, LPARAM lParam);

        // CLASSES //
    protected:
    private:

        int width, height; 

        _lightSettings *myLight = new _lightSettings();     
        _model* myModel = new _model();
        _world* myWorld = new _world();
        _sprite* test_player = new _sprite();

        _vbo vbo1 = _vbo();

        _texture texture1;

        _timerPlusPlus inputTimer; // Timer to regulate toggle keys (ensures a key only pressed once)

        float playerX = 0.0f;
        float playerY = 0.0f;

        float cameraX = 0.0f;
        float cameraY = 0.0f;
        float cameraSpeed = 128.0f; // Units per second
        float cameraZoom = 1.0f;

        bool W = false; 
        bool A = false; 
        bool S = false; 
        bool D = false;

        // DEBUGGING //
        _timerPlusPlus debugTimer;          // Timer to track time between updates for the scene
        _timerPlusPlus* fpsTimer = new _timerPlusPlus(); // Timer to track time between frames for FPS calculation

        bool isPerspective = false;
        bool debugEnabled = true;           // Enables specific debugging information for the scene
        bool inputDebugEnabled = false;     // Enables debug info for inputs (keyboard keys + mouse inputs)
        bool cameraFree = false;            // If true camera will not track player and can be moved freely

        double debugPrintInterval = 5000;   // ms between debug prints
        double fpsPrintInterval = 1000;     // ms between FPS prints
        double sceneFPS = 0.0;              // Current FPS of the scene (updated every [fpsPrintInterval] ms)
        int frameCount = 0;                 // Number of frames between FPS prints

        // Function that runs every [debugPrintInterval] ms for dubuggin purposes
        void debugPrint();
        // Function that runs every [fpsPrintInterval] ms to calculate and print the FPS of the scene
        void debugPrintFPS();
        // Handles keyboard inputs for toggle keys. Function regulated by a timer
        void keyboardHandler(WPARAM wParam); 
        // Handles commands sent from console
        void commandHandler();
        // Applies camera position zoom, etc
        void applyCamera();
};

#endif // _SCENE_H
