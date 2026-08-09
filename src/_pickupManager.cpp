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

    writeBufferCompleted.store(false);
    writeBufferInProgress.store(false);

    writeDiskCompleted.store(false);
    writeDiskInProgress.store(false);

    nextID.store(0u);

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
    // Destroy write buffer thread
    if (writeBufferThread.joinable()) {
        writeBufferThread.join();
    }
    // Destroy write disk thread
    if (writeDiskThread.joinable()) {
        writeDiskThread.join();
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
    // Save Pickups Timer //
    if (pickupSaveElapsedTime >= PICKUP_SAVE_INTERVAL) {
        pickupSaveElapsedTime = 0.0f;
        if (!writeToFile()) {
            SDL_LogError(LOG_PICKUPS, "ERROR: Unable to save pickups to disk");
        }
    }

    // Reload Pickups Distance Check //
    if (prevWritePos.distance(cameraPosition) > VIEW_RANGE*0.5f) {
        SDL_LogDebug(LOG_PICKUPS, "Camera moved too far from previous write position, reloading pickups!");
        readFromFile();
    }

    t_value += dt;
    if (alivePickups == 0) return;

    for (size_t i = 0; i < readBuffer->size(); i++) {
        _pickup& p = (*readBuffer)[i];
        if (!p.alive) continue;

        const float distance = p.pos.distance(player->pos); // Distance to player

        if (distance < 10.0f) {
            // Apply pickup
            p.alive = false;
            std::lock_guard<std::mutex> lock(m_mm); // Race condition with Apply Mutations
            mutationMap[p.id] = p;  // Add to mutation map
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

        p.vel += p.acc * dt;
        constexpr float TOLERANCE = 0.0005f; // Ignore if smaller than this
        if (p.vel.x > TOLERANCE || p.vel.x < -TOLERANCE || p.vel.y > TOLERANCE || p.vel.y < -TOLERANCE) {
            p.pos += p.vel * dt;

            // Only apply into mutation list when velocity 
            std::lock_guard<std::mutex> lock(m_mm); // Race condition with Apply Mutations
            mutationMap[p.id] = p;  // Add to mutation map
        }
    }
    pickupSaveElapsedTime += dt;
}

void _pickupManager::updateBackground() {
    // Write Buffer Join //
    if (writeBufferInProgress.load() && writeBufferCompleted.load()) {
        if (writeBufferThread.joinable()) {
            writeBufferThread.join();
            // Not done in thread to prevent race conditions
            std::swap(writeBuffer, readBuffer); // Swap the buffers so that the buffer we wrote into (write buffer) becomes the one we read (read buffer)
            SDL_LogDebug(LOG_PICKUPS, "Write Buffer Thread joined");
        } else {
            SDL_LogWarn(LOG_PICKUPS, "WARNING: Cannot join Write Buffer Thread as it is not joinable");
        }
        writeBufferInProgress.store(false);
        writeBufferCompleted.store(false);
    }

    // Write Disk Join //
    if (writeDiskInProgress.load() && writeDiskCompleted.load()) {
        if (writeDiskThread.joinable()) {
            writeDiskThread.join();
            SDL_LogDebug(LOG_PICKUPS, "Write Disk Thread joined");
        } else {
            SDL_LogWarn(LOG_PICKUPS, "WARNING: Cannot join Write Disk Thread as it is not joinable");
        }

        writeDiskInProgress.store(false);
        writeDiskCompleted.store(false);
    }

    // Clean Disk Join //
    if (cleanDiskInProgress.load() && cleanDiskCompleted.load()) {
        if (cleanDiskThread.joinable()) {
            cleanDiskThread.join();
            SDL_LogDebug(LOG_PICKUPS, "Clean Disk Thread joined");
        } else {
            SDL_LogWarn(LOG_PICKUPS, "WARNING: Cannot join Clean Disk Thread as it is not joinable");
        }

        cleanDiskInProgress.store(false);
        cleanDiskCompleted.store(false);
    }
}

bool _pickupManager::addPickup(const Vec2f &pos, pickup_type type, float value) {
    const uint32_t id = nextID.load();

    std::lock_guard<std::mutex> lock(m_mm);

    if (mutationMap.contains(id)) {
        SDL_LogError(LOG_PICKUPS, "[addPickup] :: Error cannot add ID: %u as it already exists" , id);
        return false;
    }

    const _pickup p = {
      .id = id,
      .pos = pos,
      .vel = {0.0f, 0.0f},
      .acc = {0.0f, 0.0f},
      .type = type,
      .size = 4.0f + log(value),
      .value = value,
      .alive = true   
    };

    mutationMap[id] = p;
    readBuffer->push_back(p);   // Add to read buffer so user immediately sees it. Next writeToDisk call will save it in disk

    nextID.store(id + 1);
    SDL_LogDebug(LOG_PICKUPS, "[addPickup] :: NextID: %u", nextID.load());

    return true;
}

bool _pickupManager::generateToFile(const world_config &config) {
    const std::string saveDir = std::string(SAVE_DIRECTORY + global::saveFileName + PICKUPS_EXTENSION);

    SDL_LogInfo(LOG_PICKUPS, "Generating pickups for save: %s", saveDir.c_str());

    std::fstream file(saveDir, std::ios::binary | std::ios::out);
    if (!file) {
        SDL_LogError(LOG_PICKUPS, "ERROR: Cannot create save file %s", saveDir.c_str());
        return false;
    }

    // -- WRITE HEADERS -- //

    constexpr char meta_header[2] = {'G','G'};
    file.write(meta_header,2); 
    if (!file) {
        SDL_LogError(LOG_PICKUPS, "ERROR: Could not create meta header for save file: %s", saveDir.c_str());
        return false;
    }

    constexpr char pickup_header[4] = {'P','K','U','P'};
    file.write(pickup_header,4); 
    if (!file) {
        SDL_LogError(LOG_PICKUPS, "ERROR: Could not create data header for save file: %s", saveDir.c_str());
        return false;
    }

    // -- PICKUP AMOUNT -- //
    int ID = 0;

    const uint32_t num_hp_pickups = static_cast<uint32_t>(config.num_chunks * config.health_pickups.pickups_per_chunk);
    const uint32_t num_ammo_pickups = static_cast<uint32_t>(config.num_chunks * config.ammo_pickups.pickups_per_chunk);
    const uint32_t num_speed_pickups = static_cast<uint32_t>(config.num_chunks * config.speed_pickups.pickups_per_chunk);
    const uint32_t num_max_hp_pickups = static_cast<uint32_t>(config.num_chunks * config.max_health_pickups.pickups_per_chunk);
    const uint32_t num_firerate_pickups = static_cast<uint32_t>(config.num_chunks * config.firerate_pickups.pickups_per_chunk);

    const uint32_t pickup_count = num_hp_pickups + num_ammo_pickups + num_speed_pickups + num_max_hp_pickups + num_firerate_pickups;
    file.write(reinterpret_cast<const char*>(&pickup_count),sizeof(pickup_count)); 
    if (!file) {
        SDL_LogError(LOG_PICKUPS, "ERROR: Could not create write count for save file: %s", saveDir.c_str());
        return false;
    }

    SDL_LogDebug(LOG_PICKUPS, "Pickup generation count: %u", pickup_count);

    // -- WRITE PICKUPS -- //

    SDL_LogDebug(LOG_PICKUPS, "Staring write at position: 0x%llX", static_cast<long long>(file.tellp()));

    generatePickup(file, config.max_health_pickups, config.num_chunks, PICKUP_MAX_HEALTH, ID);   // Max Health
    generatePickup(file, config.health_pickups, config.num_chunks, PICKUP_HEALTH, ID);           // Health
    generatePickup(file, config.ammo_pickups, config.num_chunks, PICKUP_AMMO, ID);               // Ammo
    generatePickup(file, config.speed_pickups, config.num_chunks, PICKUP_SPEED, ID);             // Speed
    generatePickup(file, config.firerate_pickups, config.num_chunks, PICKUP_FIRERATE, ID);       // Fire rate

    SDL_LogDebug(LOG_PICKUPS, "Final position: 0x%llX", static_cast<long long>(file.tellp()));
    
    file.close();

    SDL_LogInfo(LOG_PICKUPS, "Successfully generated pickups into save file");
    
    return true;
}

bool _pickupManager::readFromFile() {
    SDL_LogInfo(LOG_PICKUPS, "Command given to read from file");

    if (writeBufferInProgress.load() || writeBufferThread.joinable()) {
        SDL_LogWarn(LOG_PICKUPS, "WARNING: Write Buffer Thread already working, skipping command");
        return true;
    }
    if (writeDiskInProgress.load() || writeDiskThread.joinable()) {
        SDL_LogWarn(LOG_PICKUPS, "WARNING: Write Disk Thread already working, must wait until done, skipping command");
        return true;
    }
    if (cleanDiskInProgress.load() || cleanDiskThread.joinable()) {
        SDL_LogWarn(LOG_PICKUPS, "WARNING: Clean Disk Thread already working, must wait until done, skipping command");
        return true;
    }

    writeBufferInProgress.store(true);
    writeBufferThread = std::thread(&_pickupManager::writeToBuffer, this);

    prevWritePos = cameraPosition;

    return true;
}

bool _pickupManager::writeToFile() {
    SDL_LogInfo(LOG_PICKUPS, "Command given to write to file");
    if (writeDiskInProgress.load() || writeDiskThread.joinable()) {
        SDL_LogWarn(LOG_PICKUPS, "WARNING: Write Disk Thread already working, skipping command");
        return true;
    }
    if (writeBufferInProgress.load() || writeBufferThread.joinable()) {
        SDL_LogWarn(LOG_PICKUPS, "WARNING: Write Buffer Thread already working, must wait until done, skipping command");
        return true;
    }
    if (cleanDiskInProgress.load() || cleanDiskThread.joinable()) {
        SDL_LogWarn(LOG_PICKUPS, "WARNING: Clean Disk Thread already working, must wait until done, skipping command");
        return true;
    }
    if (mutationMap.empty()) {
        SDL_LogInfo(LOG_PICKUPS, "Mutation Map empty, skipping command");
        return true;
    }

    writeDiskInProgress.store(true);
    writeDiskThread = std::thread(&_pickupManager::emptyMutationMap, this);

    return true;
}

void _pickupManager::cleanDeadFromFileAsync() {
    SDL_LogInfo(LOG_PICKUPS, "Command given to remove dead pickups from file");

    if (cleanDiskInProgress.load() || cleanDiskThread.joinable()) {
        SDL_LogWarn(LOG_PICKUPS, "WARNING: Clean Disk Thread already working, skipping command");
        return;
    }
    if (writeDiskInProgress.load() || writeDiskThread.joinable()) {
        SDL_LogWarn(LOG_PICKUPS, "WARNING: Write Disk Thread already working, must wait until done, skipping command");
        return;
    }

    cleanDiskInProgress.store(true);
    cleanDiskThread = std::thread(&_pickupManager::cleanDeadFromFileWorker, this);
}

void _pickupManager::logDisk() const {
    SDL_LogInfo(LOG_PICKUPS, "Creating a log output at [logs/pickup.log]");

    const std::string saveDir = std::string(SAVE_DIRECTORY + global::saveFileName + PICKUPS_EXTENSION);
    std::fstream file(saveDir, std::ios::binary | std::ios::in);
    if (!file) {
        SDL_LogError(LOG_PICKUPS, "ERROR: Cannot open the save file: %s", saveDir.c_str());
        return;
    }

    std::fstream log("logs/pickup.log", std::ios::out);
    if (!log) {
        SDL_LogError(LOG_PICKUPS, "ERROR: Unable to create log file");
        return;
    }
    // -- VARIABLES -- //
    constexpr int BUFFER_SIZE = 4096;

    uint32_t pickup_count = 0;
    if (!verifyFile(file, pickup_count)) {
        SDL_LogError(LOG_PICKUPS, "ERROR: Cannot verify the save file");
        return;
    }

    std::streampos startPos = file.tellp();

    log << "File: " << saveDir << "\n";
    log << "Pickups: " << pickup_count << "\n";
    log << "Memory Stride: " << sizeof(pickup_serial_data) << "B" << "\n";
    log << "Memory Start: 0x" << std::hex << startPos << std::dec << "\n";
    log << "------------------\n\n";

    int pickups = static_cast<int>(pickup_count);

    while (pickups > 0) {
        std::vector<pickup_serial_data> buffer(std::clamp(pickups, 0, BUFFER_SIZE));
        file.read(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(pickup_serial_data));

        std::streamsize bytesRead = file.gcount();
        int pickupsRead = static_cast<int>(bytesRead / sizeof(pickup_serial_data));
        
        if (pickupsRead == 0) {
            SDL_LogError(LOG_PICKUPS, "ERROR: Reached end of file before reading all pickups");
            break;
        }

        for (int i = 0; i < pickupsRead; i++) {
            const auto &p = buffer[i];
            pickups--;
            
            std::streampos pos = startPos + static_cast<std::streamoff>(sizeof(pickup_serial_data) * p.id);
            log << "ID: " << p.id << " [0x" << std::uppercase << std::hex << pos << std::dec << std::nouppercase << "]\n"
                << "Value: " << p.value << "\n"
                << "Type: " << p.type << "\n"
                << "Pos X: " << p.xPos << "\n"
                << "Pos Y: " << p.yPos << "\n"
                << "Alive: " << p.alive << "\n";
            log << "------------\n";
        }
    }

    log.close();
    
    SDL_LogInfo(LOG_PICKUPS, "Finished creating a log output at [logs/pickup.log]");
}

// -- PRIVATE -- //

void _pickupManager::writeToBuffer() {
    SDL_LogInfo(LOG_PICKUPS, "[Write Buffer Thread]: Reading pickups from save file");

    auto start = std::chrono::steady_clock::now();

    const std::string saveDir = std::string(SAVE_DIRECTORY + global::saveFileName + PICKUPS_EXTENSION);
    std::fstream file(saveDir, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
        SDL_LogError(LOG_PICKUPS, "[Write Buffer Thread]: ERROR: Cannot open the save file: %s", saveDir.c_str());
        writeBufferCompleted.store(true);
        return;
    }

    // -- VARIABLES -- //
    constexpr int BUFFER_SIZE = 4096;

    uint32_t pickup_count = 0;
    if (!verifyFile(file, pickup_count)) {
        SDL_LogError(LOG_PICKUPS, "ERROR: Cannot verify the save file");
        return;
    }

    SDL_LogDebug(LOG_PICKUPS, "[Write Buffer Thread]: Pickups in save: %u", pickup_count);
    if (pickup_count == 0) {
        SDL_LogWarn(LOG_PICKUPS, "[Write Buffer Thread]: WARNING: Pickups found is 0");
    }

    if (!writeBuffer) {
        SDL_LogError(LOG_PICKUPS, "[Write Buffer Thread]: ERROR: Cannot write to the buffer as it is nullptr");
        writeBufferCompleted.store(true);
        return;
    }

    int pickups = static_cast<int>(pickup_count);
    writeBuffer->clear();

    uint32_t maxID = 0; 

    while (pickups > 0) {
        std::vector<pickup_serial_data> buffer(std::clamp(pickups, 0, BUFFER_SIZE));
        file.read(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(pickup_serial_data));

        // Add all pickups from mutation map
        {
            std::lock_guard<std::mutex> lock(m_mm);
            for (const auto &it : mutationMap) {
                writeBuffer->emplace_back();
                _pickup &pickup = (*writeBuffer)[writeBuffer->size()-1];
                pickup = it.second;
            }
        }

        for (const auto &p : buffer) {
            pickups--;

            if (p.id > maxID) maxID = p.id;

            // Look for pickup in mutation map BEFORE reading it in disk
            std::lock_guard<std::mutex> lock(m_mm);
            if (mutationMap.contains(p.id)) continue; // Already added above -- skip
            
            if (Vec2f(p.xPos,p.yPos).distance(cameraPosition) > VIEW_RANGE) continue; // Skip, out of range
            if (!p.alive) continue; // Skip writing dead pickups in memory

            writeBuffer->emplace_back();
            _pickup &pickup = (*writeBuffer)[writeBuffer->size()-1];

            pickup.pos.x = p.xPos;
            pickup.pos.y = p.yPos;
            pickup.vel = { 0.0f, 0.0f };
            pickup.acc = { 0.0f, 0.0f };
            pickup.type = static_cast<pickup_type>(p.type);
            pickup.size = 4.0f + log(p.value);
            pickup.value = p.value;
            pickup.id = p.id;
            pickup.alive = true;
        }
    }

    SDL_LogDebug(LOG_PICKUPS, "[Write Buffer Thread]: Max ID found: %u", maxID);
    if (maxID + 1 > nextID) nextID.store(maxID + 1);
    SDL_LogDebug(LOG_PICKUPS, "[Write Buffer Thread]: Next ID: %u", nextID.load());

    SDL_LogDebug(LOG_PICKUPS, "[Write Buffer Thread]: Read %llu pickups", writeBuffer->size());

    writeBufferCompleted.store(true);

    auto stop = std::chrono::steady_clock::now();
    const float d = std::chrono::duration<float, std::milli>(stop-start).count();
    SDL_LogDebug(LOG_PICKUPS, "[Write Buffer Thread]: Write To Buffer took [%fms]",d);
    SDL_LogDebug(LOG_PICKUPS, "[Write Buffer Thread]: Pickups in Mutation Map: %llu", mutationMap.size());

    SDL_LogInfo(LOG_PICKUPS, "[Write Buffer Thread]: Successfully loaded pickups from save file");
}

void _pickupManager::emptyMutationMap() {
    auto start = std::chrono::steady_clock::now();
    
    const std::string saveDir = std::string(SAVE_DIRECTORY + global::saveFileName + PICKUPS_EXTENSION);
    std::fstream file(saveDir, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
        SDL_LogError(LOG_PICKUPS, "[Disk Write Thread]: ERROR: Cannot open the save file: %s", saveDir.c_str());
        writeDiskCompleted.store(true);
        return;
    }

    // -- VARIABLES -- //
    constexpr int BUFFER_SIZE = 4096;

    uint32_t pickup_count = 0;
    if (!verifyFile(file, pickup_count)) {
        SDL_LogError(LOG_PICKUPS, "[Disk Write Thread]: ERROR: Cannot verify the save file: %s", saveDir.c_str());
        writeDiskCompleted.store(true);
        return;
    }

    std::streampos pickupCountPos = file.tellp() - static_cast<std::streamoff>(4); // 4 bytes to get back to beginning of where count is

    SDL_LogDebug(LOG_PICKUPS, "[Disk Write Thread]: Pickups in save: %u", pickup_count);
    if (pickup_count == 0) {
        SDL_LogWarn(LOG_PICKUPS, "[Disk Write Thread]: WARNING: Pickups found is 0");
    }

    int pickups = static_cast<int>(pickup_count);
    int mutatedPickups = 0;

    // Process snapshot 
    std::unordered_map<uint32_t, _pickup> snapshot;
    {
        std::lock_guard<std::mutex> lock(m_mm);
        std::swap(mutationMap, snapshot);
    }

    // EXISTING PICKUPS //

    while (pickups > 0) {
        std::streampos startPos = file.tellp();
        SDL_LogDebug(
            LOG_PICKUPS, 
            "[Disk Write Thread]: Disk Start: 0x%llX", 
            static_cast<unsigned long long>(static_cast<std::streamoff>(startPos))
        );

        std::vector<pickup_serial_data> buffer(std::clamp(pickups, 0, BUFFER_SIZE));
        file.read(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(pickup_serial_data));
        for (size_t i = 0; i < buffer.size(); i++) {
            pickup_serial_data &p = buffer[i];
            
            auto it = snapshot.find(p.id);
            if (it != snapshot.end()) {
                mutatedPickups++;
                // Modify to match whats in mutation map
                p = serializePickup(it->second);
                snapshot.erase(it); // Remove from snapshot to process remainder at end
            }
        }
        pickups -= static_cast<int>(buffer.size());

        file.seekp(startPos); // Move back to beginning of buffer to write it
        // Write modified buffer back in
        file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size() * sizeof(pickup_serial_data)); 
        if (!file) {
            SDL_LogError(LOG_PICKUPS, "[Disk Write Thread]: ERROR: Failed to write the pickup buffer data");
            writeDiskCompleted.store(true);
            return;
        }
    }
    SDL_LogDebug(LOG_PICKUPS, "[Disk Write Thread]: Mutated pickups processed: %i", mutatedPickups);

    // NEW PICKUPS //

    const size_t newPickups = snapshot.size();
    std::vector<pickup_serial_data> buffer(newPickups);

    SDL_LogDebug(LOG_PICKUPS, "[Disk Write Thread]: New pickups to add: %llu", newPickups);

    int i = 0;
    for (const auto &it : snapshot) {
        if (i > static_cast<int>(newPickups)) {
            SDL_LogError(LOG_PICKUPS, "[Disk Write Thread]: Overloaded max size of buffer at index: %i", i);
            break;
        }
        buffer[i] = (serializePickup(it.second));
        i++;
    }

    SDL_LogDebug(LOG_PICKUPS, "[Disk Write Thread]: New pickups added: %i", i);

    file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size() * sizeof(pickup_serial_data)); 
    if (!file) {
        SDL_LogError(LOG_PICKUPS, "[Disk Write Thread]: ERROR: Failed to write the pickup buffer data");
        writeDiskCompleted.store(true);
        return;
    }

    if (newPickups > 0) {
        pickup_count += static_cast<uint32_t>(newPickups);
        
        file.seekp(pickupCountPos);
        file.write(reinterpret_cast<const char*>(&pickup_count),sizeof(pickup_count)); // Pickup Count
        
        if (!file) {
            SDL_LogError(LOG_PICKUPS, "[Disk Write Thread]: ERROR: Failed to write the updated pickup_count");
            writeDiskCompleted.store(true);
            return;
        }
    }

    // DONE //
    writeDiskCompleted.store(true);

    auto stop = std::chrono::steady_clock::now();
    const float d = std::chrono::duration<float, std::milli>(stop-start).count();
    SDL_LogDebug(LOG_PICKUPS, "[Disk Write Thread]: Empty Mutation Map took [%fms]",d);
}

