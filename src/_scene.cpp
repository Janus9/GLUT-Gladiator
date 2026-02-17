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
    glewInit();
    glClearColor(0.0,0.0,0.0,1.0); //Intended to change the background color. 0001 is black
    glClearDepth(1.0); //Gives depth to the environment by having color both in the front and back. Depth-test value
    glEnable(GL_DEPTH_TEST); //Will ensure the depth of the z-coordinate is accurate through enabling depth-test
    glDepthFunc(GL_LEQUAL); //Depth-test is less than or equal to--> true for less or equal

    glEnable(GL_LIGHTING); //Enable lighting within the scene to be initialized (8 total lights)
    glEnable(GL_LIGHT0); //Light is instantiated at this module
    glEnable(GL_COLOR_MATERIAL); //To theoretically instantiate a base color on a 3D object (likely won't be used)

    // CLASS INIT //

    inputTimer.reset(); // Reset the input timer to regulate toggle keys

    myLight->setLight(GL_LIGHT0); //The light onto the object from the pointer is set to be the instantiated light from before
    myModel->initModel(); //The model is initialized from the pointer to the model class
    myWorld->initWorld(); // Initialize the world
    
    debugTimer.reset(); //Reset the update timer for the scene
    vbo1.vboInit();
    texture1.loadTexture("images/wood.png");
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
    this->width = width;
    this->height = height;
    Logger.LogInfo("Resizing window to width: " + std::to_string(width) + " and height: " + std::to_string(height), LOG_BOTH);
    GLfloat aspectRatio = (GLfloat) width/ (GLfloat) height; //Intended to keep track of window resize
    glViewport(0,0,width,height); //Integer values taken in to take in view. Setting Viewport
    glMatrixMode(GL_PROJECTION); //Turns the projection into a matrix. Initiate the projection
    glLoadIdentity(); //Keep value's axes (matrix * identity matrix = matrix). Initialize the matrix with identity matrix
    
    if (isPerspective) {
        gluPerspective(45.0, aspectRatio, 0.1, 100.0); //How far and how near do you want the perspective to be. Setup prospective projection
    } else {
        //Orthographic projection -- objects are the same size regardless of distance from camera
        glOrtho(
            0.0, width,     // left->right = 1 world unit (1 pixel) 
            0.0, height,    // top->bottom = 1 world unit (1 pixel)
            -1.0, 1.0       // z index from -1 to 1 is visible (everything else clips and is not visible)
        ); 
    }
    glMatrixMode(GL_MODELVIEW); //Inputs into matrix depend on the placement of objects in the matrix overview. Initiate model and view matrix

    glLoadIdentity();//Keep value's axes (matrix * identity matrix = matrix). Initialize the matrix with identity matrix

}

//Scene is running in a loop
void _scene::drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    //clear buffers
    glLoadIdentity(); //Whichever state the scene is in it will stay there


    glTranslatef(-cameraX, -cameraY, 0.0f); // Move the camera based on current position
    myWorld->drawWorld(); // Draw the world
}

// Runs in loop 60 times per second. dt is in ms.
void _scene::updateScene(double dt)
{
    dt = dt / 1000.0; // Convert dt to seconds for easier calculations
    if(A) cameraX -= cameraSpeed*dt; 
    if(D) cameraX += cameraSpeed*dt; 
    if(W) cameraY += cameraSpeed*dt;
    if(S) cameraY -= cameraSpeed*dt;

    if (debugTimer.getTicks() > debugPrintInterval) 
    {
        debugPrint(); 
        debugTimer.reset(); 
    }
}

void _scene::debugPrint()
{
    Logger.LogDebug("Debug Print: W=" + std::to_string(W) + " A=" + std::to_string(A) + " S=" + std::to_string(S) + " D=" + std::to_string(D), LOG_CONSOLE);
}

void _scene::keyboardHandler(WPARAM wParam)
{
    if (inputTimer.getTicks() > 200) // 200 ms debounce time for toggle keys
    {
        switch (wParam)
        {
            case 192: // ~
                commandHandler(); // Enter command mode to input text-based commands
                break;
            case 221: // ]
                debugEnabled = !debugEnabled; 
                Logger.LogInfo("Toggled debug mode: " + std::string(debugEnabled ? "ON" : "OFF"), LOG_CONSOLE);
                break;
            case 219: // [
                inputDebugEnabled = !inputDebugEnabled; 
                Logger.LogInfo("Toggled input debug mode: " + std::string(inputDebugEnabled ? "ON" : "OFF"), LOG_CONSOLE);
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
            //mouseWheelDirection = HIWORD(wParam) > 0 ? 1 : -1;
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