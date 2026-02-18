
#pragma once 

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#define UPDATE_DELAY (1000.0f / 60.0f)	// Delay in milliseconds for 60 updates per second

//Find these libraries within the project folders
//Exact paths may not be found but the libraries will be
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#include<_common.h>         // For common headers
#include<_scene.h>          // My scene context
#include<_timerPlusPlus.h>   // For the timer class

HDC			hDC=NULL;		// Private GDI Device Context
HGLRC		hRC=NULL;		// Permanent Rendering Context
HWND		hWnd=NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application

bool	keys[256];			// Array Used For The Keyboard Routine
bool	active=TRUE;		// Window Active Flag Set To TRUE By Default. Active environment or not (game will pause based on these settings)
bool	fullscreen=TRUE;	// Fullscreen Flag Set To Fullscreen Mode By Default (is the screen full-screen or not)

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc. Passing parameters into system


// CLASS INSTANCE DECLARATIONS //
_logger Logger; //New instance of logger to be used in the program. Logger will be used to track specific values and print them to the console for debugging purposes
_scene *myScene = new _scene();//handling memory to point to specific scene values. makes an instance of scene
_timerPlusPlus Timer; // Timer for handling update loop for scenese

//An option to parallel-check each individual function would be loops to handle interrupts
//Parallel while loops to be able to handle in program is handled by the callback function

/*
Top-Layer System: Application Layer
Operating System
Kernel: Communication Layer between OS and Hardware
Bottom-Layer System: Hardware
--Drivers follow through the system and translate the hardware to be controlled by the software
--Driver handling occurs in the hardware
--Keyboard, Mouse, and other peripherals should interrupt the signal to trigger a process
--Interrupt Handling allows for the interrupts needed to be processed properly
*/

#include <windows.h>

/////////////////////////////////////////////////////////////////////////////////////////////////
//										THE KILL GL WINDOW
/////////////////////////////////////////////////////////////////////////////////////////////////
void KillGLWindow()								// Properly Kill The Window
{
	if (fullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL",hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// Set hInstance To NULL
	}
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/

/////////////////////////////////////////////////////////////////////////////////////////////////
//										THE CREATE GL WINDOW
/////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag) //title of window, width/height of window space, amount of bits on window, and whether or not a fullscreen is live
{
//OpenGL center of the window is (0,0). Mouse pointer coordinates need to match the instruction set of the window itself
//Width/2 and height/2 is the center of the window with a translation
//Mouse can act weird given rationale between structure windows and OpenGL (matched to meet windows specification)

	int		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

	fullscreen=fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION); //Message Box specific to pop-ups
		return FALSE;											// Return FALSE
	}

	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle= WS_POPUP;			// must handle Gsync situations: Windows Style
		ShowCursor(FALSE);									// Hide Mouse Pointer
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

