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

// Structure for an int,int -> hash. Used for the unordered map in _world for chunks
struct PairHash {
    std::size_t operator()(const std::pair<int, int>& p) const {
        // Combine two ints into one hash using bit shifting
        // This is fast - just bit operations, no string allocation
        // AI recommended hashing function, don't know how it works -- will research it later
        return std::hash<long long>()(((long long)p.first << 32) | (unsigned int)p.second);
    }
};

// Returns the distance (in world units) between two points
inline float GetDistance(const Vec2f &start, const Vec2f &end) {
    float result = powf(end.x - start.x,2.0f) + powf(end.y - start.y,2.0f);
    return sqrtf(result);
}

// Returns the degrees of rotation required to look at a given object
inline float GetRotationAngle(const Vec2f &start, const Vec2f &end) {
    // find our delta distance
    float dx = start.x - end.x;
    float dy = end.y - start.y;

    float angleDeg = atan2f(dx, dy) * 180.0f / PI;
    angleDeg = fmodf(angleDeg, 360.0f); // Bound angleDeg to 360 degrees
    if (angleDeg < 0.0f) angleDeg += 360.0f; // Bound angleDeg to NOT be negative
    return angleDeg;
}

using namespace std;

#endif // _COMMON_H