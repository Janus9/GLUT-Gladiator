#ifndef _BULLET_MANAGER_H
#define _BULLET_MANAGER_H

#define MAX_BULLETS 2028

#include <_common.h>
#include <_world.h>
#include <_particleManager.h>
#include <_shader.h>
#include <_player.h>
#include <_sounds.h>

#include <glm/glm.hpp>                      // Core library
#include <glm/gtc/matrix_transform.hpp>     // Matrix ops like transform, scale, ortho, etc
#include <glm/gtc/type_ptr.hpp>             // Send GLM datatypes (matrix) to GPU

class _enemyManager;

/**
 * Bullet Config
 * 
 * @param amount Amount of bullets to spawn in effect 
 * @param angleOffset Angle offset (in degrees) the bullets can fire left or right. Zero means 100% accuracy; 
 * @param speed Speed of bullet (world units / second)
 * @param width Width for draw/collision (in world units)
 * @param height Height for draw/collision (in world units)
 * @param lifespan How long the bullet lives for (in seconds)
 * @param damage Damage bullet applies on hit
 * @param penetration Higher means bullet pierces more units/tiles before death (ex/ 100 means 2 tiles or 4 enemies) (0 means none)
 */
struct _bullet_config {
    int amount = 0; // Amount to spawn

    float angleOffset = 0.0f; // Deviation L/R in bullets

    // Physics
    float speed = 0.0f;

    // Bullet dimensions for drawing / collision
    float width = 0.0f;
    float height = 0.0f;
    
    // Lifetime
    float lifespan = 0.0f;

    float damage = 0.0f; // How much damage bullet applies on hit
    float penetration = 0.0f; // How many tiles (or units) it penetrates
};

class _bulletManager {
    public:
        _bulletManager();
        virtual ~_bulletManager();

        /**
         * Initialization of the bullet manager.
         * The image must face right and lie horizontal
         * 
         * @param fileName Name of file for bullet image
         * @param world Pointer to world where manager operates
         */
        void initBulletManager(const string &fileName, _world* currentWorld, _player* currentPlayer, _enemyManager* currentEnemyManager, _sounds* currentSounds);

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
         * @param dest Position where bullet is aimed at
         * @param config _bullet_config of settings for the effect (see struct for implementation info)
         */
        void spawnBulletEffect(const Vec2f &pos, const Vec2f &dest, _team bulletTeam, const _bullet_config &config);

        /**
         * Statically sets the window viewport.
         * Needed for bullet shaders.
         * Call in the scene cameraUpdate function or where the glOrtho command is called.
         * 
         * @param _left Window left bound from scene
         * @param _right Window right bound from scene
         * @param _top Window top bound from scene
         * @param _bottom Window bottom bound from scene
         */
        static void setViewportDimensions(float _left, float _right, float _top, float _bottom);    

        // Sets the view projection matrix
        static void setViewProjectionMatrix(const glm::mat4 &_viewProjectionMatrix);
    protected:
    private:
        _player* player = nullptr;                  // Pointer to player instance instantiated in scene (non-owning)
        _world* world = nullptr;                    // Pointer to world instance instantiated in scene (non-owning)
        _enemyManager* enemyManager = nullptr;      // Pointer to enemyManager instance instantiated in scene (non-owning)
        _sounds* sounds = nullptr;                  // Pointer to sounds instance instantiated in scene (non-owning)

        _texture* texture = new _texture();
        _particleManager* bulletDrops = new _particleManager();
        particle_effect bullet_shell_effect;

        _shader bulletShader;
        
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

            float angle = 0.0f; // Direction to face (in radians)

            float lifespan = 0.0f; // How long (in seconds) bullet lives 
            float age = 0.0f; // Bullets current age

            float damage = 0.0f;
            float health = 0.0f;

            _team team;
        };
        
        _bullet bulletPool[MAX_BULLETS]; // Bullet pool
        
        int aliveBullets = 0;

        GLuint vboID = 0;
        GLuint eboID = 0;
        GLuint vaoID = 0;
        
        GLint u_dimensions = -1;
        GLint u_texture = -1;
        GLint u_viewProjectionMatrix = -1;

        void buildVbo(); // Builds the VBO
        void buildEbo(); // Builds the EBO (only done once)
        void buildVao(); // Builds the VAO (only done once)

        // Static Viewport Dimensions
        static float left;
        static float right;
        static float top;
        static float bottom;

        static glm::mat4 viewProjectionMatrix;
        
        // Rng machine
        mt19937 rng;
};

#endif // _BULLET_MANAGER_H