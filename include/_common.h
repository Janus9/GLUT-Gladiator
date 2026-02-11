#ifndef _COMMON_H
#define _COMMON_H

#define GLEW_STATIC

// INCLUDES //

#include <windows.h>
#include <iostream>
#include <GL/glew.h>    // for VBOs
#include <GL/gl.h>
#include <GL/glut.h>
#include <time.h>
#include <math.h>
#include <cmath>
#include <vector>
#include <chrono>
#include <string>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdlib.h>
#include <_logger.h>

// DEFINITIONS //

#define PI 3.14159

// STRUCTS //

struct Vec3f
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct Vec3i
{
    GLint x;
    GLint y;
    GLint z;
};

struct Vec2f
{
    GLfloat x;
    GLfloat y;
};

struct Vec2i
{
    GLint x;
    GLint y;
};

struct Col3f
{
    GLfloat r;
    GLfloat g;
    GLfloat b;
};

using namespace std;

#endif // _COMMON_H