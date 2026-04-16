#include <_particleManager.h>

// -- PUBLIC -- //

_particleManager::_particleManager() : rng(random_device{}()) {
    // ctor
}

_particleManager::~_particleManager() {
    if (vboID != 0) {
        glDeleteBuffers(1,&vboID); // tell the GPU to delete the vertex buffer
        vboID = 0;
    }
    if (eboID != 0) {
        glDeleteBuffers(1,&eboID); // tell the GPU to delete the index buffer
        eboID = 0;
    }

    delete texture;
    texture = nullptr;

    delete timer;
    timer = nullptr;
}

void _particleManager::initParticleManager(const string& fileName, int _maxParticles) {
    maxParticles = _maxParticles;
    texture->loadTexture(fileName);

    glGenBuffers(1, &vboID); // Create a VBO buffer for particles
    glGenBuffers(1, &eboID); // Create a VBO buffer for particles

    particles.resize(maxParticles); // Set particles up in vector

    buildEBO();
}

void _particleManager::drawParticleManager() {
    buildVBO();
    texture->bindTexture();

    glColor3f(1.0f,1.0f,1.0f); // Reset color

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID);

    glVertexPointer(2, GL_FLOAT, 4 * sizeof(float), (void*)0);
    glTexCoordPointer(2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glDrawElements(GL_TRIANGLES, numParticlesInUse * 6, GL_UNSIGNED_INT, (void*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void _particleManager::updateParticleManger(double dt) {
    for (int i = 0; i < maxParticles; i++) {
        particle* p = &particles[i];
        
        p->age += dt; // Add delta time seconds to age
        if (p->age >= p->death) {
            p->alive = false;
            continue;
        }

        p->acc.x = 0.0f;
        p->acc.y = -GRAVITY * 4;

        p->vel += p->acc * dt;
        p->pos += p->vel * dt;
    }
}

void _particleManager::spawnEffect(const Vec2f &pos, const particle_effect &effect) {
    uniform_real_distribution<float> vel_x_dist(effect.minVelX, effect.maxVelX);
    uniform_real_distribution<float> vel_y_dist(effect.minVelY, effect.maxVelY);

    uniform_real_distribution<float> radius_dist(effect.minRadius, effect.maxRadius);

    uniform_real_distribution<float> lifeTime_dist(effect.minLifeTime, effect.maxLifeTime);

    uniform_real_distribution<float> offset_x_dist(effect.minSpawnOffsetX, effect.maxSpawnOffsetX);
    uniform_real_distribution<float> offset_y_dist(effect.minSpawnOffsetY, effect.maxSpawnOffsetY);

    int currentParticles = 0;
    for (int i = 0; i < maxParticles; i++) {
        if (currentParticles >= effect.amount) break;

        particle *p = &particles[i];
        if (!p->alive) {
            currentParticles++;
            p->pos = pos;

            p->pos.x += offset_x_dist(rng);
            p->pos.y += offset_y_dist(rng);

            p->vel.x = vel_x_dist(rng);
            p->vel.y = vel_y_dist(rng);
            
            p->radius = radius_dist(rng);
            
            p->age = 0.0f;
            p->death = lifeTime_dist(rng);

            p->alive = true;
        }
    }
}

// -- PRIVATE -- //

void _particleManager::buildVBO() {
    /**
     * Particles are rendered as a quad.
     * They require 4 vertices of 2 components of 2 floats each (x,y,u,v) 
     * 4 vertices * 2 components (pos/tex) * 2 floats each (x,y/u,v) for 2D
     */
    float particleVboData[maxParticles * 4 * 2 * 2];
    int vIndex = 0;
    
    numParticlesInUse = 0;
    for (int i = 0; i < maxParticles; i++) {
        particle* p = &particles[i];
        if (!p->alive) continue; // Skips particles that are dead

        float x = p->pos.x;
        float y = p->pos.y;
        float r = p->radius;
        
        // Vbo (Quad) //
        // Bottom-left (0)
        particleVboData[vIndex++] = x; 
        particleVboData[vIndex++] = y; 
        particleVboData[vIndex++] = 0.0f; 
        particleVboData[vIndex++] = 1.0f; 
        // Bottom-right (1)
        particleVboData[vIndex++] = x + r; 
        particleVboData[vIndex++] = y; 
        particleVboData[vIndex++] = 1.0f; 
        particleVboData[vIndex++] = 1.0f; 
        // Top-right (2)
        particleVboData[vIndex++] = x + r; 
        particleVboData[vIndex++] = y + r; 
        particleVboData[vIndex++] = 1.0f; 
        particleVboData[vIndex++] = 0.0f; 
        // Top-left (3)
        particleVboData[vIndex++] = x; 
        particleVboData[vIndex++] = y + r; 
        particleVboData[vIndex++] = 0.0f; 
        particleVboData[vIndex++] = 0.0f; 

        numParticlesInUse++;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, vIndex * sizeof(float), particleVboData, GL_DYNAMIC_DRAW); // We only send the particles that are alive
    glBindBuffer(GL_ARRAY_BUFFER,0);
}

void _particleManager::buildEBO() {
    /**
     * The ebo is what we actually use to build the object, it holds indicies TO the vbo. 
     * However, it doesnt hold indicies to each variable (float) its to each vertex. Despite a quad being used for the particles,
     * we treat it as 2 triangles (quads are depricated in modern openGL), each triangle requires 3 verticies so 3*2 is 6 verticies per tile.
     */
    uint32_t particleEboData[maxParticles * 6];
    int vertexOffset = 0;
    int eIndex = 0;
    for (int i = 0; i < maxParticles; i++) {
        // Ebo (Two Triangles) //
        // Triangle 1
        particleEboData[eIndex++] = vertexOffset + 0; // BL   
        particleEboData[eIndex++] = vertexOffset + 1; // BR
        particleEboData[eIndex++] = vertexOffset + 2; // TR
        // Triangle 2
        particleEboData[eIndex++] = vertexOffset + 0; // BL
        particleEboData[eIndex++] = vertexOffset + 2; // TR
        particleEboData[eIndex++] = vertexOffset + 3; // TL

        vertexOffset += 4;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(particleEboData), particleEboData, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

    eboBuilt = true;
}