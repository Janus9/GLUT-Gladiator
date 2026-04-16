#ifndef _BULLET_MANAGER_H
#define _BULLET_MANAGER_H

#define MAX_BULLETS 2028

#include <_common.h>
#include <_world.h>


/**
 * Bullet Config
 * 
 * @param amount Amount of bullets to spawn in effect 
 * @param angleOffset Angle offset (in degrees) the bullets can fire left or right. Zero means 100% accuracy; 
 * @param vel Speed of bullet (world units / second)
 * @param width Width for draw/collision (in world units)
 * @param height Height for draw/collision (in world units)
 * 
 */
struct _bullet_config {
    int amount = 0; // Amount to spawn

    float angleOffset = 0.0f; // Deviation L/R in bullets

    // Physics
    Vec2f vel = {0.0f, 0.0f};

    // Bullet dimensions for drawing / collision
    float width = 0.0f;
    float height = 0.0f;
};

class _bulletManager {
    public:
        _bulletManager();
        virtual ~_bulletManager();

        /**
         * Initialization of the bullet manager
         * 
         * @param world Pointer to world where manager operates
         * 
         * @return True if world was initialized successfully
         */
        bool initBulletManager(_world* world); 

        /**
         * Draw function
         */
        void drawBulletManager();

        /**
         * Update function for bullets.
         * Handles collision and lifetime. Manages world interactions (requires world to be setup).
         * 
         * @param dt Delta time (in seconds) for bullet updates 
         */
        void updateBulletManager(double dt);
        
        /**
         * @param pos Position where to spawn bullet effect
         * @param config _bullet_config of settings for the effect (see struct for implementation info)
         */
        void spawnBulletEffect(const Vec2f &pos, const _bullet_config &config);

    protected:
    private:
        _world* currentWorld = nullptr; // What world this bullet exists in, set in init
        
        // Bullet instance
        struct _bullet {
            // Lifetime
            bool alive = false;

            // Physics
            Vec2f pos = {0.0f, 0.0f};
            Vec2f vel = {0.0f, 0.0f};

            // Bullet dimensions for drawing / collision
            float width = 0.0f;
            float height = 0.0f;
        };
        
        _bullet bulletPool[MAX_BULLETS]; // Bullet pool
        
        int aliveBullets = 0;
        bool stopDrawing = false; // Stops drawing when all bullets are found dead (save performance)

        GLuint vboID = 0;
        GLuint eboID = 0;

        void buildVbo(); // Builds the VBO
        void buildEbo(); // Builds the EBO (only done once)

        // Rng machine
        mt19937 rng;
};

#endif // _BULLET_MANAGER_H