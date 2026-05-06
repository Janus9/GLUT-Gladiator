#include <_enemyManager.h>
#include <_orc.h>
#include <_vampire.h>

// -- -- ENEMY -- -- //

// -- STATIC -- //

int _enemy::nextId = 0;

// -- PUBLIC -- //

_enemy::_enemy() {
    enemyID = nextId;
    nextId++;
}

_enemy::~_enemy() {
    // dtor
}

void _enemy::updateEnemy(double dt) {

}

void _enemy::initEnemy(const enemy_config &config, const _textureManager* textureManager) {

    switch (config.type) {
        // -- DEFAULT TURRET -- //
        case ENEMY_TURRET: {
            // Data Setup //
            setMaxHealth(config.maxHP);
            resetHealth();

            fireRate = config.fireRate;
            slewRate = config.slewRate;

            detectionRadius = config.detectionRadius;

            eType = config.type;
            team = config.team;

            // Sprite Setup //
            setupSprite("MAIN");
            _sprite* main_sprite = getSprite("MAIN");
            if (main_sprite) {
                const texture_entry &tex = textureManager->getTextureEntry("images/enemy/turret.png");
                main_sprite->initSprite(tex,4,2,0,12);
                main_sprite->createSpriteAction(sprite_action("SHOOT",0,0,3));
                main_sprite->createSpriteAction(sprite_action("DEATH",1,0,3));
                main_sprite->loadSpriteAction("SHOOT");
                main_sprite->stopAnimation();
                Vec2f size = main_sprite->getSize();
                main_sprite->pivotPoint = {size.x/2.0f, size.y/2.0f};
                // main_sprite->DEBUG_showPivotPoint = true;
                // main_sprite->DEBUG_showOrigin = true;
                setSingleSprite(main_sprite); // Only one sprite
            }
            break;
        }
        // -- GATLING TURRET -- //
        case ENEMY_GATLING: {
            // Data Setup //
            setMaxHealth(config.maxHP);
            resetHealth();

            fireRate = config.fireRate;
            slewRate = config.slewRate;

            detectionRadius = config.detectionRadius;

            eType = config.type;
            team = config.team;

            // Sprite Setup //
            setupSprite("BASE");
            _sprite* base_sprite = getSprite("BASE");
            if (base_sprite) {
                const texture_entry &tex = textureManager->getTextureEntry("images/enemy/gatling_gun/gatling_base.png");
                base_sprite->initSprite(tex,1,1,0,12);
                base_sprite->setIdleFrame(0,0);
                base_sprite->stopAnimation();
            }

            setupSprite("TURRET");
            _sprite* turret_sprite = getSprite("TURRET");
            if (turret_sprite) {
                const texture_entry &tex = textureManager->getTextureEntry("images/enemy/gatling_gun/gatling_turret.png");
                turret_sprite->initSprite(tex,9,4,1,12);
                turret_sprite->createSpriteAction(sprite_action("REV",0,0,3));
                turret_sprite->createSpriteAction(sprite_action("IDLE",1,0,5));
                turret_sprite->createSpriteAction(sprite_action("SHOOT",2,0,3));
                turret_sprite->createSpriteAction(sprite_action("DEATH",3,0,8));
                turret_sprite->loadSpriteAction("SHOOT");
                turret_sprite->setIdleFrame(0,0);
                turret_sprite->startAnimation();
                Vec2f size = turret_sprite->getSize();
                turret_sprite->pivotPoint = {size.x/2.0f, size.y/2.0f};
            }
            break;
        }
    }
}

enemy_serial_data _enemy::serializeEnemy() const {
    enemy_serial_data enemy_data;
    enemy_data.type = static_cast<uint8_t>(eType);
    enemy_data.team = static_cast<uint8_t>(team);
    enemy_data.maxHP = getMaxHealth();
    enemy_data.fireRate = fireRate;
    enemy_data.slewRate = slewRate;
    enemy_data.detectionRadius = detectionRadius;
    enemy_data.posX = pos.x;
    enemy_data.posY = pos.y;
    enemy_data.padding = 0; // Padding doesnt do anything
    return enemy_data;
}

