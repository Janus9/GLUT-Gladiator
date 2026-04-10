#include <_particles.h>

// -- PUBLIC -- //

_particles::_particles() : rng(random_device{}()) {
    // ctor
}

_particles::~_particles() {
    // dtor
}

void _particles::initParticles(const string &fileName, const Vec2f &pos) {
    texture.loadTexture(fileName);

    glGenBuffers(1, &vboID); // Create a VBO buffer for particles

    uniform_real_distribution<float> vel_dist(-10.0f, 10.0f);
    uniform_real_distribution<float> radius_dist(1.0f, 3.0f);
    uniform_real_distribution<float> lifeTime_dist(1.5f, 4.0f);

    for(int i = 0; i < MAX_DROPS; i++) {
        particle* p = &drops[i];
        p->pos = pos;
        p->alive = true;
        p->vel = {vel_dist(rng), vel_dist(rng)};
        p->radius = radius_dist(rng);
        p->lifeTime = lifeTime_dist(rng);
    }

    timer.reset();
}

void _particles::updateParticles(double dt) {
    bool hasDrops = false;
    for (int i = 0; i < MAX_DROPS; i++) {
        particle* p = &drops[i];

        if (timer.getSeconds() > p->lifeTime) p->alive = false;
        if (!p->alive) continue;

        p->acc.y = -GRAVITY * 16 * dt;
        p->vel += p->acc * dt;
        p->pos += p->vel * dt;

        hasDrops = true; // Any particle found keeps class alive
    }
    isAlive = hasDrops; // Will be false when loop stops running (all particles dead)
}

void _particles::drawParticles() {
    buildParticleBuffer();
    texture.bindTexture();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glPushMatrix();
        glBindBuffer(GL_ARRAY_BUFFER, vboID);
        glVertexPointer(2, GL_FLOAT, 4 * sizeof(float), (void*)0);
        glTexCoordPointer(2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glDrawArrays(GL_TRIANGLES, 0, numDropsRendered * 6);
    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

bool _particles::hasParticles() const {
    return isAlive;
}

// -- PRIVATE -- //

void _particles::buildParticleBuffer() {
    /**
     * Total * primatives * verticies per primative * floats per vertex
     * MAX_DROP * 2 * 3 * 4
     */
    float particleVboData[MAX_DROPS * 2 * 3 * 4];
    int index = 0;
    
    numDropsRendered = 0;
    for (int i = 0; i < MAX_DROPS; i++) {
        particle* p = &drops[i];
        if (!p->alive) continue; // Skips particles that are dead

        float x = p->pos.x;
        float y = p->pos.y;
        float r = p->radius;
        
        // Triangle 1
        // Bottom-left
        particleVboData[index++] = x; 
        particleVboData[index++] = y; 
        particleVboData[index++] = 0; 
        particleVboData[index++] = 1; 
        // Bottom-right
        particleVboData[index++] = x + r; 
        particleVboData[index++] = y; 
        particleVboData[index++] = 1; 
        particleVboData[index++] = 1; 
        // Top-right
        particleVboData[index++] = x + r; 
        particleVboData[index++] = y + r; 
        particleVboData[index++] = 1; 
        particleVboData[index++] = 0; 

        // Triangle 2
        // Bottom-left
        particleVboData[index++] = x; 
        particleVboData[index++] = y; 
        particleVboData[index++] = 0; 
        particleVboData[index++] = 1; 
        // Top-right
        particleVboData[index++] = x + r; 
        particleVboData[index++] = y + r; 
        particleVboData[index++] = 1; 
        particleVboData[index++] = 0; 
        // Top-left
        particleVboData[index++] = x; 
        particleVboData[index++] = y + r; 
        particleVboData[index++] = 0; 
        particleVboData[index++] = 0; 

        numDropsRendered++;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, index * sizeof(float), particleVboData, GL_DYNAMIC_DRAW); // We only send the particles that are alive
    glBindBuffer(GL_ARRAY_BUFFER,0);
}

