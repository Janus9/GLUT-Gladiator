#include "_inputs.h"

_inputs::_inputs()
{
    //ctor
    isRotate = false; //Not currently rotating
    isTranslate = false; //Not currently translating
}

_inputs::~_inputs()
{
    //dtor
}

void _inputs::keyPressed()
{
    switch(wParam)//switch case to check different key inputs
    {
        case VK_LEFT:
        //left key
        break;
        case VK_RIGHT:
        //right key
        break;
        case VK_UP:
        //up key
        break;
        case VK_DOWN:
        //left key
        break;
    }
}

void _inputs::keyUp()
{
    switch(wParam)
    {
        default: break; //The default key is currently at break
    }
}

void _inputs::mouseEventDown(double x, double y)
{
    prevMx = x; //The double value for x is taken in and stored as that value
    prevMy = y; //The double value for y is taken in and stored as that value

    switch(wParam)
    {
    case MK_LBUTTON:
        isRotate = true; //the Left mouse button being held means the object will rotate
        break;
    case MK_RBUTTON:
        isTranslate = true; //the right mouse button being held means the object will translate
        break;
    case MK_MBUTTON: //the middle button can be set to a value
        break;
    default: //The default is currently set to nothing yet can also be set
        break;

    }
}

void _inputs::mouseEvenUp()
{
    isRotate = false; //The mouse will stop rotating the object
    isTranslate = false; //The mouse will stop translating the object
}

void _inputs::mouseMove(double, double)
{
    if(isRotate)
    {

    }
    if(isTranslate)
    {

    }
}

void _inputs::mouseWheel(double delta)
{

}

