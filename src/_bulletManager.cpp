#include <_bulletManager.h>

// -- PUBLIC -- //

_bulletManager::_bulletManager() {
    // ctor
}

_bulletManager::~_bulletManager() {
    world = nullptr; // Safe prevent weird dangling pointer
    
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

    delete bulletDrops;
    bulletDrops = nullptr;

    // no timers
}

void _bulletManager::initBulletManager(const string &fileName, _world* currentWorld) {
    // Generate new buffers
    glGenBuffers(1,&vboID);
    glGenBuffers(1,&eboID);

    texture->loadTexture(fileName);

    world = currentWorld;

    bulletDrops->initParticleManager("images/test_bullet.png",1000);
    bullet_shell_effect.amount = 1;

    bullet_shell_effect.minVelX = 0.5f;
    bullet_shell_effect.maxVelX = 1.5f;
    bullet_shell_effect.minVelY = 6.0f;
    bullet_shell_effect.maxVelY = 12.0f;

    bullet_shell_effect.minRadius = 2.0f;
    bullet_shell_effect.maxRadius = 2.0f;

    bullet_shell_effect.minLifeTime = 2.5f;
    bullet_shell_effect.maxLifeTime = 3.0f;

    buildEbo();
} 

void _bulletManager::drawBulletManager() {
    buildVbo();
    texture->bindTexture();

    glColor3f(1.0f,1.0f,0.0f); // Yellow bullet

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID);

    glVertexPointer(2, GL_FLOAT, 4 * sizeof(float), (void*)0);
    glTexCoordPointer(2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glDrawElements(GL_TRIANGLES, aliveBullets * 6, GL_UNSIGNED_INT, (void*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    bulletDrops->drawParticleManager();
}

void _bulletManager::updateBulletManager(double dt) {
    // Has really odd collision with tiles

    bulletDrops->updateParticleManger(dt);

    if (aliveBullets <= 0) return; // Skip update loops when no bullets are alive
    for (int i = 0; i < MAX_BULLETS; i++) {
        _bullet* b = &bulletPool[i];
        b->age += dt;
        if (b->age >= b->lifespan) {
            // Bullet outlived lifespan - kill
            b->alive = false;
            continue;
        }
        
        // Physics
        b->pos += b->vel * dt;

        _cell* occupyingCell = world->getCellAtWorld(b->pos);
        if (occupyingCell) {
            if (world->isTileWall(occupyingCell->tileId)) {
                // Collision event
                world->damageCell(occupyingCell,10.0f);
                b->alive = false;
                continue;
            }
        }
    }
}

void _bulletManager::spawnBulletEffect(const Vec2f &pos, const Vec2f &dest, const _bullet_config &config) {
    int currentBullets = 0;

    float distance = pos.distance(dest); // Distance between source and destination
    float xComponent = (dest.x - pos.x) / distance; // Component [0-1] 
    float yComponent = (dest.y - pos.y) / distance; // Component [0-1]

    Vec2f vel = {xComponent * config.speed, yComponent * config.speed};

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (currentBullets >= config.amount) break; // Found enough bullets

        _bullet* b = &bulletPool[i];
        if (b->alive) continue; // Skip alive bullets

        // Setup bullet
        b->alive = true;
        b->pos = pos;
        b->vel = vel;
        b->width = config.width;
        b->height = config.height;
        b->lifespan = config.lifespan;
        b->age = 0.0f;

        currentBullets++;
    }

    bulletDrops->spawnEffect(pos,bullet_shell_effect);
}

// -- PRIVATE -- //

void _bulletManager::buildVbo() {
    /**
     * Particles are rendered as a quad.
     * They require 4 vertices of 2 components of 2 floats each (x,y,u,v) 
     * 4 vertices * 2 components (pos/tex) * 2 floats each (x,y/u,v) for 2D
     * 
     * Texture coords added but unused (for now)
     */
    float particleVboData[MAX_BULLETS * 4 * 2 * 2];
    int vIndex = 0;
    
    aliveBullets = 0;
    for (int i = 0; i < MAX_BULLETS; i++) {
        _bullet* b = &bulletPool[i];
        if (!b->alive) continue; // Skips particles that are dead

        float x = b->pos.x;
        float y = b->pos.y;

        float w = b->width;
        float h = b->height;
        
        // Vbo (Quad) //
        // Bottom-left (0)
        particleVboData[vIndex++] = x; 
        particleVboData[vIndex++] = y; 
        particleVboData[vIndex++] = 0.0f; 
        particleVboData[vIndex++] = 1.0f; 
        // Bottom-right (1)
        particleVboData[vIndex++] = x + w; 
        particleVboData[vIndex++] = y; 
        particleVboData[vIndex++] = 1.0f; 
        particleVboData[vIndex++] = 1.0f; 
        // Top-right (2)
        particleVboData[vIndex++] = x + w; 
        particleVboData[vIndex++] = y + h; 
        particleVboData[vIndex++] = 1.0f; 
        particleVboData[vIndex++] = 0.0f; 
        // Top-left (3)
        particleVboData[vIndex++] = x; 
        particleVboData[vIndex++] = y + h; 
        particleVboData[vIndex++] = 0.0f; 
        particleVboData[vIndex++] = 0.0f; 

        aliveBullets++;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, vIndex * sizeof(float), particleVboData, GL_DYNAMIC_DRAW); // We only send the particles that are alive
    glBindBuffer(GL_ARRAY_BUFFER,0);
}

void _bulletManager::buildEbo() {
    /**
     * The ebo is what we actually use to build the object, it holds indicies TO the vbo. 
     * However, it doesnt hold indicies to each variable (float) its to each vertex. Despite a quad being used for the particles,
     * we treat it as 2 triangles (quads are depricated in modern openGL), each triangle requires 3 verticies so 3*2 is 6 verticies per tile.
     */
    uint32_t particleEboData[MAX_BULLETS * 6];
    int vertexOffset = 0;
    int eIndex = 0;
    for (int i = 0; i < MAX_BULLETS; i++) {
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
}