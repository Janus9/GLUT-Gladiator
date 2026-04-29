#ifndef _ENEMY_MANAGER_H
#define _ENEMY_MANAGER_H

#include <_common.h>
#include <_player.h>
#include <_world.h>
#include <_bulletManager.h>
#include <_particleManager.h>
#include <_sounds.h>

enum enemy_type : uint8_t {
    ENEMY_TURRET,
    ENEMY_GATLING
};

/**
 * This is for serializing data for world saving. It stores enemy data.
 */
struct enemy_serial_data {
    uint8_t type;
    uint8_t team;
    float maxHP;
    float fireRate;
    float slewRate;
    float detectionRadius;
};

/**
 * Configuration for creating an enemy
 * 
 * @param type Type of enemy to spawn 
 * @param team Team for the enemy 
 * @param maxHP Maximum (and current hp on spawn) enemy has
 * @param fireRate How fast the enemy shoots
 * @param slewRate How fast the enemy rotates to face player
 * @param detectionRadius How far away enemy detects player
 */
struct enemy_config {
    enemy_type type;
    _team team;
    
    float maxHP;
    float fireRate;
    float slewRate;
    float detectionRadius;
};


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
        void initEnemy(const enemy_config &config);

        // Returns a serialized struct of the enemy
        enemy_serial_data serializeEnemy() const;
  
        bool operator==(const _enemy &other) const;
        
        float fireRate = 0.0;               // In rounds per minute
        float slewRate = 0.0f;              // In degrees/second
        float detectionRadius = 0.0f;       // How far enemy spots player

        double firingTime = 0.0;    // Elapsed time for shooting
        double deathTime = 0.0;     // Elapsed time for death

        bool inDeathAnimation = false; // Whether enemy is in its death animation
        enemy_type eType;
    protected:
    private:
        int enemyID;
        static int nextId;
};

class _enemyManager {
    public:
        _enemyManager();
        virtual ~_enemyManager();

        void initEnemyManager(_player* currentPlayer, _world* currentWorld, _bulletManager* currentBulletManager, _sounds* currentSounds);

        /**
         * Update function for enemies
         * 
         * @param dt Delta time (in seconds)
         */
        void updateEnemies(double dt);

        // Draw function for enemies
        void drawEnemies();

        // Adds a single enemy (only 1 type for now)
        void addEnemy(const Vec2f &_pos, const enemy_config &config);

        // Returns a vector of all the serialized enemies for saving
        vector<enemy_serial_data> serializeEnemies() const;

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

        _bullet_config* bullet_1 = nullptr;
        _bullet_config* bullet_2 = nullptr;
    protected:
    private:
        _player* player = nullptr;                  // Pointer to player instance instantiated in scene (non-owning)
        _world* world = nullptr;                    // Pointer to world instance instantiated in scene (non-owning)
        _bulletManager* bulletManager = nullptr;    // Pointer to bulletManager instance instantiated in scene (non-owning)
        
        _sounds* sounds = nullptr;                  // Pointer to sounds instance instantiated in scene (non-owning)

        vector<unique_ptr<_enemy>> enemyList;   // List of enemy instances

        unique_ptr<_particleManager> particleManager = make_unique<_particleManager>();
        particle_effect turret_hit_effect;
};

#endif // _ENEMY_MANAGER_H