void _pickupManager::cleanDeadFromFileWorker() {
    auto start = std::chrono::steady_clock::now();
    
    // -- FILE ACCESS -- //
    const std::string saveDir = std::string(SAVE_DIRECTORY + global::saveFileName + PICKUPS_EXTENSION);
    std::fstream file(saveDir, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
        SDL_LogError(LOG_PICKUPS, "[Clean Disk Thread]: ERROR: Cannot open the save file: %s", saveDir.c_str());
        cleanDiskCompleted.store(true);
        return;
    }

    // -- VARIABLES -- //
    constexpr int BUFFER_SIZE = 4096;

    uint32_t pickup_count = 0;
    if (!verifyFile(file, pickup_count)) {
        SDL_LogError(LOG_PICKUPS, "[Clean Disk Thread]: ERROR: Cannot verify the save file: %s", saveDir.c_str());
        cleanDiskCompleted.store(true);
        return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // -- DONE -- //
    cleanDiskCompleted.store(true);

    auto stop = std::chrono::steady_clock::now();
    const float d = std::chrono::duration<float, std::milli>(stop-start).count();
    SDL_LogDebug(LOG_PICKUPS, "[Clean Disk Thread]: cleanDeadFromFileWorker took [%fms]",d);
}

pickup_serial_data _pickupManager::serializePickup(const _pickup &pickup) const {
    return pickup_serial_data {
      .id = pickup.id,
      .value = pickup.value,
      .type = static_cast<int32_t>(pickup.type),
      .xPos = pickup.pos.x,
      .yPos = pickup.pos.y,
      .alive = static_cast<uint32_t>(pickup.alive)
    };
}

bool _pickupManager::verifyFile(std::fstream &file, uint32_t &pickup_count) const {
    if (!file) {
        SDL_LogError(LOG_PICKUPS, "ERROR: File cannot be opened");
        return false;
    }

    char meta_header[2];
    file.read(meta_header,2);
    if (meta_header[0] != 'G' || meta_header[1] != 'G') {
        SDL_LogError(LOG_PICKUPS, "ERROR: Invalid pickup meta header");
        return false;
    }

    char data_header[4];
    file.read(data_header,4);
    if (data_header[0] != 'P' || data_header[1] != 'K' || data_header[2] != 'U' || data_header[3] != 'P') {
        SDL_LogError(LOG_PICKUPS, "ERROR: Invalid pickup data header");
        return false;
    }

    file.read(reinterpret_cast<char*>(&pickup_count), sizeof(pickup_count));  
    if (pickup_count == 0) {
        SDL_LogWarn(LOG_PICKUPS, "WARNING: Pickup count is zero");
    }

    return true;
}

bool _pickupManager::generatePickup(std::fstream &file, const pickup_config &config, float numChunks, pickup_type type, int &ID) {
    SDL_LogInfo(LOG_PICKUPS, "Generating pickups instance");
    auto start = std::chrono::steady_clock::now();

    // -- VARIABLES -- //
    [[maybe_unused]] const float WORLD_RADIUS = sqrt(numChunks) * NUM_TILES_CHUNK_SQR * TILE_D * 0.5f;
    [[maybe_unused]] const float WORLD_DIAMETER = sqrt(numChunks) * NUM_TILES_CHUNK_SQR * TILE_D;

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
                    buffer[i].id = ID++;
                    buffer[i].value = 10.0f;
                    buffer[i].type = type;
                    buffer[i].xPos = pos.x;
                    buffer[i].yPos = pos.y;
                    buffer[i].alive = static_cast<uint32_t>(true);

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

    auto stop = std::chrono::steady_clock::now();
    const float d = std::chrono::duration<float, std::milli>(stop - start).count();
    SDL_LogDebug(LOG_PICKUPS, "Generate Pickups took [%fms]", d);

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

        const float type = static_cast<float>(p->type);

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
        vbo[vIndex++] = type * uWidth; 
        vbo[vIndex++] = 1.0f; 
        vbo[vIndex++] = centerX; 
        vbo[vIndex++] = centerY; 
        vbo[vIndex++] = angle; 
        // Bottom-right (1)
        vbo[vIndex++] = halfWidth; 
        vbo[vIndex++] = -halfHeight; 
        vbo[vIndex++] = (type + 1) * uWidth; 
        vbo[vIndex++] = 1.0f; 
        vbo[vIndex++] = centerX; 
        vbo[vIndex++] = centerY; 
        vbo[vIndex++] = angle; 
        // Top-right (2)
        vbo[vIndex++] = halfWidth; 
        vbo[vIndex++] = halfHeight; 
        vbo[vIndex++] = (type + 1) * uWidth; 
        vbo[vIndex++] = 0.0f; 
        vbo[vIndex++] = centerX; 
        vbo[vIndex++] = centerY; 
        vbo[vIndex++] = angle; 
        // Top-left (3)
        vbo[vIndex++] = -halfWidth; 
        vbo[vIndex++] = halfHeight; 
        vbo[vIndex++] = type * uWidth; 
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