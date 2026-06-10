#ifndef PARTICLE_ENGINE_H
#define PARTICLE_ENGINE_H

#include <_common.h>
#include <_textureManager.h>
#include <_lightManager.h>
#include <_shader.h>

#include <glm/glm.hpp>                      // Core library
#include <glm/gtc/matrix_transform.hpp>     // Matrix ops like transform, scale, ortho, etc
#include <glm/gtc/type_ptr.hpp>             // Send GLM datatypes (matrix) to GPU

namespace particles {
    /**
     * Particle Effect Configuration
     */
    struct Config {
        int particleCount = 0;              /// Required: number of particles to spawn.
    
        // Animation //
        std::string texturePath;            /// Required: particle sprite sheet image path.
        int sheetColumns = 0;               /// Required: number of frame columns in the sprite sheet. [1-255]
        int sheetRows = 0;                  /// Required: number of frame rows in the sprite sheet. [1-255]
    
        int animationRow = 0;               /// Default 0: row of the sprite sheet used by this effect. [0-255]
        int animationFPS = 0;               /// Default 0: animation frame rate in frames/second. Use 0 for static particles. [0-255]
        bool pingPongAnimation = false;     /// Default false: True means animation does not loop to beginning, it bounces
        bool deathOnAnimationEnd = false;   /// Default false: True means particle is destroyed once animation completes one full loop

        // Velocity //
        float minVelX = 0.0f;               /// Default 0.0f: minimum horizontal velocity.
        float maxVelX = 0.0f;               /// Default 0.0f: maximum horizontal velocity.
        float minVelY = 0.0f;               /// Default 0.0f: minimum vertical velocity.
        float maxVelY = 0.0f;               /// Default 0.0f: maximum vertical velocity.
    
        // Size //
        float minRadius = 0.0f;             /// Default 0.0f: minimum particle radius/size.
        float maxRadius = 0.0f;             /// Default 0.0f: maximum particle radius/size.
    
        // Lifetime //
        float minLifeTime = 0.0f;           /// Default 0.0f: minimum particle lifespan in seconds.
        float maxLifeTime = 0.0f;           /// Default 0.0f: maximum particle lifespan in seconds.
    
        // Spawn Offset //
        float minSpawnOffsetX = 0.0f;       /// Default 0.0f: minimum horizontal spawn offset from emitter position.
        float maxSpawnOffsetX = 0.0f;       /// Default 0.0f: maximum horizontal spawn offset from emitter position.
        float minSpawnOffsetY = 0.0f;       /// Default 0.0f: minimum vertical spawn offset from emitter position.
        float maxSpawnOffsetY = 0.0f;       /// Default 0.0f: maximum vertical spawn offset from emitter position.
    
        // Rotation //
        float minRotation = 0.0f;           /// Default 0.0f: minimum rotation speed in degrees/second.
        float maxRotation = 0.0f;           /// Default 0.0f: maximum rotation speed in degrees/second.
    
        // Gravity //
        bool hasGravity = true;             /// Default true: particles are affected by gravity.
    
        // Floor Kill Plane //
        bool hasFloor = false;              /// Default false: if true, particles die below spawnY + floorOffset.
        float floorOffset = 0.0f;           /// Default 0.0f: vertical offset from spawn position used as the floor kill height.
    
        // Wave Motion //
        float waveAmplitudeMin = 0.0f;      /// Default 0.0f: minimum wave height in world units. Use 0.0f for no wave motion.
        float waveAmplitudeMax = 0.0f;      /// Default 0.0f: maximum wave height in world units. Use 0.0f for no wave motion.
        float waveFrequencyMin = 0.0f;      /// Default 0.0f: minimum wave frequency in cycles/second. Use 0.0f for no wave motion.
        float waveFrequencyMax = 0.0f;      /// Default 0.0f: maximum wave frequency in cycles/second. Use 0.0f for no wave motion.
    };
    
    /**
     * Setup context containing required dependencies
     */
    struct Context {
        _textureManager* textureManager = nullptr;  
        _lightManager* lightManager = nullptr;       
    };

