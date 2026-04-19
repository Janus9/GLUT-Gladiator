#include <_bulletManager.h>
#include <_enemyManager.h>

// -- STATIC MEMBERS -- //

float _bulletManager::left = 0.0f;
float _bulletManager::right = 0.0f;
float _bulletManager::top = 0.0f;
float _bulletManager::bottom = 0.0f;

void _bulletManager::setViewportDimensions(float _left, float _right, float _top, float _bottom) {
    left = _left;
    right = _right;
    top = _top;
    bottom = _bottom;
}    

// -- PUBLIC -- //

_bulletManager::_bulletManager() : rng(random_device{}()) {
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
    if (vaoID != 0) {
        glDeleteVertexArrays(1,&vaoID); // tell the GPU to delete the array buffer
        vaoID = 0;
    }

    delete texture;
    texture = nullptr;

    delete bulletDrops;
    bulletDrops = nullptr;

    // no timers
}

void _bulletManager::initBulletManager(const string &fileName, _world* currentWorld, _player* currentPlayer, _enemyManager* currentEnemyManager) {
    // Generate new buffers
    glGenBuffers(1,&vboID);
    glGenBuffers(1,&eboID);
    glGenVertexArrays(1,&vaoID);

    texture->loadTexture(fileName);

    world = currentWorld;
    player = currentPlayer;
    enemyManager = currentEnemyManager;
    
    // -- SHADER SETUP -- //
    bulletShader.initShader("shaders/bullet_manager/vertex.vs","shaders/bullet_manager/fragment.fs");

    uint32_t program = bulletShader.getProgram();
    
    // Uniforms
    u_dimensions = glGetUniformLocation(program,"u_dimensions");
    u_texture = glGetUniformLocation(program,"u_texture");

    bulletDrops->initParticleManager("images/bullet_casing.png",1000);
    bullet_shell_effect.amount = 1;

    bullet_shell_effect.minVelX = 5.5f;
    bullet_shell_effect.maxVelX = 10.5f;
    bullet_shell_effect.minVelY = 15.0f;
    bullet_shell_effect.maxVelY = 28.0f;

    bullet_shell_effect.minRadius = 2.0f;
    bullet_shell_effect.maxRadius = 2.0f;

    bullet_shell_effect.minLifeTime = 0.5f;
    bullet_shell_effect.maxLifeTime = 1.1f;

    // 7 entries per vertex, 4 vertex per primitive (quad), MAX_BULLETS amount, ~4 bytes per float (may change)
    int maxSizeBytes = 7 * 4 * MAX_BULLETS * sizeof(float);

    glBindBuffer(GL_ARRAY_BUFFER,vboID);
    // This allocates memoery for the buffer but does NOT assign any data
    glBufferData(GL_ARRAY_BUFFER,maxSizeBytes,nullptr,GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    buildEbo();
    buildVao();
} 

void _bulletManager::drawBulletManager() {
    // Build buffers and send data into the GPU
    buildVbo();

    if (aliveBullets <= 0) {
        // Skip drawing if now bullets, but particle manager still needs to run
        bulletDrops->drawParticleManager();
        return;
    }

    bulletShader.useProgram();

    texture->bindTexture();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // Setup uniforms
    glUniform4f(u_dimensions,left,right,top,bottom);
    glUniform1i(u_texture, 0); // Uses texture slot not ID thus its 0

    glBindVertexArray(vaoID);
    glDrawElements(GL_TRIANGLES, aliveBullets * 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0); // Stop using program (prevent using on bullet particle drops)

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    bulletDrops->drawParticleManager();
}

void _bulletManager::updateBulletManager(double dt) {
    bulletDrops->updateParticleManger(dt);

    if (aliveBullets <= 0) return; // Skip update loops when no bullets are alive
    for (int i = 0; i < MAX_BULLETS; i++) {
        _bullet* b = &bulletPool[i];
        if (!b->alive) continue;
        
        b->age += dt;
        if (b->age >= b->lifespan) {
            // Bullet outlived lifespan - kill
            b->alive = false;
            continue;
        }
        // Physics
        b->pos += b->vel * dt;
        
        // World Collision Checks
        _cell* occupyingCell = world->getCellAtWorld(b->pos);
        if (occupyingCell) {
            if (world->isTileWall(occupyingCell->tileId)) {
                // Collision event
                world->damageCell(occupyingCell,10.0f);
                b->alive = false;
                continue;
            }
        }

        if (b->team == _team::FRIENDLY || b->team == _team::NEUTRAL) {
            // Enemy Collision Check
            _enemy* enemy = enemyManager->isColliding(b->pos,5.0f); 
            if (enemy) {
                enemy->health -= 10.0f;
                b->alive = false;
                continue;
            }
        } else {
            // Friendly Collision Check
            if (b->pos.distance(player->pos) < 5.0f) {
                player->health -= 10.0f;
                b->alive = false;
                continue;
            }
        }
    }
}

void _bulletManager::spawnBulletEffect(const Vec2f &pos, const Vec2f &dest, _team bulletTeam, const _bullet_config &config) {
    int currentBullets = 0;

    uniform_real_distribution<float> angle_dist(-config.angleOffset, config.angleOffset);

    float angleOffset = degreeToRad(angle_dist(rng)); // Radians

    Vec2f dir = (dest-pos).normalized(); // Direction vector

    float sinA = sin(angleOffset);
    float cosA = cos(angleOffset);

    /**
     * [cos -sin][x]
     * [sin  cos][y]
     * 
     * Formula for 2D rotations
     */
    Vec2f newDir = {dir.x * cosA - dir.y * sinA, dir.x * sinA + dir.y * cosA};

    Vec2f vel = {newDir.x * config.speed, newDir.y * config.speed};

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
        b->angle = atan2(b->vel.y, b->vel.x); // Gets angle bullet must rotate for its velocity 
        b->lifespan = config.lifespan;
        b->age = 0.0f;
        b->team = bulletTeam;

        currentBullets++;
    }

    bulletDrops->spawnEffect(pos,bullet_shell_effect);
}

