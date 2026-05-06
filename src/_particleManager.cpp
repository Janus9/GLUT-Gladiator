#include <_particleManager.h>

// -- STATIC MEMBERS -- //

glm::mat4 _particleManager::viewProjectionMatrix;

void _particleManager::setViewProjectionMatrix(const glm::mat4 &_viewProjectionMatrix) {
    viewProjectionMatrix = _viewProjectionMatrix;
}

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
    if (vaoID != 0) {
        glDeleteVertexArrays(1,&vaoID); // tell the GPU to delete the array buffer
        vaoID = 0;
    }

    delete texture;
    texture = nullptr;

    delete timer;
    timer = nullptr;
}

void _particleManager::initParticleManager(const string& fileName, int _numImages, _lightManager* lightManager, int _maxParticles) {
    maxParticles = _maxParticles;
    numImages = _numImages;
    sceneLightManager = lightManager;

    texture->loadTexture(fileName);

    // -- SHADER SETUP -- //
    particleShader.initShader("shaders/particle_manager/vertex.vs","shaders/particle_manager/fragment.fs");
    uint32_t program = particleShader.getProgram();

    sceneLightManager->addProgram(program);

    // Uniforms
    u_viewProjectionMatrix = glGetUniformLocation(program,"u_viewProjectionMatrix");
    u_texture = glGetUniformLocation(program,"u_texture");
    u_t = glGetUniformLocation(program,"u_t");

    glGenBuffers(1, &vboID); // Create a VBO buffer for particles
    glGenBuffers(1, &eboID); // Create a VBO buffer for particles
    glGenVertexArrays(1, &vaoID);

    particles.resize(maxParticles); // Set particles up in vector

    int maxSizeBytes = 10 * 4 * maxParticles * sizeof(float);

    glBindBuffer(GL_ARRAY_BUFFER,vboID);
    // This allocates memoery for the buffer but does NOT assign any data
    glBufferData(GL_ARRAY_BUFFER,maxSizeBytes,nullptr,GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    buildEBO();
    buildVAO();
}

void _particleManager::drawParticleManager() {
    buildVBO();
    
    if (aliveParticles <= 0) return; // No particles skip update loop
    
    glUseProgram(particleShader.getProgram());
    
    texture->bindTexture();

    // Setup uniforms
    glUniformMatrix4fv(u_viewProjectionMatrix, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
    glUniform1i(u_texture, 0); // Uses texture slot not ID thus its 0
    glUniform1f(u_t,t_value);

    sceneLightManager->applyLights(particleShader.getProgram());

    glBindVertexArray(vaoID);
    glDrawElements(GL_TRIANGLES, aliveParticles * 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}

void _particleManager::updateParticleManger(double dt) {
    t_value += dt;
    if (aliveParticles <= 0) return; // No particles skip update loop
    for (int i = 0; i < maxParticles; i++) {
        particle* p = &particles[i];
        
        p->age += dt; // Add delta time seconds to age
        if (p->age >= p->death) {
            p->alive = false;
        }

        if (p->hasFloor && p->pos.y <= p->floorPosY) {
            // Particle hit floor
            p->alive = false;
        }

        if (!p->alive) continue;

        p->acc.x = 0.0f;
        if(p->hasGravity) p->acc.y = -GRAVITY * 4;

        p->vel += p->acc * dt;
        p->pos += p->vel * dt;

        p->angle += p->rotationSpeed * dt;
    }
}

void _particleManager::spawnEffect(const Vec2f &pos, const particle_effect &effect) {
    uniform_real_distribution<float> vel_x_dist(effect.minVelX, effect.maxVelX);
    uniform_real_distribution<float> vel_y_dist(effect.minVelY, effect.maxVelY);
    uniform_real_distribution<float> rot_dist(effect.minRotation, effect.maxRotation);

    uniform_real_distribution<float> radius_dist(effect.minRadius, effect.maxRadius);

    uniform_real_distribution<float> lifeTime_dist(effect.minLifeTime, effect.maxLifeTime);

    uniform_real_distribution<float> offset_x_dist(effect.minSpawnOffsetX, effect.maxSpawnOffsetX);
    uniform_real_distribution<float> offset_y_dist(effect.minSpawnOffsetY, effect.maxSpawnOffsetY);

    uniform_real_distribution<float> wave_amp_dist(effect.waveAmplitudeMin, effect.waveAmplitudeMax);
    uniform_real_distribution<float> wave_freq_dist(effect.waveFrequencyMin, effect.waveFrequencyMax);
    uniform_real_distribution<float> wave_off_dist(0.0f, 2* PI);

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
            p->acc = {0.0f, 0.0f};
            p->rotationSpeed = rot_dist(rng);

            p->radius = radius_dist(rng);
            
            p->age = 0.0f;
            p->death = lifeTime_dist(rng);

            p->alive = true;

            p->angle = 0.0f;

            p->hasGravity = effect.hasGravity;
            p->hasFloor = effect.hasFloor;
            p->floorPosY = p->pos.y + effect.floorOffset;

            p->waveAmplitude = wave_amp_dist(rng);
            p->waveFrequency = wave_freq_dist(rng);
            p->waveOffset = wave_off_dist(rng);

            p->imageIndex = effect.imageIndex;
        }
    }
}

// -- PRIVATE -- //

void _particleManager::buildVBO() {
    float particleVboData[maxParticles * 10 * 4];
    int vIndex = 0;
    
    aliveParticles = 0;
    for (int i = 0; i < maxParticles; i++) {
        particle* p = &particles[i];
        if (!p->alive) continue; // Skips particles that are dead

        float angle = degreeToRad(p->angle);

        float centerX = p->pos.x;
        float centerY = p->pos.y;

        // Can change to width/height later
        float halfWidth = p->radius * 0.5f;
        float halfHeight = p->radius * 0.5f;

        if (numImages == 0) {
            // No images cant build VBO (protect against divide by 0)
            return;
        }
        const float uWidth = 1.0f / numImages;
        
        // Vbo (Quad) //
        // Bottom-left (0)
        particleVboData[vIndex++] = -halfWidth; 
        particleVboData[vIndex++] = -halfHeight; 
        particleVboData[vIndex++] = p->imageIndex * uWidth; 
        particleVboData[vIndex++] = 1.0f; 
        particleVboData[vIndex++] = centerX; 
        particleVboData[vIndex++] = centerY; 
        particleVboData[vIndex++] = angle; 
        particleVboData[vIndex++] = p->waveAmplitude; 
        particleVboData[vIndex++] = p->waveFrequency; 
        particleVboData[vIndex++] = p->waveOffset; 
        // Bottom-right (1)
        particleVboData[vIndex++] = halfWidth; 
        particleVboData[vIndex++] = -halfHeight; 
        particleVboData[vIndex++] = (p->imageIndex + 1) * uWidth; 
        particleVboData[vIndex++] = 1.0f; 
        particleVboData[vIndex++] = centerX; 
        particleVboData[vIndex++] = centerY; 
        particleVboData[vIndex++] = angle; 
        particleVboData[vIndex++] = p->waveAmplitude; 
        particleVboData[vIndex++] = p->waveFrequency; 
        particleVboData[vIndex++] = p->waveOffset; 
        // Top-right (2)
        particleVboData[vIndex++] = halfWidth; 
        particleVboData[vIndex++] = halfHeight; 
        particleVboData[vIndex++] = (p->imageIndex + 1) * uWidth; 
        particleVboData[vIndex++] = 0.0f; 
        particleVboData[vIndex++] = centerX; 
        particleVboData[vIndex++] = centerY; 
        particleVboData[vIndex++] = angle; 
        particleVboData[vIndex++] = p->waveAmplitude; 
        particleVboData[vIndex++] = p->waveFrequency; 
        particleVboData[vIndex++] = p->waveOffset; 
        // Top-left (3)
        particleVboData[vIndex++] = -halfWidth; 
        particleVboData[vIndex++] = halfHeight; 
        particleVboData[vIndex++] = p->imageIndex * uWidth; 
        particleVboData[vIndex++] = 0.0f; 
        particleVboData[vIndex++] = centerX; 
        particleVboData[vIndex++] = centerY; 
        particleVboData[vIndex++] = angle; 
        particleVboData[vIndex++] = p->waveAmplitude; 
        particleVboData[vIndex++] = p->waveFrequency; 
        particleVboData[vIndex++] = p->waveOffset; 

        aliveParticles++;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferSubData(GL_ARRAY_BUFFER,0,vIndex * sizeof(float),particleVboData); // We only send the particles that are alive
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

void _particleManager::buildVAO() {
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
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);

    GLsizei stride = 10 * sizeof(float);

    /**
     * void glVertexAttribPointer 	
     * ---------------------------
     * GLuint index          - Attribute to be modified (like a cache)
     * GLint size            - Number of components in the attribute (ex/ vec2 is 2)
  	 * GLenum type           - Type of data (ex/ float is GL_FLOAT)
  	 * GLboolean normalized  - ??? just set to GL_FALSE
  	 * GLsizei stride        - How much data per vertex (here its 7 floats so 9 * sizeof(float))
  	 * const void * pointer  - Offset of data for first component to start at (ex/ localPos is 0 since its first in the VBO structure setup)
     * 
     */

    // Setup a data array for the attributes (data is per vertex!)
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,stride,(void*)(0 * sizeof(float)));
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,stride,(void*)(2 * sizeof(float)));
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,stride,(void*)(4 * sizeof(float)));
    glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,stride,(void*)(6 * sizeof(float)));
    glVertexAttribPointer(4,1,GL_FLOAT,GL_FALSE,stride,(void*)(7 * sizeof(float)));
    glVertexAttribPointer(5,1,GL_FLOAT,GL_FALSE,stride,(void*)(8 * sizeof(float)));
    glVertexAttribPointer(6,1,GL_FLOAT,GL_FALSE,stride,(void*)(9 * sizeof(float)));

    glBindVertexArray(0);
}