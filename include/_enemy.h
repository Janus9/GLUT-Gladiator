#ifndef _ENEMY_H
#define _ENEMY_H

#include <_common.h>
#include <_unit.h>

class _enemy : public _unit {
    public:
        _enemy();
        virtual ~_enemy();

        // Initialization function for animations/sprites/textures etc
        void initEnemy();
    protected:
    private:
};

#endif // _ENEMY_H