// -- PRIVATE -- //

void _bulletManager::buildVbo() {
    /**
     * VBO Structure -> localX, localY, u, v, centerX, centerY, angle
     * localX - X component of the quad's position 
     * localY - Y component of the quad's position 
     * u - X component of the quad's texture 
     * v - Y component of the quad's texture 
     * centerX - X component of bullet's position
     * centerY - Y component of bullets's position
     * angle - Angle bullet is facing 
     * (7 floats)
     * 
     * Before, we sent up posX/posY for the x,y components meaning the CPU decided the drawing position.
     * Here, the shader uses a fixed localX and localY and applies centerX, centerY, and angle onto it to modify it's drawing position INSIDE the GPU
     * 
     * Bullets are rendered as a quad (2 triangles) and require 4 verticies. 
     * 
     * 4 * 7 * MAX_BULLETS
     * 
     */
    float bulletVboData[4 * 7 * MAX_BULLETS];
    int vIndex = 0;
    
    aliveBullets = 0;
    for (int i = 0; i < MAX_BULLETS; i++) {
        _bullet* b = &bulletPool[i];
        if (!b->alive) continue; // Skips particles that are dead

        float angle = b->angle;

        float centerX = b->pos.x;
        float centerY = b->pos.y;

        float halfWidth = b->width * 0.5f;
        float halfHeight = b->height * 0.5f;
        
        // Vbo (Quad) //
        // Bottom-left (0)
        bulletVboData[vIndex++] = -halfWidth; 
        bulletVboData[vIndex++] = -halfHeight; 
        bulletVboData[vIndex++] = 0.0f; 
        bulletVboData[vIndex++] = 1.0f; 
        bulletVboData[vIndex++] = centerX; 
        bulletVboData[vIndex++] = centerY; 
        bulletVboData[vIndex++] = angle; 
        // Bottom-right (1)
        bulletVboData[vIndex++] = halfWidth; 
        bulletVboData[vIndex++] = -halfHeight; 
        bulletVboData[vIndex++] = 1.0f; 
        bulletVboData[vIndex++] = 1.0f; 
        bulletVboData[vIndex++] = centerX; 
        bulletVboData[vIndex++] = centerY; 
        bulletVboData[vIndex++] = angle; 
        // Top-right (2)
        bulletVboData[vIndex++] = halfWidth; 
        bulletVboData[vIndex++] = halfHeight; 
        bulletVboData[vIndex++] = 1.0f; 
        bulletVboData[vIndex++] = 0.0f; 
        bulletVboData[vIndex++] = centerX; 
        bulletVboData[vIndex++] = centerY; 
        bulletVboData[vIndex++] = angle; 
        // Top-left (3)
        bulletVboData[vIndex++] = -halfWidth; 
        bulletVboData[vIndex++] = halfHeight; 
        bulletVboData[vIndex++] = 0.0f; 
        bulletVboData[vIndex++] = 0.0f; 
        bulletVboData[vIndex++] = centerX; 
        bulletVboData[vIndex++] = centerY; 
        bulletVboData[vIndex++] = angle; 

        aliveBullets++;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferSubData(GL_ARRAY_BUFFER,0,vIndex * sizeof(float),bulletVboData); // We only send the particles that are alive
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

void _bulletManager::buildVao() {
    // Bind the vertex array
    glBindVertexArray(vaoID);
    
    // Bind the buffers (setup before attributes since they read from the VBO)
    glBindBuffer(GL_ARRAY_BUFFER,vboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,eboID);

    // Enable the attribute arrays (not setup with data yet, just enabled)
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    GLsizei stride = 7 * sizeof(float);

    /**
     * void glVertexAttribPointer 	
     * ---------------------------
     * GLuint index          - Attribute to be modified (like a cache)
     * GLint size            - Number of components in the attribute (ex/ vec2 is 2)
  	 * GLenum type           - Type of data (ex/ float is GL_FLOAT)
  	 * GLboolean normalized  - ??? just set to GL_FALSE
  	 * GLsizei stride        - How much data per vertex (here its 7 floats so 7 * sizeof(float))
  	 * const void * pointer  - Offset of data for first component to start at (ex/ localPos is 0 since its first in the VBO structure setup)
     * 
     */

    // Setup a data array for the attributes (data is per vertex!)
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,stride,(void*)(0 * sizeof(float)));
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,stride,(void*)(2 * sizeof(float)));
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,stride,(void*)(4 * sizeof(float)));
    glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,stride,(void*)(6 * sizeof(float)));

    glBindVertexArray(0);
}
