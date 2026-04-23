#ifndef _ENEMY_MANAGER_H
#define _ENEMY_MANAGER_H

#include <_common.h>
#include <_player.h>
#include <_world.h>
#include <_bulletManager.h>
#include <_particleManager.h>
#include <_sounds.h>

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
        
        float fireRate = 200; // In rounds per minute

        double firingTime = 0.0;    // Elapsed time for shooting
        double deathTime = 0.0;     // Elapsed time for death

        bool inDeathAnimation = false; // Whether enemy is in its death animation
    protected:
    private:
        int enemyID;
        static int nextId;
};

class _enemyManager {
    public:
        _enemyManager();
        virtual ~_enemyManager();

        void initEnemyManager(_player* currentPlayer, _world* currentWorld, _bulletManager* currentBulletManager,_bullet_config* _bullet_1, _sounds* currentSounds);

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

        /**
         * Checks if any enemy instance is colliding with the provided position
         * 
         * @param pos Position of object
         * @param registerDistance Distance for collision to register 
         * 
         * @return Pointer to enemy collision occured with, or nullptr if none
         */
        _enemy* isColliding(const Vec2f &pos, float registerDistance) const;

        // Returns number of enemies alive in the list
        int getNumEnemies();
    protected:
    private:
        _player* player = nullptr;                  // Pointer to player instance instantiated in scene (non-owning)
        _world* world = nullptr;                    // Pointer to world instance instantiated in scene (non-owning)
        _bulletManager* bulletManager = nullptr;    // Pointer to bulletManager instance instantiated in scene (non-owning)
        _bullet_config* bullet_1 = nullptr;
        _sounds* sounds = nullptr;                  // Pointer to sounds instance instantiated in scene (non-owning)

        vector<unique_ptr<_enemy>> enemyList;   // List of enemy instances

        unique_ptr<_particleManager> particleManager = make_unique<_particleManager>();
        particle_effect turret_hit_effect;
};

#endif // _ENEMY_MANAGER_H