    class Engine {
        public:
            Engine();
            virtual ~Engine();

            /**
             * Initialization function
             * 
             * @param context Container of required dependencies
             */
            void init(const Context& context);

            /**
             * Draw function
             * 
             * @param viewProjectionMatrix Matrix for the View Projection
             */
            void draw(const glm::mat4 &viewProjectionMatrix);

            /**
             * Update function
             * 
             * @param dt Delta Time (in seconds)
             */
            void update(double dt);

            /**
             * Spawns a given effect based on a provided config
             * 
             * @param pos Position of where to spawn the effect
             * @param config Configuration for the effect
             */
            void spawnEffect(glm::vec2 pos, const std::string &ID);

            // Debugging function that outputs GPU memory usage to console
            void logGpuMemoryUsage() const;

            // Debugging function that outputs GPU memory usage to console
            void logCpuMemoryUsage() const;
        protected:
        private:
            // Single instance particle struct //
            struct Particle {
                glm::vec2 pos;              
                glm::vec2 vel;              
                glm::vec2 acc;
                
                float rotationSpeed;
                float radius;
                float floorPosY;
                float waveAmplitude;
                float waveFrequency;
                float waveOffset;       // Randomly chosen
                float death;            // When the particle will die (an offset from birth decides this)
                float age;              // Particles current age
                float angle;
                float animationTimer;   // Time since last animation frame occured (resets after exceeding)

                bool alive = false;
                bool hasGravity;
                bool hasFloor;
                bool pingPongAnimation;
                bool deathOnAnimationEnd;
                
                bool inReverseAnimation = false;
    
                uint8_t colIndex;
                uint8_t rowIndex;
                uint8_t fps;
            };

            // Injections //
            _textureManager* textureManager = nullptr;  // Pointer to scene texture manager (non-owning)
            _lightManager* lightManager = nullptr;      // Pointer to scene light manager (non-owning)
    
            // RNG //
            std::mt19937 rng;   

            // Data Structures //

            // Batch of particle data + image data
            struct ParticleBatch {
                std::string texturePath;
                GLuint textureID = 0;

                int sheetColumns = 1;
                int sheetRows = 1;

                std::vector<Particle> particles;

                // Pre-calculated values per layer for optimzation
                float c_uWidth;
                float c_vWidth;

                float aliveParticles;
            };

            // Helper Functions //
            void iterateAnimationFrameRegular(Particle &p, const ParticleBatch &pBatch);
            void iterateAnimationFramePingPong(Particle &p, const ParticleBatch &pBatch);

            std::vector<ParticleBatch> particleList;                      // Particles are batched by their texture in a list
            std::unordered_map<std::string, size_t> particleTable;        // Lookup table to particleList index (for spawn effect)
            std::unordered_map<std::string, Config> configTable;          // Lookup table of configs by a unique string ID (name)

            // -- Rendering -- //
            struct Vertex {
                float w;    // Width
                float h;    // Height
                float u;    // TexCoord X
                float v;    // TexCoord Y
                float x;    // Pos X
                float y;    // Pos Y
                float a;    // Angle    
                float w_a;  // Wave Amplitude
                float w_f;  // Wave Frequency
                float w_o;  // Wave Offset
            };

            static constexpr int FLOATS_PER_VERTEX = 10;
            static constexpr int VERTICIES_PER_PARTICLE = 4;
            static constexpr int INDICIES_PER_PARTICLE = 6;

            // Builds dynamic VBO (EBO/VAO static & built in init)
            void buildVBO(); 
            GLuint vboID = 0;
            GLuint eboID = 0;
            GLuint vaoID = 0;

            int totalAliveParticles = 0;
            static constexpr int MAX_PARTICLES = 10000;    // May be moved into config read at some point

            // Shaders //
            _shader particleShader;
            // Uniforms
            GLint u_viewProjectionMatrix = -1;
            GLint u_texture = -1;
            GLint u_t = -1;
            float t_value = 0.0f;   // Value of u_t (dt)
    };
}

#endif // PARTICLE_ENGINE_H