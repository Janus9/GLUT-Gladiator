#ifndef _INPUTS_H
#define _INPUTS_H
#include <_common.h>

class _inputs
{
    public:
        _inputs();
        virtual ~_inputs();

        void keyPressed(); //tracks to see if the key was pressed
        void keyUp(); //tracks to see if the key has been released (is the button currently being held)

        void mouseEventDown(double, double); //has the mouse key been pressed
        void mouseEvenUp(); //has the mouse key been released (is the mouse key currently being held)
        void mouseMove(double, double); //is the mouse currently moving
        void mouseWheel(double); //is there currently scrolling happening from the mouse

        double prevMx; //where was the mouse's previous x coordinate on the screen
        double prevMy; //where was the mouse's previous y coordinate on the screen

        /*
        - Using the mouse to rotate means a flag is needed to track when an object is being rotated
        - The mouse key is held to relate this rotation
        */

        bool isRotate; //Is the mouse currently rotating
        bool isTranslate; //How to translate the mouse to the object

        WPARAM wParam; //A parameter from the keyboard to take in the actual keystrokes

    protected:

    private:
};

#endif // _INPUTS_H
