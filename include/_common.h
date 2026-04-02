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
#include <limits>
#include <algorithm>
#include <unordered_map> // Hashtable

// USING // 

using namespace std;

// DEFINITIONS //

#define PI 3.14159

// STRUCTS //

// Structure for a Vector 2D of type float containg simple functions and operator overloads
struct Vec2f 
{
    float x;
    float y;

    Vec2f() {
        x = 0.0f;
        y = 0.0f;
    }

    Vec2f(float _x, float _y) {
        x = _x;
        y = _y;
    }

    // Performs addition on another Vec2f and returns the result
    Vec2f operator+(const Vec2f &other) const {
        return Vec2f(x + other.x, y + other.y);
    }
    
    // Adds another Vec2f to this current object and returns it
    Vec2f& operator+=(const Vec2f &other) { // We use & to send the object by reference and not by copy (more efficient)
        x += other.x;
        y += other.y;
        return *this; // This is a pointer, we must dereference it
    }

    // Performs subtraction on another Vec2f and returns the result
    Vec2f operator-(const Vec2f &other) const {
        return Vec2f(x - other.x, y - other.y);
    } 

    // Subtracts another Vec2f to this current object and returns it
    Vec2f& operator-=(const Vec2f &other) { 
        x -= other.x;
        y -= other.y;
        return *this; // This is a pointer, we must dereference it
    }

    // Performs multiplication on a scalar float and returns the result
    Vec2f operator*(float other) const {
        return Vec2f(x*other, y*other);
    }

    // Performs multiplication of a scalar float onto the current Vec2f and returns it
    Vec2f& operator*=(float other) { 
        x *= other;
        y *= other;
        return *this; // This is a pointer, we must dereference it
    }

    // Performs a dot product between this and another Vec2f and returns the result
    float dot(const Vec2f &other) const {
        return x*other.x + y*other.y;
    }

    // Retrieves the length using pythagorean theorem
    float length() const {
        return sqrt(x*x + y*y);
    }

    // Performs a normalization on the current vector and returns the result
    Vec2f normalized() const {
        float len = length();
        if (len == 0.0f) return Vec2f(0.0f, 0.0f);  // Prevent divide by 0 errors
        return Vec2f(x / len, y / len);
    }

    // Normalizes the current Vec2f
    void normalize() {
        float len = length();
        if (len == 0.0f) {
           // Prevent divide by 0 errors
           x = 0.0f;
           y = 0.0f;
           return; 
        } 
        x = x / len;
        y = y / len;
    }

    // Returns the scalar distance between this Vec2f and another
    float distance(const Vec2f &other) const {
        Vec2f delta = *this - other;
        return sqrt(delta.x * delta.x + delta.y * delta.y);
    }

    // Returns a negated version of the current vector (flips sign of x,y)
    Vec2f operator-() const {
        return Vec2f(-x, -y);
    }

    // Performs a cross product on this and another Vec2f returning a scalar result
    float cross(const Vec2f &other) const {
        return x * other.y - y * other.x;
    }

    /**
     * Converts the Vec2f into an easy to visualize string for debugging
     * 
     * @param unit Unit to show next to values (defaults to nothing)
     * 
     * @return string formated as (x[unit], y[unit]) with no newline
     */
    string toString(const string &unit = "") const {
        return "(" + to_string(x) + unit + ", " + to_string(y) + unit + ")";
    }

    /**
     * Draws the vector for debugging purposes
     * 
     * @param pos Position for the Vec2f to start from as a position
     */
    void drawVec2f(const Vec2f &pos, float mulX = 1.0f, float mulY = 1.0f) const {
        glPushMatrix();
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);

            glColor3f(1.0f,0.0f,0.0f);

            glBegin(GL_LINES);  
                glVertex2f(pos.x,pos.y); // Start point
                glVertex2f(x*mulX,y*mulY); // End point
            glEnd();

            glEnable(GL_TEXTURE_2D);
            glEnable(GL_LIGHTING);
            glEnable(GL_DEPTH_TEST);
        glPopMatrix();
    }
};


// Structure for a Vector #D of type float containg simple functions and operator overloads
struct Vec3f 
{
    float x;
    float y;
    float z;

    Vec3f() {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
    }

    Vec3f(float _x, float _y, float _z) {
        x = _x;
        y = _y;
        z = _z;
    }

    // Performs addition on another Vec3f and returns the result
    Vec3f operator+(const Vec3f &other) const {
        return Vec3f(x + other.x, y + other.y, z + other.z);
    }