bool _enemy::operator==(const _enemy &other) const {
    return enemyID == other.enemyID;
}

// -- -- ENEMY MANAGER -- -- //

// -- STATIC -- //
glm::mat4 _enemyManager::viewProjectionMatrix;

void _enemyManager::setViewProjectionMatrix(const glm::mat4 &_viewProjectionMatrix) {
    viewProjectionMatrix = _viewProjectionMatrix;
}

// -- PUBLIC -- //

_enemyManager::_enemyManager() {
    
}

_enemyManager::~_enemyManager() {
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
}

void _enemyManager::initEnemyManager(_player* currentPlayer, _world* currentWorld, _bulletManager* currentBulletManager, _sounds* currentSounds, _lightManager* lightManager, _textureManager* textureManager) {
    player = currentPlayer;
    world = currentWorld;
    bulletManager = currentBulletManager;
    sounds = currentSounds;
    sceneLightManager = lightManager;
    sceneTextureManager = textureManager;

    // -- SHADER SETUP -- //
    shader.initShader("shaders/enemy_manager/vertex.vs","shaders/enemy_manager/fragment.fs");
    uint32_t program = shader.getProgram();

    sceneLightManager->addProgram(program);

    // Uniforms
    u_viewProjectionMatrix = glGetUniformLocation(program,"u_viewProjectionMatrix");
    u_texture = glGetUniformLocation(program,"u_texture");
    u_time = glGetUniformLocation(program,"u_time");

    glGenBuffers(1, &vboID); 
    glGenBuffers(1, &eboID); 
    glGenVertexArrays(1, &vaoID);

    buildVAO();

    particleManager->initParticleManager("images/enemy/enemy_particles.png",6, sceneLightManager, 10000);
    // Turret Hit Effect //
    turret_hit_effect.amount = 15;
    turret_hit_effect.imageIndex = 3;

    turret_hit_effect.minVelX = -8.0f;
    turret_hit_effect.maxVelX = 8.0f;
    turret_hit_effect.minVelY = 10.0f;
    turret_hit_effect.maxVelY = 25.0f;
    turret_hit_effect.minRotation = -45.0f;
    turret_hit_effect.maxRotation = 45.0f;

    turret_hit_effect.minRadius = 1.0f;
    turret_hit_effect.maxRadius = 2.0f;

    turret_hit_effect.minLifeTime = 5.0f;
    turret_hit_effect.maxLifeTime = 5.0f;

    turret_hit_effect.minSpawnOffsetX = -3.0f;
    turret_hit_effect.maxSpawnOffsetX = 3.0f;
    turret_hit_effect.minSpawnOffsetY = -3.0f;
    turret_hit_effect.maxSpawnOffsetY = 3.0f;

    turret_hit_effect.hasGravity = true;
    turret_hit_effect.hasFloor = true;
    turret_hit_effect.floorOffset = -10.0f;

    // Turret Death Effect //
    turret_death_effect.amount = 45;
    turret_death_effect.imageIndex = 0;

    turret_death_effect.minVelX = -12.0f;
    turret_death_effect.maxVelX = 12.0f;
    turret_death_effect.minVelY = 14.0f;
    turret_death_effect.maxVelY = 30.0f;
    turret_death_effect.minRotation = -45.0f;
    turret_death_effect.maxRotation = 45.0f;

    turret_death_effect.minRadius = 1.8f;
    turret_death_effect.maxRadius = 3.2f;

    turret_death_effect.minLifeTime = 5.0f;
    turret_death_effect.maxLifeTime = 5.0f;

    turret_death_effect.minSpawnOffsetX = -4.5f;
    turret_death_effect.maxSpawnOffsetX = 4.5f;
    turret_death_effect.minSpawnOffsetY = -4.5f;
    turret_death_effect.maxSpawnOffsetY = 4.5f;

    turret_death_effect.hasGravity = true;
    turret_death_effect.hasFloor = true;
    turret_death_effect.floorOffset = -10.0f;

    // Gatling Death Effect //
    gatling_death_effect.amount = 100;
    gatling_death_effect.imageIndex = 1;

    gatling_death_effect.minVelX = -7.0f;
    gatling_death_effect.maxVelX = 7.0f;
    gatling_death_effect.minVelY = 30.0f;
    gatling_death_effect.maxVelY = 55.0f;
    gatling_death_effect.minRotation = -65.0f;
    gatling_death_effect.maxRotation = 65.0f;

    gatling_death_effect.minRadius = 2.5f;
    gatling_death_effect.maxRadius = 4.5f;

    gatling_death_effect.minLifeTime = 5.0f;
    gatling_death_effect.maxLifeTime = 5.0f;

    gatling_death_effect.minSpawnOffsetX = -16.0f;
    gatling_death_effect.maxSpawnOffsetX = 16.0f;
    gatling_death_effect.minSpawnOffsetY = -16.0f;
    gatling_death_effect.maxSpawnOffsetY = 16.0f;

    gatling_death_effect.hasGravity = true;
    gatling_death_effect.hasFloor = true;
    gatling_death_effect.floorOffset = -10.0f;

    // Gatling Death Smoke Effect //
    gatling_death_effect_smoke.amount = 250;
    gatling_death_effect_smoke.imageIndex = 2;

    gatling_death_effect_smoke.minVelX = -3.0f;
    gatling_death_effect_smoke.maxVelX = 3.0f;
    gatling_death_effect_smoke.minVelY = 15.0f;
    gatling_death_effect_smoke.maxVelY = 25.0f;

    gatling_death_effect_smoke.minRadius = 2.9f;
    gatling_death_effect_smoke.maxRadius = 5.1f;

    gatling_death_effect_smoke.minLifeTime = 3.5f;
    gatling_death_effect_smoke.maxLifeTime = 6.0f;

    gatling_death_effect_smoke.minSpawnOffsetX = -6.0f;
    gatling_death_effect_smoke.maxSpawnOffsetX = 6.0f;
    gatling_death_effect_smoke.minSpawnOffsetY = -6.0f;
    gatling_death_effect_smoke.maxSpawnOffsetY = 6.0f;

    gatling_death_effect_smoke.hasGravity = false;

    gatling_death_effect_smoke.waveAmplitudeMin = 2.0f;
    gatling_death_effect_smoke.waveAmplitudeMax = 3.0f;
    gatling_death_effect_smoke.waveFrequencyMin = 0.5f;
    gatling_death_effect_smoke.waveFrequencyMax = 1.2f;

    // Turret Bullet Casing Effect //
    turret_bullet_casing.amount = 1;
    turret_bullet_casing.imageIndex = 5;

    turret_bullet_casing.minVelX = 5.5f;
    turret_bullet_casing.maxVelX = 10.5f;
    turret_bullet_casing.minVelY = 15.0f;
    turret_bullet_casing.maxVelY = 28.0f;
    turret_bullet_casing.minRotation = 240.0f;
    turret_bullet_casing.maxRotation = 280.0f;

    turret_bullet_casing.minRadius = 2.0f;
    turret_bullet_casing.maxRadius = 2.0f;

    turret_bullet_casing.minLifeTime = 5.0f;
    turret_bullet_casing.maxLifeTime = 5.0f;

    turret_bullet_casing.hasFloor = true;
    turret_bullet_casing.floorOffset = -24.0f;

    // Gatling Bullet Casing Effect //
    gatling_bullet_casing.amount = 1;
    gatling_bullet_casing.imageIndex = 4;

    gatling_bullet_casing.minVelX = 5.5f;
    gatling_bullet_casing.maxVelX = 10.5f;
    gatling_bullet_casing.minVelY = 15.0f;
    gatling_bullet_casing.maxVelY = 28.0f;
    gatling_bullet_casing.minRotation = 240.0f;
    gatling_bullet_casing.maxRotation = 280.0f;

    gatling_bullet_casing.minRadius = 3.0f;
    gatling_bullet_casing.maxRadius = 3.0f;

    gatling_bullet_casing.minLifeTime = 5.0f;
    gatling_bullet_casing.maxLifeTime = 5.0f;

    gatling_bullet_casing.hasFloor = true;
    gatling_bullet_casing.floorOffset = -24.0f;
}

