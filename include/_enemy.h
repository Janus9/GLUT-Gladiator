#ifndef _ENEMY_H
#define _ENEMY_H

#include <_common.h>
#include <_unit.h>

class _enemy : public _unit {
    public:
        _enemy();
        virtual ~_enemy();

        /**
         * Update function for an enemy
         * 
         * @param dt Delta Time (in seconds)
         */
        void updateEnemy(double dt);

        // Initialization function for animations/sprites/textures etc
        void initEnemy();

        bool operator==(const _enemy &other) const;
    protected:
    private:
        int enemyID;
        static int nextId;
};

#endif // _ENEMY_H