    // Adds another Vec3f to this current object and returns it
    Vec3f& operator+=(const Vec3f &other) { // We use & to send the object by reference and not by copy (more efficient)
        x += other.x;
        y += other.y;
        z += other.z;
        return *this; // This is a pointer, we must dereference it
    }

    // Performs subtraction on another Vec3f and returns the result
    Vec3f operator-(const Vec3f &other) const {
        return Vec3f(x - other.x, y - other.y, z - other.z);
    } 

    // Subtracts another Vec3f to this current object and returns it
    Vec3f& operator-=(const Vec3f &other) { 
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this; // This is a pointer, we must dereference it
    }

    // Performs multiplication on a scalar float and returns the result
    Vec3f operator*(float other) const {
        return Vec3f(x*other, y*other, z*other);
    }

    // Performs multiplication of a scalar float onto the current Vec3f and returns it
    Vec3f& operator*=(float other) { 
        x *= other;
        y *= other;
        z *= other;
        return *this; // This is a pointer, we must dereference it
    }

    // Performs a dot product between this and another Vec3f and returns the result
    float dot(const Vec3f &other) const {
        return x*other.x + y*other.y + z*other.z;
    }

    // Retrieves the length using pythagorean theorem
    float length() const {
        return sqrt(x*x + y*y + z*z);
    }

    // Performs a normalization on the current vector and returns the result
    Vec3f normalized() const {
        float len = length();
        if (len == 0.0f) return Vec3f(0.0f, 0.0f, 0.0f);  // Prevent divide by 0 errors
        return Vec3f(x / len, y / len, z / len);
    }

    // Normalizes the current Vec3f
    void normalize() {
        float len = length();
        if (len == 0.0f) {
           // Prevent divide by 0 errors
           x = 0.0f;
           y = 0.0f;
           z = 0.0f;
           return; 
        } 
        x = x / len;
        y = y / len;
        z = z / len;
    }

    // Returns the scalar distance between this Vec3f and another
    float distance(const Vec3f &other) const {
        Vec3f delta = *this - other;
        return sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
    }

    // Returns a negated version of the current vector (flips sign of x,y)
    Vec3f operator-() const {
        return Vec3f(-x, -y, -z);
    }

    // Performs a cross product between the current Vec3f and another returning the result
    Vec3f cross(const Vec3f &other) const {
        return Vec3f(
            y*other.z - z*other.y, 
            z*other.x - x*other.z, 
            x*other.y - y*other.x
        );
    }

    /**
     * Converts the Vec3f into an easy to visualize string for debugging
     * 
     * @param unit Unit to show next to values (defaults to nothing)
     * 
     * @return string formated as (x[unit], y[unit], z[unit]) with no newline
     */
    string toString(const string &unit = "") const {
        return "(" + to_string(x) + unit + ", " + to_string(y) + unit + ", " + to_string(z) + unit + ")";
    }
};

struct Vec3i
{
    GLint x;
    GLint y;
    GLint z;
};

struct Vec2i
{
    GLint x;
    GLint y;

    Vec2i() {
        x = 0;
        y = 0;
    }

    Vec2i(const GLint _x, const GLint _y) {
        x = _x;
        y = _y;
    }

    /**
     * Converts the Vec2i into an easy to visualize string for debugging
     * 
     * @param unit Unit to show next to values (defaults to nothing)
     * 
     * @return string formated as (x[unit], y[unit]) with no newline
     */
    string toString(const string &unit = "") const {
        return "(" + to_string(x) + unit + ", " + to_string(y) + unit + ")";
    }
};

struct Col3f
{
    GLfloat r;
    GLfloat g;
    GLfloat b;
};

struct TilePosI
{
    uint8_t row;
    uint8_t col;
};

/**
 * Negate helper for floats
 * 
 * @param val Value to process
 * @param negate True means negate
 * 
 * @returns Negated value if true 
 */
inline float negatef(float val, bool negate) {
    if (negate) return -val;
    return val;
}

// Returns radian conversion from degree input
inline float degreeToRad(float degree) {
    return degree * (PI / 180.0);
}

// Returns degree conversion from radian input
inline float radToDegree(float rad) {
    return rad * (180.0 / PI);
}

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

/**
 * Performs modulus of b on a using floor modulus. This is true mathematical modulus versus c++ division based.
 * 
 * @param a Value to mod
 * 
 * @param b Mod by
 * 
 * @return int value result
 */
inline int modFloor(int a, int b)
{
    int r = a % b;
    if (r < 0)
    {
        r += b;
    }
    return r;
}

#endif // _COMMON_H