void _enemyManager::updateEnemies(double dt) {
    time += dt;

    // Iterate backwards to removal safety
    particleManager->updateParticleManger(dt);
    if (enemyList.size() <= 0) return; // Empty list - no need to run loop
    if (!player) return; 
    for (int i = enemyList.size()-1; i >= 0; i--) {
        _enemy* enemy = enemyList[i].get();
        enemy->setPosition(enemy->pos);
        enemy->updateUnit(dt);

        // Kill enemy event //
        if (enemy->isDead() && enemy->deathTime > enemy->timeInDeathAnimation) {
            // Get a list of sprites registered to the enemy
            const vector<_sprite*>& enemySpriteList = enemy->getSpriteList();
            for (const auto &sprite : enemySpriteList) {
                // For each sprite we remove it from the map

                const GLuint textureID = sprite->getTextureID();
                const int layer = sprite->getLayer();
                
                auto layerIt = layerMap.find(layer);
                if (layerIt != layerMap.end()) {
                    // Layer map not empty -- check texture map
                    auto& textureMap = layerIt->second; 
                    auto mapIt = textureMap.find(textureID);
                    if (mapIt != textureMap.end()) {
                        // Texture map not empty -- remove
                        vector<_sprite*> &spriteVector = mapIt->second;      // Pull vector out of the map
                        auto spriteIt = find(spriteVector.begin(),spriteVector.end(),sprite);
                        if (spriteIt != spriteVector.end()) {
                            spriteVector.erase(spriteIt);
                        }
                        if (spriteVector.empty()) {
                            // Vector empty? remove it from map
                            textureMap.erase(mapIt);
                        }
                        if (textureMap.empty()) {
                            // Texture empty? remove it from map
                            layerMap.erase(layerIt);
                        }
                    }
                }
            }
            spriteCount -= enemy->getNumSprites();
        }

        switch(enemy->eType) {            
            // -- DEFAULT TURRET -- //
            case ENEMY_TURRET: {
                _sprite* sprite = enemy->getSingleSprite();

                // Handle player being dead
                if (player->isDead() || player->isRealDead) {
                    sprite->stopAnimation();
                    sounds->removeSfx3DLooped(enemy->getID());
                    continue;
                }

                if (enemy->isDead() && !enemy->inDeathAnimation) {
                    enemy->inDeathAnimation = true;
                    sprite->setFPS(4);
                    sprite->setIdleFrame(3,1);
                    sprite->playAction("DEATH");
                    enemy->deathTime = 0.0;
                    particleManager->spawnEffect(enemy->pos,turret_death_effect);
                    if (sounds) sounds->playSfx("ENEMY_DEATH");
                    continue;
                } else if (enemy->isDead() && enemy->deathTime > enemy->timeInDeathAnimation) {
                    enemyList.erase(enemyList.begin() + i);
                    continue;
                }
                if (enemy->isDead()) {
                    enemy->deathTime += dt;
                    continue;
                }
                float distance = enemy->pos.distance(player->pos);
                if (distance < enemy->detectionRadius) {
                    // Enemy in range
                    bool focused = enemy->focusOn(player->pos,enemy->slewRate);
                    if (focused) {
                        enemy->firingTime += dt;
                        if (enemy->firingTime > 1.0f/(enemy->fireRate/60.0f)) {
                            bulletManager->spawnBulletEffect(enemy->pos,player->pos,_team::ENEMY,*bullet_1);
                            particleManager->spawnEffect(enemy->pos,turret_bullet_casing);
                            if (sounds) sounds->playSfx3D("ENEMY_SHOOT", enemy->pos);
                            sprite->setFPS(enemy->fireRate / 60.0f);
                            enemy->firingTime = 0;
                        }
                        sprite->startAnimation();
                    } else {
                        sprite->setFPS(12);
                        sprite->stopAnimation();
                    }
                }  else {
                    sprite->setFPS(12);
                    sprite->stopAnimation();
                }
                break;
            }

            // -- ORC -- //
            case ENEMY_ORC: {
                _orc* orc = static_cast<_orc*>(enemy);
                _sprite* sprite = orc->getSingleSprite();

                // Handle player being dead
                if (player->isDead() || player->isRealDead) {
                    sprite->stopAnimation();
                    sounds->removeSfx3DLooped(enemy->getID());
                    continue;
                }

                if (enemy->isDead() && !enemy->inDeathAnimation) {
                    orc->triggerDeath(sounds);
                    continue;
                } else if (enemy->isDead() && enemy->deathTime > enemy->timeInDeathAnimation) {
                    enemyList.erase(enemyList.begin() + i);
                    continue;
                }
                if (enemy->isDead()) {
                    enemy->deathTime += dt;
                    continue;
                }
                orc->updateOrc(dt, player, world, sounds);
                break;
            }

            // -- VAMPIRE (boss + minions) -- //
            case ENEMY_VAMPIRE_MINION1:
            case ENEMY_VAMPIRE_MINION2:
            case ENEMY_VAMPIRE: {
                _vampire* vampire = static_cast<_vampire*>(enemy);
                _sprite* sprite = vampire->getSingleSprite();

                // Handle player being dead
                if (player->isDead() || player->isRealDead) {
                    sprite->stopAnimation();
                    sounds->removeSfx3DLooped(enemy->getID());
                    continue;
                }

                if (enemy->isDead() && !enemy->inDeathAnimation) {
                    vampire->triggerDeath(sounds);
                    continue;
                } else if (enemy->isDead() && enemy->deathTime > enemy->timeInDeathAnimation) {
                    enemyList.erase(enemyList.begin() + i);
                    continue;
                }
                if (enemy->isDead()) {
                    enemy->deathTime += dt;
                    continue;
                }
                vampire->updateVampire(dt, player, world, sounds);
                break;
            }

            // -- GATLING TURRET -- //
            case ENEMY_GATLING: {
                _sprite* sprite = enemy->getSprite("TURRET");
                
                // Handle player being dead
                if (player->isDead() || player->isRealDead) {
                    sprite->stopAnimation();
                    sounds->removeSfx3DLooped(enemy->getID());
                    continue;
                }

                // Initial death event
                if (enemy->isDead() && !enemy->inDeathAnimation) {
                    enemy->inDeathAnimation = true;
                    sprite->setFPS(12);
                    sprite->setIdleFrame(8,1);
                    sprite->playAction("DEATH");
                    enemy->deathTime = 0.0;
                    if (sounds) sounds->playSfx("ENEMY_DEATH");
                    particleManager->spawnEffect(enemy->pos,gatling_death_effect);
                    particleManager->spawnEffect(enemy->pos,gatling_death_effect_smoke);
                    continue;
                // Final death event (removes enemy)
                } else if (enemy->isDead() && enemy->deathTime > enemy->timeInDeathAnimation) {
                    enemyList.erase(enemyList.begin() + i);
                    continue;
                }

                // If dead update death time to final death event
                if (enemy->isDead()) {
                    enemy->deathTime += dt;
                    continue;
                }

                const float distance = enemy->pos.distance(player->pos);
                if (distance < enemy->detectionRadius) {
                    // Enemy in range
                    const bool focused = enemy->focusOn(player->pos,enemy->slewRate,5.0f,sprite);
                    const bool reved = enemy->revTime > enemy->timeInRevAnimation;
                    if (focused && reved) {
                        // Focused on player -- ready to fire
                        enemy->firingTime += dt;
                        if (enemy->firingTime > 1.0f/(enemy->fireRate/60.0f)) {
                            bulletManager->spawnBulletEffect(enemy->pos,player->pos,_team::ENEMY,*bullet_2);
                            particleManager->spawnEffect(enemy->pos,gatling_bullet_casing);
                            enemy->firingTime = 0;
                        }
                        sounds->playSfx3DLooped("GATLING_SHOOT", enemy->getID(), enemy->pos);
                        sprite->setFPS(enemy->fireRate / 60.0f);
                        sprite->loadSpriteAction("SHOOT");
                    } else {
                        if (focused && !reved) {
                            enemy->revTime += dt;
                        }
                        if (!focused) {
                            enemy->revTime = 0;
                        }
                        sprite->loadSpriteAction("REV");
                        sprite->setFPS(enemy->fireRate / 60.0f);
                        sounds->playSfx3DLooped("GATLING_REV", enemy->getID(), enemy->pos);
                    }
                }  else {
                    enemy->revTime = 0.0;
                    sprite->loadSpriteAction("IDLE");
                    sprite->setFPS(12);
                    sounds->removeSfx3DLooped(enemy->getID());
                }
                break;
            }
        }
    }
}

