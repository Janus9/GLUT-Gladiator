#ifndef _COMMON_H
#define _COMMON_H

#define GLEW_STATIC

// INCLUDES //

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
#include <cstdint>  // for uint32_t and other fixed-width integer types
#include <_logger.h>
#include <random>
#include <unordered_map> // Hashtable

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

struct TilePosI
{
    uint8_t row;
    uint8_t col;
};

struct Col3f
{
    GLfloat r;
    GLfloat g;
    GLfloat b;
};

struct BenchmarkPackage
{
    std::string name = "";      
    double benchmarkTicks = 0.0;
    int numIterations = 0;    
};

// Structure for an int,int -> hash. Used for the unordered map in _world for chunks
struct PairHash {
    std::size_t operator()(const std::pair<int, int>& p) const {
        // Combine two ints into one hash using bit shifting
        // This is fast - just bit operations, no string allocation
        // AI recommended hashing function, don't know how it works -- will research it later
        return std::hash<long long>()(((long long)p.first << 32) | (unsigned int)p.second);
    }
};

using namespace std;

#endif // _COMMON_H