//Pixels are handled by the buffer to process each of the elements of the image
//Animation, for instance, shifts between the buffers based on whichever one is loaded or loading
//
	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	if (!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority. Temporary screen until front window arrives
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window. Main Active Window

	myScene->reSize(width,height);  //Intended to resize the scene to match the width and height
	if(!myScene->initGL())                          //If scene does not initialize
    {
        KillGLWindow();    //Kills window if game does not initialize
        MessageBox(NULL,"Can't Initialize GL.", "ERROR", MB_OK|MB_ICONEXCLAMATION); //Message Box declares that OpenGL crashed
	}
	Logger.InitLogger("logs/main.log");
	Timer.reset();
	return TRUE;									// Success
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//										THE WINDOW PROCEDURE
/////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{
	myScene->winMsg(hWnd, uMsg, wParam, lParam); //Passes the message to the scene to be handled by the scene class. This allows for the scene to handle specific messages related to the scene itself (such as mouse movement and key presses)
	switch (uMsg)									// Check For Windows Messages
	{
	    //Is Window Currently Active?
		case WM_ACTIVATE:							// Watch For Window Activate Message
		{
			if (!HIWORD(wParam))					// Check Minimization State
			{
				active=TRUE;						// Program Is Active
			}
			else
			{
				active=FALSE;						// Program Is No Longer Active
			}

			return 0;								// Return To The Message Loop
		}

		case WM_SYSCOMMAND:							// Intercept System Commands
		{
			switch (wParam)							// Check System Calls
			{
				case SC_SCREENSAVE:					// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
				return 0;							// Prevent From Happening
			}
			break;									// Exit
		}


        //Is the window closed?
		case WM_CLOSE:								// Did We Receive A Close Message?
		{
			PostQuitMessage(0);						// Send A Quit Message
			return 0;								// Jump Back
		}

		//Was a Key Pressed?
		case WM_KEYDOWN:							// Is A Key Being Held Down?
		{
			keys[wParam] = TRUE;					// If So, Mark It As TRUE
			return 0;								// Jump Back
		}

		//Was a Key-Released
		//Wparam: Windows Parameter refers to the specific type of key that it is
		//A specific parameter based on the key that was pressed
		case WM_KEYUP:								// Has A Key Been Released?
		{
			keys[wParam] = FALSE;					// If So, Mark It As FALSE

			return 0;								// Jump Back
		}

		//Glut Resize Window function to change the size of the window
		//All includes and prototype allocation starts a program and then goes to main
		case WM_SIZE:								// Resize The OpenGL Window
		{
            myScene->reSize(LOWORD(lParam), HIWORD(lParam));    // LoWord=Width, HiWord=Height. Resize happens when the size of the window is forced to change
			return 0;								// Jump Back
		}
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//										THE WINMAIN
/////////////////////////////////////////////////////////////////////////////////////////////////

//Main program for game
int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
					HINSTANCE	hPrevInstance,		// Previous Instance
					LPSTR		lpCmdLine,			// Command Line Parameters
					int			nCmdShow)			// Window Show State
{
	MSG		msg;									// Windows Message Structure
	BOOL	done=FALSE;								// Bool Variable To Exit Loop

	//If a message arrives it will arrive in this manner

	int	fullscreenWidth  = GetSystemMetrics(SM_CXSCREEN); //Whenever a window is created, the size of the window is maintained
    int	fullscreenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Ask The User Which Screen Mode They Prefer
	//First message shown

	if (MessageBox(NULL,"Would You Like To Run In Fullscreen Mode?", "Start FullScreen?",MB_YESNO|MB_ICONQUESTION)==IDNO) 	//Message, Title, Buttons
	{
		fullscreen=FALSE;							// Windowed Mode
	}
	// Create Our OpenGL Window
	if (!CreateGLWindow("Game Engine Lesson 01",fullscreenWidth,fullscreenHeight,256,fullscreen))
	{
		return 0;									// Quit If Window Was Not Created, Program is exited
	}

	while(!done)									// Loop That Runs While done=FALSE
        //Program will run in a while loop until the user states it is done
        //Parallel ideas interlinking together
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
			{
				done=TRUE;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
		if (keys[VK_ESCAPE])
			{
				done=TRUE;							// ESC or DrawGLScene Signalled A Quit
			}
			else									// Not Time To Quit, Update Screen
			{
				myScene->drawScene(); //So long as the key is not escaping (quitting), keep drawing the scene
				if (Timer.getTicks() > UPDATE_DELAY) // If the time since the last update is greater than 16.67ms (60fps), update the scene
				{
					myScene->updateScene(Timer.getTicks()); //Update the scene with the time since the last update
					Timer.reset(); // Reset the timer for the next update
				}
				SwapBuffers(hDC);				// Swap Buffers (Double Buffering)
			}

			if (keys[VK_F1])						// Is F1 Being Pressed?
			{
				keys[VK_F1]=FALSE;					// If So Make Key FALSE
				KillGLWindow();						// Kill Our Current Window
				fullscreen=!fullscreen;				// Toggle Fullscreen / Windowed Mode
				// Recreate Our OpenGL Window
				if (!CreateGLWindow("Game Engine Lesson 01",fullscreenWidth,fullscreenHeight,256,fullscreen))
				{
					return 0;						// Quit If Window Was Not Created
				}
			}
		}
	}

	// Shutdown
	KillGLWindow();									// Kill The Window
	return (msg.wParam);							// Exit The Program. Program is then exited after the parameter has been fulfilled
}

