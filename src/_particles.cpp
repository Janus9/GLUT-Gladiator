#include <_particles.h>

_particles::_particles() {
    // ctor
}

_particles::~_particles() {
    delete timer;
    timer = nullptr;

    delete texture;
    texture = nullptr;
}

void _particles::initParticles(const string &fileName) {
    texture->loadTexture(fileName);

    glGenBuffers(1, &vboID); // Create a VBO buffer for particles

    rng = mt19937(eng());

    uniform_real_distribution<float> vel_dist(-5.0f, 5.0f);
    uniform_real_distribution<float> radius_dist(1.0f, 4.0f);

    for(int i = 0; i < MAX_DROPS; i++) {
        particle* p = &drops[i];
        p->vel = {vel_dist(rng), vel_dist(rng)};
        p->radius = radius_dist(rng);
    }
}

void _particles::updateParticles(double dt) {
    for (int i = 0; i < MAX_DROPS; i++) {
        drops[i].acc.y = -GRAVITY * dt;
        drops[i].vel += drops[i].acc * dt;
        drops[i].pos += drops[i].vel * dt;
    }
}

void _particles::drawParticles() {
    buildParticleBuffer();
    texture->bindTexture();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glPushMatrix();
        glBindBuffer(GL_ARRAY_BUFFER, vboID);
        glVertexPointer(2, GL_FLOAT, 4 * sizeof(float), (void*)0);
        glTexCoordPointer(2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glDrawArrays(GL_QUADS, 0, MAX_DROPS * 4);
    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void _particles::buildParticleBuffer() {
    /**
     * MAX_DROP partics * 4 vertices * 2 floats + 4 textures * 2 floats
     */
    float particleVboData[MAX_DROPS * 4 * 4];
    int index = 0;

    for (int i = 0; i < MAX_DROPS; i++) {
        particle* p = &drops[i];
        if (!p->alive) continue; // Skips particles that are dead

        // Bottom-left
        particleVboData[index++] = p->pos.x; 
        particleVboData[index++] = p->pos.y; 
        particleVboData[index++] = 0; 
        particleVboData[index++] = 0; 
        // Bottom-right
        particleVboData[index++] = p->pos.x + p->radius; 
        particleVboData[index++] = p->pos.y; 
        particleVboData[index++] = 1; 
        particleVboData[index++] = 0; 
        // Top-right
        particleVboData[index++] = p->pos.x + p->radius; 
        particleVboData[index++] = p->pos.y + p->radius; 
        particleVboData[index++] = 1; 
        particleVboData[index++] = 1; 
        // Top-left
        particleVboData[index++] = p->pos.x; 
        particleVboData[index++] = p->pos.y + p->radius; 
        particleVboData[index++] = 0; 
        particleVboData[index++] = 1; 
    }

    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particleVboData), particleVboData, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER,0);
}

