#include <_particles.h>

// -- STATIC -- //
bool _particles::textureLoaded = false;
_texture _particles::texture;


// -- PUBLIC -- //

_particles::_particles() : rng(random_device{}()) {
    // ctor
}

_particles::~_particles() {
    if (vboID != 0) {
        glDeleteBuffers(1,&vboID); // tell the GPU to delete the vertex buffer
        vboID = 0;
    }
    if (eboID != 0) {
        glDeleteBuffers(1,&eboID); // tell the GPU to delete the index buffer
        eboID = 0;
    }
}

void _particles::initParticles(const string &fileName, const Vec2f &pos) {
    if (!textureLoaded) {
        texture.loadTexture(fileName);
        textureLoaded = true;
    }

    glGenBuffers(1, &vboID); // Create a VBO buffer for particles
    glGenBuffers(1, &eboID); // Create a VBO buffer for particles

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

    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID);

    glVertexPointer(2, GL_FLOAT, 4 * sizeof(float), (void*)0);
    glTexCoordPointer(2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glDrawElements(GL_TRIANGLES, numDropsRendered * 6, GL_UNSIGNED_INT, (void*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

bool _particles::hasParticles() const {
    return isAlive;
}

// -- PRIVATE -- //

void _particles::buildParticleBuffer() {
    /**
     * Particles are rendered as a quad.
     * They require 4 vertices of 2 components of 2 floats each (x,y,u,v) 
     * 4 vertices * 2 components (pos/tex) * 2 floats each (x,y/u,v) for 2D
     */
    float particleVboData[MAX_DROPS * 4 * 2 * 2];
    int vIndex = 0;
    /**
     * The ebo is what we actually use to build the object, it holds indicies TO the vbo. 
     * However, it doesnt hold indicies to each variable (float) its to each vertex. Despite a quad being used for the particles,
     * we treat it as 2 triangles (quads are depricated in modern openGL), each triangle requires 3 verticies so 3*2 is 6 verticies per tile.
     */
    uint32_t particleEboData[MAX_DROPS * 6];
    int eIndex = 0;

    int vertexOffset = 0;
    
    numDropsRendered = 0;
    for (int i = 0; i < MAX_DROPS; i++) {
        particle* p = &drops[i];
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

        // Ebo (Two Triangles) //
        // Triangle 1
        particleEboData[eIndex++] = vertexOffset + 0; // BL   
        particleEboData[eIndex++] = vertexOffset + 1; // BR
        particleEboData[eIndex++] = vertexOffset + 2; // TR
        // Triangle 2
        particleEboData[eIndex++] = vertexOffset + 0; // BL
        particleEboData[eIndex++] = vertexOffset + 2; // TR
        particleEboData[eIndex++] = vertexOffset + 3; // TL

        vertexOffset += 4; // Offset the Vbo verticies (4)

        numDropsRendered++;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, vIndex * sizeof(float), particleVboData, GL_DYNAMIC_DRAW); // We only send the particles that are alive
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, eIndex * sizeof(uint32_t), particleEboData, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
}

