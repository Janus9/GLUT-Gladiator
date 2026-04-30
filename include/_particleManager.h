#ifndef _PARTICLE_MANAGER_H
#define _PARTICLE_MANAGER_H

#include <_common.h>
#include <_texture.h>
#include <_timerPlusPlus.h>
#include <_shader.h>

#include <glm/glm.hpp>                      // Core library
#include <glm/gtc/matrix_transform.hpp>     // Matrix ops like transform, scale, ortho, etc
#include <glm/gtc/type_ptr.hpp>             // Send GLM datatypes (matrix) to GPU

/**
 * Particle Effect Setting
 * 
 * Allows for creating a customizable particle effect to be loaded into spawnEffect(). 
 */
struct particle_effect {
    int amount = 0;                // Amount of particles to spawn

    float minVelX = 0.0f;          // Minimum horizontal velocity
    float maxVelX = 0.0f;          // Maximum horizontal velocity 
    float minVelY = 0.0f;          // Minimum verticle velocity
    float maxVelY = 0.0f;          // Maximum verticle velocity

    float minRadius = 0.0f;        // Minimum particle size
    float maxRadius = 0.0f;        // Maximum particle size

    float minLifeTime = 0.0f;      // Minimum particle lifespan (in seconds)
    float maxLifeTime = 0.0f;      // Maximum particle lifespan (in seconds)

    float minSpawnOffsetX = 0.0f;  // Minimum horizontal offset of particle spawn
    float maxSpawnOffsetX = 0.0f;  // Maximum horizontal offset of particle spawn
    float minSpawnOffsetY = 0.0f;  // Minimum verticle offset of particle spawn
    float maxSpawnOffsetY = 0.0f;  // Maximum verticle offset of particle spawn

    float minRotation = 0.0f;      // Minimum Rotation speed of the particles (degrees/second) 
    float maxRotation = 0.0f;      // Maximum Rotation speed of the particles (degrees/second)
};

struct particle {
    bool alive = false;

    // Physics //
    Vec2f pos = {0.0f,0.0f};              
    Vec2f vel = {0.0f,0.0f};              
    Vec2f acc = {0.0f,0.0f};
    float rotationSpeed = 0.0f;
    
    float radius = 0.0f;

    float death = 0.0f; // When the particle will die (an offset from birth decides this)
    float age = 0.0f; // Particles current age

    float angle = 0.0f;
};

class _particleManager {
    public:
        _particleManager();
        ~_particleManager();

        /**
         * Initializes the particle manager
         * 
         * @param fileName Image to apply to each particle (cannot be changed)
         * @param maxParticles Number of particles the system holds (defaults to 10,000)
         */
        void initParticleManager(const string& fileName, int _maxParticles = 10000);

        // Draw function
        void drawParticleManager();

        // Update function
        void updateParticleManger(double dt);

        /**
         * Spawns a particle effect at the given position.
         * 
         * @param pos Position to spawn the particle effect (in world coordinates)
         * @param effect An effect setting to load. Should be set beforehand, check particle_effect docs for implementation
         */
        void spawnEffect(const Vec2f &pos, const particle_effect &effect);

        // Sets the view projection matrix
        static void setViewProjectionMatrix(const glm::mat4 &_viewProjectionMatrix);
    protected:
    private:
        // Builds dynamic VBO
        void buildVBO(); 
        // Builds static EBO
        void buildEBO();
        // Builds the VAO
        void buildVAO();

        GLuint vboID = 0;
        GLuint eboID = 0;
        GLuint vaoID = 0;

        bool eboBuilt = false;
        
        // Maximum particles the system supports
        int maxParticles = 0;
        // Number of particles activley being used
        int aliveParticles = 0;

        _texture* texture = new _texture();
        _timerPlusPlus* timer = new _timerPlusPlus();

        vector<particle> particles;

        // Shaders //
        _shader particleShader;
        GLint u_viewProjectionMatrix = -1;
        GLint u_texture = -1;

        static glm::mat4 viewProjectionMatrix;

        // Rng machine
        mt19937 rng;   
};

#endif // _PARTICLE_MANAGER_H