void _enemyManager::drawEnemies() {
    // cout << "textureMap size: " << textureMap.size() << endl;

    if (spriteCount <= 0) {
        // No enemies to draw
        particleManager->drawParticleManager();
        return;
    }

    glUseProgram(shader.getProgram());

    // Setup uniforms
    glUniformMatrix4fv(u_viewProjectionMatrix, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
    glUniform1i(u_texture, 0); // Uses texture slot not ID thus its 0
    glUniform1f(u_time,time);

    sceneLightManager->applyLights(shader.getProgram());

    glBindVertexArray(vaoID);

    // This iterates layer by layer so that some sprites draw on top of another
    for (auto it = layerMap.begin(); it != layerMap.end(); it++) {
        auto& textureMap = it->second;
        for (auto it = textureMap.begin(); it != textureMap.end(); it++) {

            const GLuint textureID = it->first;                   // Texture ID
            const vector<_sprite*> &spriteVector = it->second;    // List of sprites mapped to texture ID

            // cout << "Texture ID: " << textureID << "\n";
            // cout << " - Num Sprites: " << spriteVector.size() << "\n";

            if (spriteVector.size() <= 0) {
                // Texture ID is empty, skip
                continue;
            } 

            // BUILD BATCH VBO //
            const int spriteBatchCount = spriteVector.size();
            vector<float> vboData(spriteBatchCount * 7 * 4);
            int vIndex = 0;

            for (auto &sprite : spriteVector) {
                if (!sprite->hidden && sprite->ocapacity > 0.0f) { // Ignore hidden sprites
                    sprite->buildSpriteVBO(vboData.data(),vIndex);
                }
            }

            glBindBuffer(GL_ARRAY_BUFFER, vboID);
            glBufferData(GL_ARRAY_BUFFER, vboData.size() * sizeof(float), vboData.data(), GL_DYNAMIC_DRAW);

            // BUILD BATCH EBO //
            vector<uint32_t> eboData(spriteBatchCount * 6);
            int vertexOffset = 0;
            int eIndex = 0;

            for (int i = 0; i < spriteBatchCount; i++) {
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

            // cout << "-- VBO DATA OUT FOR TEXTURE ID " << textureID << " -- \n";
            // for (int i = 0; i < vboData.size(); i++) {
            //     cout << "vboData[" << i << "] = " << vboData[i] << "\n";
            // }
            // cout << "-- VBO DATA FINISH -- \n";

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, eboData.size() * sizeof(uint32_t), eboData.data(), GL_DYNAMIC_DRAW);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);

            glDrawElements(GL_TRIANGLES, spriteBatchCount * 6, GL_UNSIGNED_INT, 0);
        } 
    }

    glBindVertexArray(0);

    glUseProgram(0);

    particleManager->drawParticleManager();
}

