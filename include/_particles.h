#ifndef _PARTICLES_H
#define _PARTICLES_H

#include <_common.h>
#include <_timerPlusPlus.h>
#include <_texture.h>

struct particle {
    bool alive = false;

    // Physics //
    Vec2f pos = {0.0f,0.0f};              
    Vec2f vel = {0.0f,0.0f};              
    Vec2f acc = {0.0f,0.0f};
    
    float radius = 0.0f;

    float lifeTime = 0.0f; // Seconds
};

class _particles {
    public:
        _particles();
        virtual ~_particles();

        // Set image to render for particles
        void initParticles(const string &fileName, const Vec2f &pos);

        /**
         * Update loop for applying physics/movement
         * 
         * @param dt Delta Time (in seconds)
         */
        void updateParticles(double dt);

        // Main draw loop
        void drawParticles();

        // Returns True if particles are still being rendered
        bool hasParticles() const;

    protected:
    private:
        mt19937 rng;

        // Builds the VBO for draw renders
        void buildParticleBuffer();

        particle drops[MAX_DROPS];
        GLuint vboID = 0; 

        int numDrops = 0;
        int newDrops = 0;

        _timerPlusPlus timer;

        static _texture texture;
        static bool textureLoaded;

        int numDropsRendered = 0; // Number of drops actually being rendered

        bool isAlive = true; // Whether the class has any particles left to render 
};

#endif // _PARTICLES_H