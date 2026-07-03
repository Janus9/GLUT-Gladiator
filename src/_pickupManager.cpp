#include <_pickupManager.h>

#include <_enemyManager.h>

// -- STATIC MEMBERS -- //

glm::mat4 _pickupManager::viewProjectionMatrix;
Vec2f _pickupManager::cameraPosition;

void _pickupManager::setViewProjectionMatrix(const glm::mat4 &_viewProjectionMatrix) {
    viewProjectionMatrix = _viewProjectionMatrix;
}

void _pickupManager::setCameraPosition(const Vec2f &_cameraPosition) {
    cameraPosition = _cameraPosition;
}

// -- PUBLIC -- //

_pickupManager::_pickupManager() : rng(std::random_device{}()) {
    alivePickups = 0;

    writeCompleted.store(false);
    writeInProgress.store(false);

    readBuffer = &dataBuffer1;
    writeBuffer = &dataBuffer2;
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
    // Destroy write thread
    if (writeThread.joinable()) {
        writeThread.join();
    }
}

void _pickupManager::initPickupManager(
    const std::string& fileName, 
    int imageWidth, 
    _player* currentPlayer, 
    _lightManager* currentLightManager,
    _world* currentWorld
) {
    SDL_LogInfo(LOG_PICKUPS, "Initializing the pickup manager");
    
    player = currentPlayer;
    world = currentWorld;
    sceneLightManager = currentLightManager;
    numImages = imageWidth;

    if (!player || !sceneLightManager || !world) {
        SDL_LogError(LOG_PICKUPS, "ERROR: Player or LightManager is nullptr");
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
    u_t = glGetUniformLocation(program,"u_time");

    glGenBuffers(1, &vboID); 
    glGenBuffers(1, &eboID); 
    glGenVertexArrays(1, &vaoID);

    // pickupList.resize(maxPickups); 

    // 7 floats * 4 verticies * maxPickups * float size
    int maxSizeBytes = 7 * 4 * MAX_RENDER_PICKUPS * sizeof(float);

    glBindBuffer(GL_ARRAY_BUFFER,vboID);
    // This allocates memory for the buffer but does NOT assign any data
    glBufferData(GL_ARRAY_BUFFER,maxSizeBytes,nullptr,GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    buildEBO();
    buildVAO();

    SDL_LogInfo(LOG_PICKUPS, "Successfully initialized the pickup manager");
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
    if (prevWritePos.distance(cameraPosition) > VIEW_RANGE*0.5f) {
        SDL_LogDebug(LOG_PICKUPS, "Camera moved too far from previous write position, reloading pickups!");
        readFromFile();
    }

    if (writeInProgress.load() && writeCompleted.load()) {
        if (writeThread.joinable()) {
            writeThread.join();
            // Not done in thread to prevent race conditions
            std::swap(writeBuffer, readBuffer); // Swap the buffers so that the buffer we wrote into (write buffer) becomes the one we read (read buffer)
        } else {
            SDL_LogWarn(LOG_PICKUPS, "WARNING: Cannot join thread as it is not joinable");
        }
        writeInProgress.store(false);
        writeCompleted.store(false);
    }

    t_value += dt;
    if (alivePickups == 0) return;

    for (int i = 0; i < readBuffer->size(); i++) {
        _pickup& p = (*readBuffer)[i];
        if (!p.alive) continue;

        const float distance = p.pos.distance(player->pos); // Distance to player

        if (distance < 10.0f) {
            // Apply pickup
            p.alive = false;
            // Player variables
            switch (p.type) {
                case PICKUP_HEALTH:
                    player->addHealth(p.value);
                    break;
                case PICKUP_AMMO:
                    player->addAmmo(p.value);
                    break;
                case PICKUP_SPEED:
                    player->addSpeed(p.value);
                    break;
                case PICKUP_MAX_HEALTH:
                    player->addMaxHealth(p.value);
                    break;
                case PICKUP_XP:
                    player->addXP(p.value);    
                    break;
                case PICKUP_FIRERATE:
                    player->addFireRate(p.value);    
                    break;
                default:
                    SDL_LogError(LOG_PICKUPS, "ERROR: Could not determine pickup type");
                    break;
            }
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

// NEEDS TO BE REDONE //
bool _pickupManager::addPickup(const Vec2f &pos, pickup_type type, float value) {
    SDL_LogWarn(LOG_PICKUPS, "WARNING: Function depricated!");
    for (int i = 0; i < MAX_RENDER_PICKUPS; i++) {
        _pickup& p = (*readBuffer)[i];
        if (!p.alive) {
            p.alive = true;

            p.type = type;

            p.pos = pos;
            p.vel = {0.0f,0.0f};
            p.acc = {0.0f,0.0f};

            p.value = value;

            p.size = 4.0f + log(value);

            return true;
        }
    }

    return false; // Never found a free pickup (likely full)
}

bool _pickupManager::generateToFile(const world_config &config) {
    SDL_LogInfo(LOG_PICKUPS, "Generating pickups into save file");

    std::fstream file("saves/game.gg_world", std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
        SDL_LogError(LOG_PICKUPS, "ERROR: Cannot open the save file");
        return false;
    }

    // -- MOVE READ HEAD -- //
    moveHeadToData(file);

    // -- READ PICKUP AMOUNT -- //
    const uint32_t num_hp_pickups = static_cast<uint32_t>(config.num_chunks * config.health_pickups.pickups_per_chunk);
    const uint32_t num_ammo_pickups = static_cast<uint32_t>(config.num_chunks * config.ammo_pickups.pickups_per_chunk);
    const uint32_t num_speed_pickups = static_cast<uint32_t>(config.num_chunks * config.speed_pickups.pickups_per_chunk);
    const uint32_t num_max_hp_pickups = static_cast<uint32_t>(config.num_chunks * config.max_health_pickups.pickups_per_chunk);
    const uint32_t num_firerate_pickups = static_cast<uint32_t>(config.num_chunks * config.firerate_pickups.pickups_per_chunk);

    const uint32_t pickup_count = num_hp_pickups + num_ammo_pickups + num_speed_pickups + num_max_hp_pickups + num_firerate_pickups;
    file.write(reinterpret_cast<const char*>(&pickup_count),sizeof(pickup_count)); // Pickup Count

    SDL_LogDebug(LOG_PICKUPS, "Pickup generation count: %u", pickup_count);

    // -- WRITE PICKUPS -- //

    SDL_LogDebug(LOG_PICKUPS, "Staring write at position: 0x%llX", static_cast<long long>(file.tellp()));

    generatePickup(file, config.health_pickups, config.num_chunks, PICKUP_HEALTH);           // Health
    generatePickup(file, config.max_health_pickups, config.num_chunks, PICKUP_MAX_HEALTH);   // Max Health
    generatePickup(file, config.ammo_pickups, config.num_chunks, PICKUP_AMMO);               // Ammo
    generatePickup(file, config.speed_pickups, config.num_chunks, PICKUP_SPEED);             // Speed
    generatePickup(file, config.firerate_pickups, config.num_chunks, PICKUP_FIRERATE);       // Fire rate

    SDL_LogDebug(LOG_PICKUPS, "Final position: 0x%llX", static_cast<long long>(file.tellp()));
    
    SDL_LogInfo(LOG_PICKUPS, "Successfully generated pickups into save file");
    
    return true;
}

bool _pickupManager::readFromFile() {
    SDL_LogInfo(LOG_PICKUPS, "Command given to read from file");

    if (writeInProgress.load() || writeThread.joinable()) {
        SDL_LogWarn(LOG_PICKUPS, "WARNING: Thread already working, skipping command");
        return true;
    }

    writeInProgress.store(true);
    writeThread = std::thread(&_pickupManager::writeToBuffer, this);

    prevWritePos = cameraPosition;
    
    return true;
}

std::vector<pickup_serial_data> _pickupManager::exportSerializedPickups() const {
    SDL_LogWarn(LOG_PICKUPS, "WARNING: Function depricated!");
    std::vector<pickup_serial_data> data;
    for (const _pickup &p : (*readBuffer)) {
        if (!p.alive) continue; // Skip dead pickups
        data.push_back(serializePickup(p));
    }
    return data;
}

bool _pickupManager::importSerializedPickups(const std::vector<pickup_serial_data> &pickup_data) {
    SDL_LogWarn(LOG_PICKUPS, "WARNING: Function depricated!");
    if (pickup_data.empty()) {
        SDL_LogWarn(LOG_PICKUPS, "WARNING: Cannot import pickups as the data is empty");
        return false;
    }
    for (const pickup_serial_data &p : pickup_data) {
        if (!addPickup({p.xPos, p.yPos}, static_cast<pickup_type>(p.type), p.value)) {
            SDL_LogError(LOG_PICKUPS, "ERROR: Cannot add pickup");
        }
    }
    return true;
}

// -- PRIVATE -- //

void _pickupManager::writeToBuffer() {
    SDL_LogInfo(LOG_PICKUPS, "Thread: Reading pickups from save file");

    std::fstream file("saves/game.gg_world", std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
        SDL_LogError(LOG_PICKUPS, "Thread: ERROR: Cannot open the save file");
        return;
    }

    // -- VARIABLES -- //
    constexpr int BUFFER_SIZE = 4096;

    // -- MOVE READ HEAD -- //
    moveHeadToData(file);

    uint32_t pickup_count = 0;
    file.read(reinterpret_cast<char*>(&pickup_count), sizeof(pickup_count));  // Enemy Count

    SDL_LogDebug(LOG_PICKUPS, "Thread: Pickups in save: %u", pickup_count);
    if (pickup_count == 0) {
        SDL_LogWarn(LOG_PICKUPS, "Thread: WARNING: Pickups found is 0");
    }

    if (!writeBuffer) {
        SDL_LogError(LOG_PICKUPS, "Thread: ERROR: Cannot write to the buffer as it is nullptr");
        return;
    }

    int pickups = static_cast<int>(pickup_count);
    writeBuffer->clear();
    // writeBuffer->resize(pickups);

    while (pickups > 0) {
        std::vector<pickup_serial_data> buffer(std::clamp(pickups, 0, BUFFER_SIZE));
        file.read(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(pickup_serial_data));

        for (const auto &p : buffer) {
            pickups--;
            
            if (Vec2f(p.xPos,p.yPos).distance(cameraPosition) > VIEW_RANGE) continue; // Skip, out of range

            writeBuffer->emplace_back();
            _pickup &pickup = (*writeBuffer)[writeBuffer->size()-1];
            pickup.pos.x = p.xPos;
            pickup.pos.y = p.yPos;
            pickup.vel = { 0.0f, 0.0f };
            pickup.acc = { 0.0f, 0.0f };
            pickup.type = static_cast<pickup_type>(p.type);
            pickup.size = 4.0f + log(p.value);
            pickup.value = p.value;
            pickup.alive = true;
        }
    }

    SDL_LogDebug(LOG_PICKUPS, "Thread: Read %zu pickups", writeBuffer->size());

    writeCompleted.store(true);
    SDL_LogInfo(LOG_PICKUPS, "Thread: Successfully loaded pickups from save file");
}

pickup_serial_data _pickupManager::serializePickup(const _pickup &pickup) const {
    pickup_serial_data data {
        data.value = pickup.value,
        data.type = static_cast<int32_t>(pickup.type),
        data.xPos = pickup.pos.x,
        data.yPos = pickup.pos.y,
    };
    return data;
}

void _pickupManager::moveHeadToData(std::fstream &head) {
    // Skip Header, Seed, Time Stamp, Save System Version ID, Game Version
    head.seekg(static_cast<std::streamoff>(2 + 4 + 8 + 4 + 4), std::ios::cur);

    int32_t chunk_count = 0;
    head.read(reinterpret_cast<char*>(&chunk_count), sizeof(chunk_count));  // Chunk Count
    // Skip the Chunk Data Header
    head.seekg(4, std::ios::cur);
    // Skip all the chunks
    head.seekg(static_cast<std::streamoff>(chunk_count * sizeof(chunk_serial_data)), std::ios::cur);
    
    // Skip the Enemy Data Header
    head.seekg(4, std::ios::cur);
    int32_t enemy_count = 0;
    head.read(reinterpret_cast<char*>(&enemy_count), sizeof(enemy_count));  // Enemy Count
    // Skip all enemies
    head.seekg(static_cast<std::streamoff>(enemy_count * sizeof(enemy_serial_data)), std::ios::cur);
    
    // Skip the Pickup Data Header
    head.seekg(4, std::ios::cur);
}

bool _pickupManager::generatePickup(std::fstream &file, const pickup_config &config, float numChunks, pickup_type type) {
    SDL_LogInfo(LOG_PICKUPS, "Generating pickups instance");

    // -- VARIABLES -- //
    const float WORLD_RADIUS = sqrt(numChunks) * NUM_TILES_CHUNK_SQR * TILE_D * 0.5f;
    const float WORLD_DIAMETER = sqrt(numChunks) * NUM_TILES_CHUNK_SQR * TILE_D;

    // Only does pickup health for now
    constexpr int BUFFER_SIZE = 4096;
    
    const float max_distance = Vec2f(WORLD_RADIUS,WORLD_RADIUS).distance({0.0f, 0.0f});
    int pickups = static_cast<int>(numChunks * config.pickups_per_chunk);
    
    SDL_LogDebug(LOG_PICKUPS, "Pickups to generate: %i", pickups);

    std::uniform_real_distribution<float> pickup_hp_dist(config.near_bound, config.far_bound);
    std::uniform_real_distribution<float> rad_rng(0.0f, 2.0f * PI);
    std::uniform_real_distribution<float> pickup_rng(0.0f, 1.0f);
    
    while (pickups > 0) {
        std::vector<pickup_serial_data> buffer(std::clamp(pickups, 0, BUFFER_SIZE));
        
        for (size_t i = 0; i < buffer.size(); i++) {
            bool lookingForSpawn = true;
            while (lookingForSpawn) {
                const float radius = WORLD_RADIUS * pickup_hp_dist(rng);   
                const float theta = rad_rng(rng);
                const Vec2f pos = {radius * std::cosf(theta), radius * std::sinf(theta)};

                const _cell *cell = world->getCellAtWorld(pos);
                
                if (cell && world->isCellWall(cell)) continue;

                const float dist = pos.distance({0.0f,0.0f});
                const float dist_norm = std::clamp(dist / max_distance, 0.0f, 1.0f);
                
                const float t = std::clamp((dist_norm - config.near_bound) / (config.far_bound - config.near_bound), 0.0f, 1.0f);

                const float chance = std::lerp(1.0, config.min_chance, t);
                
                if (chance > pickup_rng(rng)) {
                    buffer[i].value = 10.0f;
                    buffer[i].type = type;
                    buffer[i].xPos = pos.x;
                    buffer[i].yPos = pos.y;

                    lookingForSpawn = false;
                    pickups--;
                }
            }
        }

        file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size() * sizeof(pickup_serial_data)); // Pickup Data
        if (!file) {
            SDL_LogError(LOG_PICKUPS, "ERROR: Failed to write the pickup data");
            return false;
        }
    }
    SDL_LogInfo(LOG_PICKUPS, "Finished generating pickups instance");
    return true;
}


void _pickupManager::buildVBO() {
    std::vector<float> vbo(MAX_RENDER_PICKUPS * 7 * 4);
    int vIndex = 0;
    
    alivePickups = 0;
    for (int i = 0; i < std::clamp(static_cast<int>(readBuffer->size()), 0, MAX_RENDER_PICKUPS); i++) {
        const _pickup* p = &(*readBuffer)[i];
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
        vbo[vIndex++] = -halfWidth; 
        vbo[vIndex++] = -halfHeight; 
        vbo[vIndex++] = p->type * uWidth; 
        vbo[vIndex++] = 1.0f; 
        vbo[vIndex++] = centerX; 
        vbo[vIndex++] = centerY; 
        vbo[vIndex++] = angle; 
        // Bottom-right (1)
        vbo[vIndex++] = halfWidth; 
        vbo[vIndex++] = -halfHeight; 
        vbo[vIndex++] = (p->type + 1) * uWidth; 
        vbo[vIndex++] = 1.0f; 
        vbo[vIndex++] = centerX; 
        vbo[vIndex++] = centerY; 
        vbo[vIndex++] = angle; 
        // Top-right (2)
        vbo[vIndex++] = halfWidth; 
        vbo[vIndex++] = halfHeight; 
        vbo[vIndex++] = (p->type + 1) * uWidth; 
        vbo[vIndex++] = 0.0f; 
        vbo[vIndex++] = centerX; 
        vbo[vIndex++] = centerY; 
        vbo[vIndex++] = angle; 
        // Top-left (3)
        vbo[vIndex++] = -halfWidth; 
        vbo[vIndex++] = halfHeight; 
        vbo[vIndex++] = p->type * uWidth; 
        vbo[vIndex++] = 0.0f; 
        vbo[vIndex++] = centerX; 
        vbo[vIndex++] = centerY; 
        vbo[vIndex++] = angle; 

        alivePickups++;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferSubData(GL_ARRAY_BUFFER,0,vIndex * sizeof(float),vbo.data());  
    glBindBuffer(GL_ARRAY_BUFFER,0);
}

void _pickupManager::buildEBO() {
    uint32_t eboData[MAX_RENDER_PICKUPS * 6];
    int vertexOffset = 0;
    int eIndex = 0;
    for (int i = 0; i < MAX_RENDER_PICKUPS; i++) {
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