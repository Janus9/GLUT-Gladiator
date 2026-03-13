#ifndef _UNIT_H
#define _UNIT_H

#include <_common.h>
#include <_sprite.h>

class _unit : public _sprite {
    public:
        _unit();
        virtual ~_unit();

        enum class Team {
            NEUTRAL = 0,
            FRIENDLY = 1,
            ENEMY = 2
        };

        // Setup unit with health/speed (HP units, pixels/second)
        void initUnit(float _maxHealth = 100.0f, float _speed = 32.0f);
        void drawUnit();
        // Enforces the unit to rotate to a given point given a speed entry in degrees/second (entries <= 0 mean instant rotation)
        void focusOn(const Vec2f &_pos, float speed = 0.0f);

        float maxHealth = 0.0f;
        float currentHealth = 0.0f;
        float speed = 0.0f;
        
        Team team = Team::NEUTRAL;

        // Returns the units alive state based on health points
        const bool isAlive();
        
        // Physics //
        Vec2f velocity; 
        Vec2f acceleration;

    protected:
    private:
};

#endif // _UNIT_H