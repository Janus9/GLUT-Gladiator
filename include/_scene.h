#ifndef _SCENE_H
#define _SCENE_H
#include <math.h>
#include <iostream>
#include<_common.h> //Intended to include all necessary header files
#include<_lightSettings.h>

//Scene will be used to initialize GL properties within scene
class _scene
{
    public:

        _scene();
        virtual ~_scene();

        GLint initGL(); //initialization of a GL function. initialize the openGL contents

        //This is to achieve function prototype
        //This is to handle resize the window
        void reSize(GLint, GLint); //Width and height window resize values.
        void drawScene(); //Render the final scene and draw it to the window

        _lightSettings *myLight = new _lightSettings(); //New Instance of Lights needed to be used in the scene


    protected:

    private:
};

#endif // _SCENE_H