void _enemyManager::addEnemy(const Vec2f &_pos, const enemy_config &config) {
    unique_ptr<_enemy> newEnemy;
    if (config.type == ENEMY_ORC) {
        unique_ptr<_orc> orc = make_unique<_orc>();
        orc->initOrc(sceneTextureManager);
        newEnemy = move(orc);
    } else if (config.type == ENEMY_VAMPIRE) {
        unique_ptr<_vampire> vampire = make_unique<_vampire>();
        vampire->initVampire(sceneTextureManager, VAMPIRE_BOSS);
        newEnemy = move(vampire);
    } else if (config.type == ENEMY_VAMPIRE_MINION1) {
        unique_ptr<_vampire> vampire = make_unique<_vampire>();
        vampire->initVampire(sceneTextureManager, VAMPIRE_MINION1);
        newEnemy = move(vampire);
    } else if (config.type == ENEMY_VAMPIRE_MINION2) {
        unique_ptr<_vampire> vampire = make_unique<_vampire>();
        vampire->initVampire(sceneTextureManager, VAMPIRE_MINION2);
        newEnemy = move(vampire);
    } else {
        newEnemy = make_unique<_enemy>();
        newEnemy->initEnemy(config, sceneTextureManager);
    }
    newEnemy->pos = _pos;

    const vector<_sprite*>& enemySpriteList = newEnemy->getSpriteList();
    for (const auto &sprite : enemySpriteList) {
        const GLuint textureID = sprite->getTextureID();
        const int layer = sprite->getLayer(); 

        layerMap[layer][textureID].push_back(sprite);
    }

    spriteCount += newEnemy->getNumSprites();   // Increase sprite count

    // Push_back last to avoid weird move stuff
    enemyList.push_back(move(newEnemy));
}

