#include <_unit.h>

_unit::_unit() {
    // ctor
}

_unit::~_unit() {
    // dtor
}

void _unit::initUnit(float _maxHealth, float _speed) {
    maxHealth = _maxHealth;
    currentHealth = _maxHealth; // Default HP = Max HP
    speed = _speed;
}

void _unit::drawUnit() {
    drawSprite();
}

const bool _unit::isAlive() {
    return currentHealth > 0.0f; 
}

void _unit::focusOn(const Vec2f &_pos, float speed) {
    float rotNeeded = GetRotationAngle(pos,_pos); 
    if (speed <= 0.0f) {
        rot = rotNeeded;
        return;
    }
    speed /= 60.0f; // Adjust to degrees/second (breaks if update FPS ever changes)
    if (rot < rotNeeded) {
        rot += speed;
    } else {
        rot -= speed;
    }
}