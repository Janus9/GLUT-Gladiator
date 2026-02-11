#ifndef _SCENE_H
#define _SCENE_H

// INCLUDES //

#include<_common.h> 

#include<_lightSettings.h>
#include<_model.h>
#include<_timerPlusPlus.h>  
#include<_sprite.h>
#include<_vbo.h>

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
        _lightSettings *myLight = new _lightSettings();     // New Instance of Lights needed to be used in the scene
        _model* myModel = new _model();                     // New Instance of Model needed to be used in the scene


    protected:
    private:
        bool W,A,S,D; // Track WASD key states (true = help, false = released)
        
        _timerPlusPlus debugTimer; // Timer to track time between updates for the scene
        bool debugEnabled = true; // Enables specific debugging information for the scene
        double debugPrintInterval = 5000; // ms between debug prints
        // Function that runs every [debugPrintInterval] ms for dubuggin purposes
        void debugPrint();
};

#endif // _SCENE_H