vector<enemy_serial_data> _enemyManager::exportSerializedEnemies() const {
    vector<enemy_serial_data> enemy_data;
    for (int i = 0; i < enemyList.size(); i++) {
        const _enemy* enemy = enemyList[i].get();
        if (enemy->inDeathAnimation || enemy->isDead()) continue; // Skip dead enemies
        enemy_data.push_back(enemy->serializeEnemy());
    }
    return enemy_data;
}

bool _enemyManager::importSerializedEnemies(const vector<enemy_serial_data> &enemy_data) {
    if (enemy_data.empty()) {
        cout << "ERROR: Cannot import enemies as the data is empty\n";
        return false;
    }
    enemyList.reserve(enemy_data.size());
    enemy_config tempConfig; // This config is modified and fed repeatably for adding enemies
    for (int i = 0; i < enemy_data.size(); i++) {
        tempConfig.type = static_cast<enemy_type>(enemy_data[i].type);
        tempConfig.team = static_cast<_team>(enemy_data[i].team);
        tempConfig.maxHP = enemy_data[i].maxHP;
        tempConfig.fireRate = enemy_data[i].fireRate;
        tempConfig.slewRate = enemy_data[i].slewRate;
        tempConfig.detectionRadius = enemy_data[i].detectionRadius;

        addEnemy({enemy_data[i].posX,enemy_data[i].posY},tempConfig);
    }
    return true;
}


