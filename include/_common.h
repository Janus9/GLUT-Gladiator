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
}

// // Converts a chunk position into a string to be used as a key for the loadedChunks unordered map. This isnt a good key making a string is expensive
inline std::string ChunkPosToKey(Vec2i chunkPos) {
    return std::to_string(chunkPos.x) + "," + std::to_string(chunkPos.y);
}

// Dont know what inline does
// Converts a chunk position (using individual components) into a string to be used as a key for the loadedChunks unordered map. This isnt a good key making a string is expensive
inline std::string ChunkPosToKey(int chunkX, int chunkY) {
    return std::to_string(chunkX) + "," + std::to_string(chunkY);
}

using namespace std;

#endif // _COMMON_H