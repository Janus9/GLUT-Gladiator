#ifndef _PARTICLE_ENGINE
#define _PARTICLE_ENGINE

#include <_common.h>
#include <_textureManager.h>
#include <_lightManager.h>
#include <_shader.h>

#include <glm/glm.hpp>                      // Core library
#include <glm/gtc/matrix_transform.hpp>     // Matrix ops like transform, scale, ortho, etc
#include <glm/gtc/type_ptr.hpp>             // Send GLM datatypes (matrix) to GPU

/**
 * Particle Effect Configuration
 */
struct particle_effect_config {
    int particleCount = 0;         /// Required: number of particles to spawn.

    // Animation //
    std::string texturePath;       /// Required: particle sprite sheet image path.
    int sheetColumns = 0;          /// Required: number of frame columns in the sprite sheet.
    int sheetRows = 0;             /// Required: number of frame rows in the sprite sheet.

    int animationRow = 0;          /// Default 0: row of the sprite sheet used by this effect.
    int animationFPS = 0;          /// Default 0: animation frame rate in frames/second. Use 0 for static particles.

    // Velocity //
    float minVelX = 0.0f;          /// Default 0.0f: minimum horizontal velocity.
    float maxVelX = 0.0f;          /// Default 0.0f: maximum horizontal velocity.
    float minVelY = 0.0f;          /// Default 0.0f: minimum vertical velocity.
    float maxVelY = 0.0f;          /// Default 0.0f: maximum vertical velocity.

    // Size //
    float minRadius = 0.0f;        /// Default 0.0f: minimum particle radius/size.
    float maxRadius = 0.0f;        /// Default 0.0f: maximum particle radius/size.

    // Lifetime //
    float minLifeTime = 0.0f;      /// Default 0.0f: minimum particle lifespan in seconds.
    float maxLifeTime = 0.0f;      /// Default 0.0f: maximum particle lifespan in seconds.

    // Spawn Offset //
    float minSpawnOffsetX = 0.0f;  /// Default 0.0f: minimum horizontal spawn offset from emitter position.
    float maxSpawnOffsetX = 0.0f;  /// Default 0.0f: maximum horizontal spawn offset from emitter position.
    float minSpawnOffsetY = 0.0f;  /// Default 0.0f: minimum vertical spawn offset from emitter position.
    float maxSpawnOffsetY = 0.0f;  /// Default 0.0f: maximum vertical spawn offset from emitter position.

    // Rotation //
    float minRotation = 0.0f;      /// Default 0.0f: minimum rotation speed in degrees/second.
    float maxRotation = 0.0f;      /// Default 0.0f: maximum rotation speed in degrees/second.

    // Gravity //
    bool hasGravity = true;        /// Default true: particles are affected by gravity.

    // Floor Kill Plane //
    bool hasFloor = false;         /// Default false: if true, particles die below spawnY + floorOffset.
    float floorOffset = 0.0f;      /// Default 0.0f: vertical offset from spawn position used as the floor kill height.

    // Wave Motion //
    float waveAmplitudeMin = 0.0f; /// Default 0.0f: minimum wave height in world units. Use 0.0f for no wave motion.
    float waveAmplitudeMax = 0.0f; /// Default 0.0f: maximum wave height in world units. Use 0.0f for no wave motion.
    float waveFrequencyMin = 0.0f; /// Default 0.0f: minimum wave frequency in cycles/second. Use 0.0f for no wave motion.
    float waveFrequencyMax = 0.0f; /// Default 0.0f: maximum wave frequency in cycles/second. Use 0.0f for no wave motion.
};

class _particleEngine {
    public:
        _particleEngine();
        virtual ~_particleEngine();

        void init(_textureManager* sceneTextureManager);

        void draw();
    protected:
    private:
        // Single instance particle struct //
        struct particle {
            bool alive = false;

            // Physics //
            Vec2f pos;              
            Vec2f vel;              
            Vec2f acc;
            float rotationSpeed;
            
            float radius;

            float death;    // When the particle will die (an offset from birth decides this)
            float age;      // Particles current age

            float angle;

            bool hasGravity;
        
            bool hasFloor;
            float floorPosY;

            float waveAmplitude;
            float waveFrequency;
            float waveOffset; // Randomly chosen

            int imageIndex;
        };

        // Injections //
        _textureManager* textureManager = nullptr;  // Pointer to scene texture manager (non-owning)
        _lightManager* lightManager = nullptr;      // Pointer to scene light manager (non-owning)

        // Data Structures //
};

#endif // _PARTICLE_ENGINE