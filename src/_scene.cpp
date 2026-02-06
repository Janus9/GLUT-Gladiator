#include "_scene.h"

_scene::_scene()
{
    //ctor
}

_scene::~_scene()
{
    //dtor
}


GLint _scene::initGL()
{
    glClearColor(0.0,0.0,0.0,1.0); //Intended to change the background color. 0001 is black
    glClearDepth(1.0); //Gives depth to the environment by having color both in the front and back. Depth-test value
    glEnable(GL_DEPTH_TEST); //Will ensure the depth of the z-coordinate is accurate through enabling depth-test
    glDepthFunc(GL_LEQUAL); //Depth-test is less than or equal to--> true for less or equal

    glEnable(GL_LIGHTING); //Enable lighting within the scene to be initialized (8 total lights)
    glEnable(GL_LIGHT0); //Light is instantiated at this module
    glEnable(GL_COLOR_MATERIAL); //To theoretically instantiate a base color on a 3D object (likely won't be used)

    myLight->setLight(GL_LIGHT0); //The light onto the object from the pointer is set to be the instantiated light from before
    return true;

}

//Forced perspective projection: Farther away objects look smaller
//Objects needs to have perspective to construct how the objects actually look (size of object has to match the specific scale with which the window is in)
//The model has to be in the right position in the scene to match the model functionality
//A camera needs to be placed to center the camera and establish view properties to apply projection

//Aspect ratio for window has to match the specific resize being managed
//Width and Height of window need to be considered to match the specific width and height of the window being generated
//The window size can be changed and the game's structural integrity is the same

void _scene::reSize(GLint width, GLint height)
{
    GLfloat aspectRatio = (GLfloat) width/ (GLfloat) height; //Intended to keep track of window resize
    glViewport(0,0,width,height); //Integer values taken in to take in view. Setting Viewport
    glMatrixMode(GL_PROJECTION); //Turns the projection into a matrix. Initiate the projection
    glLoadIdentity(); //Keep value's axes (matrix * identity matrix = matrix). Initialize the matrix with identity matrix

    gluPerspective(45.0, aspectRatio, 0.1, 100.0); //How far and how near do you want the perspective to be. Setup prospective projection
    glMatrixMode(GL_MODELVIEW); //Inputs into matrix depend on the placement of objects in the matrix overview. Initiate model and view matrix

    glLoadIdentity();//Keep value's axes (matrix * identity matrix = matrix). Initialize the matrix with identity matrix

}

//Scene is running in a loop
void _scene::drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    //clear buffers
    glLoadIdentity(); //Whichever state the scene is in it will stay there
    glColor3f(1.0,0.5,1.9); //sets the color of the model
    glTranslatef(0.0,0.0,-8.0); //Translating the specific position of the object that will be drawn. -Z means into the scene (center and back position)
    glutSolidTeapot(1.5); //Places a teapot model within the scene. draw teapot

}


