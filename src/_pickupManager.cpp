#include <_pickupManager.h>

// -- STATIC MEMBERS -- //

glm::mat4 _pickupManager::viewProjectionMatrix;

void _pickupManager::setViewProjectionMatrix(const glm::mat4 &_viewProjectionMatrix) {
    viewProjectionMatrix = _viewProjectionMatrix;
}

// -- PUBLIC -- //

_pickupManager::_pickupManager() : rng(random_device{}()) {
    alivePickups = 0;
}

_pickupManager::~_pickupManager() {
    if (vboID != 0) {
        glDeleteBuffers(1,&vboID); 
        vboID = 0;
    }
    if (eboID != 0) {
        glDeleteBuffers(1,&eboID); 
        eboID = 0;
    }
    if (vaoID != 0) {
        glDeleteVertexArrays(1,&vaoID); 
        vaoID = 0;
    }
}

void _pickupManager::initPickupManager(const string& fileName, int imageWidth, _player* currentPlayer, _lightManager* currentLightManager) {
    player = currentPlayer;
    sceneLightManager = currentLightManager;
    numImages = imageWidth;

    if (!player || !sceneLightManager) {
        cout << "ERROR: Player or LightManager is nullptr\n";
        return;
    }
    
    textureLoader.loadTexture(fileName);

    // -- SHADER SETUP -- //

    pickupShader.initShader("shaders/pickup_manager/vertex.vs","shaders/pickup_manager/fragment.fs");
    uint32_t program = pickupShader.getProgram();

    sceneLightManager->addProgram(program);

    // Uniforms
    u_viewProjectionMatrix = glGetUniformLocation(program,"u_viewProjectionMatrix");
    u_texture = glGetUniformLocation(program,"u_texture");
    u_t = glGetUniformLocation(program,"u_t");

    glGenBuffers(1, &vboID); 
    glGenBuffers(1, &eboID); 
    glGenVertexArrays(1, &vaoID);

    pickupList.resize(maxPickups); 

    // 7 floats * 4 verticies * maxPickups * float size
    int maxSizeBytes = 7 * 4 * maxPickups * sizeof(float);

    glBindBuffer(GL_ARRAY_BUFFER,vboID);
    // This allocates memoery for the buffer but does NOT assign any data
    glBufferData(GL_ARRAY_BUFFER,maxSizeBytes,nullptr,GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    buildEBO();
    buildVAO();

    initialized = true;
}

void _pickupManager::drawPickups() {
    buildVBO();
    
    if (alivePickups <= 0) return; // No particles skip update loop
    
    glUseProgram(pickupShader.getProgram());
    
    textureLoader.bindTexture();

    // Setup uniforms
    glUniformMatrix4fv(u_viewProjectionMatrix, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
    glUniform1i(u_texture, 0); // Uses texture slot not ID thus its 0
    glUniform1f(u_t,t_value);

    sceneLightManager->applyLights(pickupShader.getProgram());

    glBindVertexArray(vaoID);
    glDrawElements(GL_TRIANGLES, alivePickups * 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}

void _pickupManager::updatePickups(const double dt) {
    t_value += dt;
    if (alivePickups == 0) return;

    for (int i = 0; i < maxPickups; i++) {
        _pickup& p = pickupList[i];
        if (!p.alive) continue;

        const float distance = p.pos.distance(player->pos); // Distance to player

        if (distance < 10.0f) {
            // Apply pickup
            p.alive = false;

            player->resupply(p.health, p.ammo);
            player->setMaxHealth(player->getMaxHealth() + p.maxHealth);
            player->movementSpeed += p.speed;
            player->fireRate += p.fireRate;
            // DO XP LATER //

            continue;
        }

        if (distance < 48.0f) {
            // Apply movement towards player
            const Vec2f direction = (player->pos - p.pos).normalized();
            const float speed = 48.0f / (distance); // Speed ramps up closer to player
            p.acc = (direction * speed * 50.0f);
        } else {
            // Player out of range
            p.acc = {0.0f,0.0f};
        }

        p.pos += p.vel * dt;
        p.vel += p.acc * dt;
    }
}

bool _pickupManager::addPickup(const Vec2f &pos, const pickup_config& config) {
    for (int i = 0; i < maxPickups; i++) {
        _pickup& p = pickupList[i];
        if (!p.alive) {
            p.alive = true;

            p.imageIndex = config.imageIndex;

            p.pos = pos;
            p.vel = {0.0f,0.0f};
            p.acc = {0.0f,0.0f};

            p.size = config.size;

            p.health = config.health;
            p.maxHealth = config.maxHealth;
            p.ammo = config.ammo;
            p.speed = config.speed;
            p.fireRate = config.fireRate;
            p.xp = config.xp;

            return true;
        }
    }

    return false; // Never found a free pickup (likely full)
}

// -- PRIVATE -- //

void _pickupManager::buildVBO() {
    float vboData[maxPickups * 10 * 4];
    int vIndex = 0;
    
    alivePickups = 0;
    for (int i = 0; i < maxPickups; i++) {
        const _pickup* p = &pickupList[i];
        if (!p->alive) continue; 

        const float angle = 0.0f;  // No angle -- kept in case we need it later

        const float centerX = p->pos.x;
        const float centerY = p->pos.y;

        // Can change to width/height later
        const float halfWidth = p->size * 0.5f;
        const float halfHeight = p->size * 0.5f;

        if (numImages == 0) {
            // No images cant build VBO (protect against divide by 0)
            return;
        }
        const float uWidth = 1.0f / numImages;
        
        // Vbo (Quad) //
        // Bottom-left (0)
        vboData[vIndex++] = -halfWidth; 
        vboData[vIndex++] = -halfHeight; 
        vboData[vIndex++] = p->imageIndex * uWidth; 
        vboData[vIndex++] = 1.0f; 
        vboData[vIndex++] = centerX; 
        vboData[vIndex++] = centerY; 
        vboData[vIndex++] = angle; 
        // Bottom-right (1)
        vboData[vIndex++] = halfWidth; 
        vboData[vIndex++] = -halfHeight; 
        vboData[vIndex++] = (p->imageIndex + 1) * uWidth; 
        vboData[vIndex++] = 1.0f; 
        vboData[vIndex++] = centerX; 
        vboData[vIndex++] = centerY; 
        vboData[vIndex++] = angle; 
        // Top-right (2)
        vboData[vIndex++] = halfWidth; 
        vboData[vIndex++] = halfHeight; 
        vboData[vIndex++] = (p->imageIndex + 1) * uWidth; 
        vboData[vIndex++] = 0.0f; 
        vboData[vIndex++] = centerX; 
        vboData[vIndex++] = centerY; 
        vboData[vIndex++] = angle; 
        // Top-left (3)
        vboData[vIndex++] = -halfWidth; 
        vboData[vIndex++] = halfHeight; 
        vboData[vIndex++] = p->imageIndex * uWidth; 
        vboData[vIndex++] = 0.0f; 
        vboData[vIndex++] = centerX; 
        vboData[vIndex++] = centerY; 
        vboData[vIndex++] = angle; 

        alivePickups++;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferSubData(GL_ARRAY_BUFFER,0,vIndex * sizeof(float),vboData);  
    glBindBuffer(GL_ARRAY_BUFFER,0);
}

void _pickupManager::buildEBO() {
    uint32_t eboData[maxPickups * 6];
    int vertexOffset = 0;
    int eIndex = 0;
    for (int i = 0; i < maxPickups; i++) {
        // Ebo (Two Triangles) //
        // Triangle 1
        eboData[eIndex++] = vertexOffset + 0; // BL   
        eboData[eIndex++] = vertexOffset + 1; // BR
        eboData[eIndex++] = vertexOffset + 2; // TR
        // Triangle 2
        eboData[eIndex++] = vertexOffset + 0; // BL
        eboData[eIndex++] = vertexOffset + 2; // TR
        eboData[eIndex++] = vertexOffset + 3; // TL

        vertexOffset += 4;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(eboData), eboData, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
}

void _pickupManager::buildVAO() {
    glBindVertexArray(vaoID);
    
    glBindBuffer(GL_ARRAY_BUFFER,vboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,eboID);
    
    const GLsizei stride = 7 * sizeof(float);

    // Size
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,stride,(void*)(0 * sizeof(float)));
    
    // Texture Coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,stride,(void*)(2 * sizeof(float)));
    
    // Center Position
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,stride,(void*)(4 * sizeof(float)));
    
    // Angle
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,stride,(void*)(6 * sizeof(float)));
    
    glBindVertexArray(0);
}