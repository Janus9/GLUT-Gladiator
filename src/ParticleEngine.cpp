/**
 * /summary
 *  Particle Engine stores a vector of "Particle Batch" as memory storage. Despite have a max particle size, its memory is dynamic, memory is freed once all particles
 *  of a given layer are marked as dead. However, video memory is fixed to the max particle size. It is buffered by sub data, but allocated by full. 
 * 
 * /ParticleBatch
 *  This stores information relating to a "batch" of particles. A "batch" is simply a particle of a given texture. Thus particles using
 *  the same texture path are in the same batch. The batch stores a GPU textureID, how many columns the animation sheet is. 
 *  It includes c_uWidth and c_vWidth which are simply how many u/v units for textuing the sheet is per frame. 
 *  The biggest piece is aliveParticles, this says how many ALIVE particles exist, the particles vector is not wiped until ALL PARTICLES ARE DEAD. 
 *  
 *  Invariants
 *      particles.size() >= aliveParticles
 */

#include <ParticleEngine.h>
#include <SDL3/SDL_timer.h>

namespace particles {
    // PUBLIC //

    Engine::Engine() : rng(std::random_device{}()) {

    }

    Engine::~Engine() {
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

    void Engine::init(const Context& context) {
        textureManager = context.textureManager;
        lightManager = context.lightManager;

        if (!textureManager || !lightManager) {
            const std::string msg = std::string("ERROR: One of the context injections is nullptr:")
                + "\ntextureManager: " + (textureManager ? "OK" : "NULLPTR")
                + "\nlightManager: " + (lightManager ? "OK" : "NULLPTR");
            SDL_LogError(LOG_PARTICLE_ENGINE,msg.c_str());
        }

        // -- SHADER SETUP -- //
        particleShader.initShader("shaders/particle_manager/vertex.vs","shaders/particle_manager/fragment.fs");
        uint32_t program = particleShader.getProgram();

        lightManager->addProgram(program);

        // Uniforms
        u_viewProjectionMatrix = glGetUniformLocation(program,"u_viewProjectionMatrix");
        u_texture = glGetUniformLocation(program,"u_texture");
        u_t = glGetUniformLocation(program,"u_t");

        glGenBuffers(1, &vboID); // Create a VBO buffer for particles
        glGenBuffers(1, &eboID); // Create a VBO buffer for particles
        glGenVertexArrays(1, &vaoID);

        constexpr size_t vboSize = MAX_PARTICLES * VERTICIES_PER_PARTICLE * sizeof(Vertex);

        glBindBuffer(GL_ARRAY_BUFFER,vboID);
        // This allocates memoery for the buffer but does NOT assign any data
        glBufferData(GL_ARRAY_BUFFER,vboSize,nullptr,GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,0);

        // EBO //
        std::vector<GLuint> particleEboData(MAX_PARTICLES * INDICIES_PER_PARTICLE);
        int vertexOffset = 0;
        int eIndex = 0;
        for (int i = 0; i < MAX_PARTICLES; i++) {
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
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, particleEboData.size() * sizeof(GLuint), particleEboData.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

        // VAO //
        // Bind the vertex array
        glBindVertexArray(vaoID);
        
        // Bind the buffers (setup before attributes since they read from the VBO)
        glBindBuffer(GL_ARRAY_BUFFER,vboID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,eboID);

        constexpr GLsizei stride = 10 * sizeof(float);

        // Enable & set the attributes
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,stride,(void*)(0 * sizeof(float)));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,stride,(void*)(2 * sizeof(float)));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,stride,(void*)(4 * sizeof(float)));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,stride,(void*)(6 * sizeof(float)));

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4,1,GL_FLOAT,GL_FALSE,stride,(void*)(7 * sizeof(float)));

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5,1,GL_FLOAT,GL_FALSE,stride,(void*)(8 * sizeof(float)));

        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6,1,GL_FLOAT,GL_FALSE,stride,(void*)(9 * sizeof(float)));

        glBindVertexArray(0);
    }

    void Engine::draw(const glm::mat4 &viewProjectionMatrix) {
        if (totalAliveParticles == 0) return;   // No particles to draw -- skipping
        
        buildVBO();

        // Shader
        glUseProgram(particleShader.getProgram());
        lightManager->applyLights(particleShader.getProgram());

        // Setup uniforms
        glUniformMatrix4fv(u_viewProjectionMatrix, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
        glUniform1i(u_texture, 0); // Uses texture slot not ID thus its 0
        glUniform1f(u_t,t_value);
        
        glBindVertexArray(vaoID);

        GLsizei offset = 0;
        for (size_t layer = 0; layer < particleList.size(); layer++) {
            const ParticleBatch &pBatch = particleList[layer];
            if (pBatch.aliveParticles == 0) continue;  // Skip layers of 0 particles

            glBindTexture(GL_TEXTURE_2D, pBatch.textureID);

            const GLsizei count = pBatch.aliveParticles * INDICIES_PER_PARTICLE;
            glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (void*)(offset * sizeof(GLuint)));
            offset += count;
        }

        glBindVertexArray(0);
        glUseProgram(0);
    }

    void Engine::update(double dt) {
        t_value += dt;
        if (totalAliveParticles == 0) return; // No particles skip update loop
        
        for (size_t layer = 0; layer < particleList.size(); layer++) {
            ParticleBatch &pBatch = particleList[layer];
            if (pBatch.particles.empty()) continue; // Skip layers of no particles to update

            for (size_t i = 0; i < pBatch.particles.size(); i++) {
                Particle &p = pBatch.particles[i];
                if (!p.alive) continue; // First check -- skip dead particles (if they become dead this looped, handled below)

                p.age += dt; // Add delta time seconds to age
                if (p.age >= p.death) {
                    // Particle reached end of life
                    p.alive = false;
                }

                if (p.hasFloor && p.pos.y <= p.floorPosY) {
                    // Particle hit floor
                    p.alive = false;
                }

                if (!p.alive) {
                    // Particle death
                    pBatch.aliveParticles--;
                    totalAliveParticles--;
                    continue;
                }

                p.acc.x = 0.0f;
                if (p.hasGravity) p.acc.y = -GRAVITY * 4;

                p.vel += p.acc * static_cast<float>(dt);
                p.pos += p.vel * static_cast<float>(dt);

                p.angle += p.rotationSpeed * dt;
            }

            if (pBatch.aliveParticles == 0) {
                SDL_LogDebug(LOG_PARTICLE_ENGINE, "Clearing batch: %s as all particles are dead", pBatch.texturePath.c_str());
                pBatch.particles.clear(); // Clear the memory since all particles are now dead
            }
        }
    }

    void Engine::spawnEffect(glm::vec2 pos, const Config &config) {
        std::uniform_real_distribution<float> vel_x_dist(config.minVelX, config.maxVelX);
        std::uniform_real_distribution<float> vel_y_dist(config.minVelY, config.maxVelY);
        std::uniform_real_distribution<float> rot_dist(config.minRotation, config.maxRotation);

        std::uniform_real_distribution<float> radius_dist(config.minRadius, config.maxRadius);

        std::uniform_real_distribution<float> lifeTime_dist(config.minLifeTime, config.maxLifeTime);

        std::uniform_real_distribution<float> offset_x_dist(config.minSpawnOffsetX, config.maxSpawnOffsetX);
        std::uniform_real_distribution<float> offset_y_dist(config.minSpawnOffsetY, config.maxSpawnOffsetY);

        std::uniform_real_distribution<float> wave_amp_dist(config.waveAmplitudeMin, config.waveAmplitudeMax);
        std::uniform_real_distribution<float> wave_freq_dist(config.waveFrequencyMin, config.waveFrequencyMax);
        std::uniform_real_distribution<float> wave_off_dist(0.0f, 2* PI);

        if (totalAliveParticles + config.particleCount > MAX_PARTICLES) {
            SDL_LogWarn(LOG_PARTICLE_ENGINE, "WARNING: Particle count of: %i exceeds max count of: %i", totalAliveParticles, MAX_PARTICLES);
        }

        bool registed;

        size_t layerIndex = -1;
        auto it = particleTable.find(config.texturePath);
        if (it != particleTable.end()) {
            // Already registered
            layerIndex = it->second;
            registed = true;
            SDL_LogDebug(LOG_PARTICLE_ENGINE, "Effect: %s already registed", config.texturePath.c_str());
        } else {
            // New registration
            particleList.emplace_back(); // Create a new particle entry into the list
            layerIndex = particleList.size() - 1; // Get the index of the new entry           
            particleTable[config.texturePath] = layerIndex; // Setup new entry in table
            registed = false;
            SDL_LogDebug(LOG_PARTICLE_ENGINE, "Effect: %s has not been registed", config.texturePath.c_str());
        }
        ParticleBatch &pBatch = particleList[layerIndex];
        if (!registed) {
            
            pBatch.sheetColumns = config.sheetColumns;
            pBatch.sheetRows = config.sheetRows;
            
            pBatch.texturePath = config.texturePath;
            
            bool divByZero = false;
            if (pBatch.sheetColumns <= 0) {
                SDL_LogWarn(LOG_PARTICLE_ENGINE, "WARNING: Number of columns in config: %s is 0 or less. Should be greater than 0.", pBatch.texturePath.c_str());
                divByZero = true;
            }

            if (pBatch.sheetRows <= 0) {
                SDL_LogWarn(LOG_PARTICLE_ENGINE, "WARNING: Number of rows in config: %s is 0 or less. Should be greater than 0.", pBatch.texturePath.c_str());
                divByZero = true;
            }
            
            // Values never change per registed animation sheet -- calculated once here
            if (divByZero) { // Protect against divide by 0
                SDL_LogWarn(LOG_PARTICLE_ENGINE, "WARNING: Image: %s will have 0 image dimensions (not visible) due to bad row/column params", pBatch.texturePath.c_str());
                pBatch.c_uWidth = 0.0f;
                pBatch.c_vWidth = 0.0f;
            } else {
                pBatch.c_uWidth = 1.0f / static_cast<float>(config.sheetColumns);
                pBatch.c_vWidth = 1.0f / static_cast<float>(config.sheetRows);
            }

            const texture_entry &texture = textureManager->getTextureEntry(config.texturePath);
            if (texture.ID == 0) {
                SDL_LogError(LOG_PARTICLE_ENGINE, "ERROR: Unable to load image: %s\n - Removing entry", config.texturePath.c_str());
                // Remove entries from bad insertion
                particleList.erase(particleList.begin() + layerIndex);
                particleTable.erase(config.texturePath);
                return;
            }
            pBatch.textureID = texture.ID;
        }

        const int particlesInMemory = pBatch.particles.size();  // Count of particles in the current memory layer
        pBatch.particles.resize(config.particleCount + particlesInMemory);  // Resize to fit the new elements

        for (int i = 0; i < config.particleCount; i++) {
            const int index = particlesInMemory + i;
            Particle &p = pBatch.particles[index];

            p.pos = pos;

            p.pos.x += offset_x_dist(rng);
            p.pos.y += offset_y_dist(rng);

            p.vel.x = vel_x_dist(rng);
            p.vel.y = vel_y_dist(rng);
            p.acc = {0.0f, 0.0f};
            p.rotationSpeed = rot_dist(rng);

            p.radius = radius_dist(rng);
            
            p.age = 0.0f;
            p.death = lifeTime_dist(rng);

            p.alive = true;

            p.angle = 0.0f;

            p.hasGravity = config.hasGravity;
            p.hasFloor = config.hasFloor;
            p.floorPosY = p.pos.y + config.floorOffset;

            p.waveAmplitude = wave_amp_dist(rng);
            p.waveFrequency = wave_freq_dist(rng);
            p.waveOffset = wave_off_dist(rng);

            p.colIndex = 0;
            p.rowIndex = config.animationRow;
        }

        totalAliveParticles += config.particleCount;
        pBatch.aliveParticles += config.particleCount; 
    }

    void Engine::logGpuMemoryUsage() const {
        size_t total = 0;

        total += static_cast<size_t>(MAX_PARTICLES) * 
            static_cast<size_t>(VERTICIES_PER_PARTICLE) * 
            sizeof(Vertex);
        
        total += static_cast<size_t>(MAX_PARTICLES) *
            static_cast<size_t>(INDICIES_PER_PARTICLE) *
            sizeof(GLuint);

        const std::string msg = std::string("\n") +
            "| >> PARTICLE ENGINE DEBUG << |\n" + 
            "|-----------------------------|\n" +
            "| - GPU Memory Usage: " + std::to_string(total) + "B" + " (" + std::to_string(static_cast<double>(total) / 1000000.0) + "MB)\n" +
            "|-----------------------------|\n";
        
        SDL_LogDebug(LOG_PARTICLE_ENGINE, msg.c_str());
    }

    void Engine::logCpuMemoryUsage() const {
        size_t total = 0;
        total += sizeof(*this); // Size of engine itself
        total += particleList.capacity() * sizeof(ParticleBatch);   // Estimation of particle list
        // Vector //
        for (size_t i = 0; i < particleList.size(); i++) {
            const ParticleBatch &pBatch = particleList[i];
            // Heap memory of batch
            total += pBatch.particles.capacity() * sizeof(Particle);
            total += pBatch.texturePath.capacity() * sizeof(char);
        }
        // Unordered Map //
        for (auto it = particleTable.begin(); it != particleTable.end(); it++) {
            total += sizeof(*it);
            total += it->first.capacity() * sizeof(char);
        }

        const std::string msg = std::string("\n") +
            "| >> PARTICLE ENGINE DEBUG << |\n" + 
            "|-----------------------------|\n" +
            "| - CPU Memory Usage: " + std::to_string(total) + "B" + " (" + std::to_string(static_cast<double>(total) / 1000000.0) + "MB)\n" +
            "|-----------------------------|\n";
        
        SDL_LogDebug(LOG_PARTICLE_ENGINE, msg.c_str());
    }

    // PRIVATE //
    void Engine::buildVBO() {
        glBindBuffer(GL_ARRAY_BUFFER, vboID);

        int runningVertexOffset = 0;
        for (size_t layer = 0; layer < particleList.size(); layer++) {
            const ParticleBatch &pBatch = particleList[layer];

            int realAliveCount = 0;

            for (size_t i = 0; i < pBatch.particles.size(); i++) {
                if (pBatch.particles[i].alive) {
                    realAliveCount++;
                }
            }

            if (realAliveCount != pBatch.aliveParticles) {
                std::cout << "ERROR: aliveParticles mismatch on layer " << layer << "\n";
                std::cout << "pBatch.aliveParticles: " << pBatch.aliveParticles << "\n";
                std::cout << "realAliveCount: " << realAliveCount << "\n";
            }
                        
            std::vector<Vertex> vboData(pBatch.aliveParticles * VERTICIES_PER_PARTICLE);
            int vIndex = 0;
            for (size_t layerIndex = 0; layerIndex < particleList[layer].particles.size(); layerIndex++) {
                const Particle &p = particleList[layer].particles[layerIndex];

                if (!p.alive) continue; // Skip dead particles

                const float angle = degreeToRad(p.angle);

                const float centerX = p.pos.x;
                const float centerY = p.pos.y;

                const float halfWidth = p.radius * 0.5f;
                const float halfHeight = p.radius * 0.5f;

                const float u0 = static_cast<float>(p.colIndex) * pBatch.c_uWidth;
                const float v0 = static_cast<float>(p.rowIndex) * pBatch.c_vWidth;
                const float u1 = static_cast<float>(p.colIndex + 1.0f) * pBatch.c_uWidth;
                const float v1 = static_cast<float>(p.rowIndex + 1.0f) * pBatch.c_vWidth;

                // Bottom-left
                vboData[vIndex].w = -halfWidth;
                vboData[vIndex].h = -halfHeight;
                vboData[vIndex].u = u0;
                vboData[vIndex].v = v1;
                vboData[vIndex].x = centerX;
                vboData[vIndex].y = centerY;
                vboData[vIndex].a = angle;
                vboData[vIndex].w_a = p.waveAmplitude;
                vboData[vIndex].w_f = p.waveFrequency;
                vboData[vIndex].w_o = p.waveOffset;
                vIndex++;
                // Bottom-right
                vboData[vIndex].w = halfWidth;
                vboData[vIndex].h = -halfHeight;
                vboData[vIndex].u = u1;
                vboData[vIndex].v = v1;
                vboData[vIndex].x = centerX;
                vboData[vIndex].y = centerY;
                vboData[vIndex].a = angle;
                vboData[vIndex].w_a = p.waveAmplitude;
                vboData[vIndex].w_f = p.waveFrequency;
                vboData[vIndex].w_o = p.waveOffset;
                vIndex++;
                // Top-right
                vboData[vIndex].w = halfWidth;
                vboData[vIndex].h = halfHeight;
                vboData[vIndex].u = u1;
                vboData[vIndex].v = v0;
                vboData[vIndex].x = centerX;
                vboData[vIndex].y = centerY;
                vboData[vIndex].a = angle;
                vboData[vIndex].w_a = p.waveAmplitude;
                vboData[vIndex].w_f = p.waveFrequency;
                vboData[vIndex].w_o = p.waveOffset;
                vIndex++;
                // Top-left
                vboData[vIndex].w = -halfWidth;
                vboData[vIndex].h = halfHeight;
                vboData[vIndex].u = u0;
                vboData[vIndex].v = v0;
                vboData[vIndex].x = centerX;
                vboData[vIndex].y = centerY;
                vboData[vIndex].a = angle;
                vboData[vIndex].w_a = p.waveAmplitude;
                vboData[vIndex].w_f = p.waveFrequency;
                vboData[vIndex].w_o = p.waveOffset;
                vIndex++;
            }
            const GLintptr offset = runningVertexOffset * sizeof(Vertex);
            const GLsizeiptr size = vIndex * sizeof(Vertex);
            glBufferSubData(GL_ARRAY_BUFFER,offset, size, vboData.data());

            runningVertexOffset += vIndex;
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}