_enemy* _enemyManager::isColliding(const Vec2f &pos, float registerDistance) const {
    for (int i = 0; i < enemyList.size(); i++) {
        _enemy* enemy = enemyList[i].get();
        bool hit = false;
        if (_collisionBound* cb = enemy->getCollisionBound()) {
            Vec2f boxPos = cb->getPos();
            Vec2f size = cb->getSize();
            float dx = std::fabs(pos.x - boxPos.x);
            float dy = std::fabs(pos.y - boxPos.y);
            hit = (dx <= size.x * 0.5f + registerDistance) &&
                  (dy <= size.y * 0.5f + registerDistance);
        } else {
            hit = enemy->pos.distance(pos) <= registerDistance;
        }
        if (hit) {
            particleManager->spawnEffect(enemy->pos,turret_hit_effect);
            return enemy;
        }
    }
    return nullptr;
}

int _enemyManager::getNumEnemies() {
    return enemyList.size();
}

// -- PRIVATE -- //

void _enemyManager::buildVAO() {
    glBindVertexArray(vaoID);

    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID);

    GLsizei stride = 7 * sizeof(float);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,stride,(void*)(0 * sizeof(float))); // Size (vec2)

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,stride,(void*)(2 * sizeof(float))); // Tex Coords (vec2)

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,stride,(void*)(4 * sizeof(float))); // Center position (vec2)

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,stride,(void*)(6 * sizeof(float))); // Angle (float)

    glBindVertexArray(0);
}