#ifndef _PARTICLES_H
#define _PARTICLES_H

#include <_common.h>
#include <_timerPlusPlus.h>
#include <_texture.h>

struct particle {
    bool alive = true;     // Life time

    // Physics //
    Vec2f pos = {0.0f,0.0f};              // Position
    Vec2f vel = {0.0f,0.0f};              // Velocity
    Vec2f acc = {0.0f,0.0f};              // Acceleration
    float mass = 0.0f;       // For gravity

    Col3f col;              // Color
    float opacity = 1.0f;   // Visibility
    
    float t = 0.0f;          // For parametric equations
    float radius = 0.0f;
    float expandRadius = 0.0f;
    float theta = 0.0f;
};

class _particles {
    public:
        _particles();
        virtual ~_particles();

        // Set image to render for particles
        void initParticles(const string &fileName);

        /**
         * Update loop for applying physics/movement
         * 
         * @param dt Delta Time (in seconds)
         */
        void updateParticles(double dt);

        // Main draw loop
        void drawParticles();

    protected:
    private:
        random_device eng;
        mt19937 rng;


        // Builds the VBO for draw renders
        void buildParticleBuffer();

        particle drops[MAX_DROPS];
        GLuint vboID = 0; 

        int numDrops = 0;
        int newDrops = 0;

        _timerPlusPlus* timer = new _timerPlusPlus();
        _texture* texture = new _texture();
};

#endif // _PARTICLES_H