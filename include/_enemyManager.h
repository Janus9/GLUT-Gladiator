#ifndef _ENEMY_MANAGER_H
#define _ENEMY_MANAGER_H

#include <_common.h>
#include <_enemy.h>
#include <_player.h>

class _enemyManager {
    public:
        _enemyManager();
        virtual ~_enemyManager();

        void initEnemyManager(_player* currentPlayer);

        /**
         * Update function for enemies
         * 
         * @param dt Delta time (in seconds)
         */
        void updateEnemies(double dt);

        // Draw function for enemies
        void drawEnemies();

        // Adds a single enemy (only 1 type for now)
        void addEnemy(const Vec2f &_pos);

        // Returns number of enemies alive in the list
        int getNumEnemies();
    protected:
    private:
        _player* player = nullptr;              // Pointer to currently active player instance (non-owning)
        vector<unique_ptr<_enemy>> enemyList;   // List of enemy instances
};

#endif // _ENEMY_MANAGER_H
