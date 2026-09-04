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
        case ENEMY_ORC:
            SDL_LogWarn(LOG_ENEMY_MANAGER, "WARNING: Not yet implemented");
            break;
        case ENEMY_VAMPIRE:
            SDL_LogWarn(LOG_ENEMY_MANAGER, "WARNING: Not yet implemented");
            break;
        case ENEMY_VAMPIRE_MINION1:
            SDL_LogWarn(LOG_ENEMY_MANAGER, "WARNING: Not yet implemented");
            break;
        case ENEMY_VAMPIRE_MINION2:
            SDL_LogWarn(LOG_ENEMY_MANAGER, "WARNING: Not yet implemented");
            break;
        default:
            SDL_LogError(LOG_ENEMY_MANAGER, "ERROR: Unable to initialize an enemy");
            break;
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

_enemyManager::_enemyManager() : rng(std::random_device{}()) {
    
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

void _enemyManager::initEnemyManager(enemyManagerContext& context) {
    if (!context.validate()) {
        SDL_LogError(LOG_ENEMY_MANAGER, "ERROR: Enemy Manager Context was not able to be validated");
    }
    
    player = context.player;
    world = context.world;
    bulletManager = context.bullets;
    sounds = context.sounds;
    sceneLightManager = context.lights;
    sceneTextureManager = context.textures;
    scenePickupManager = context.pickups;
    ParticleEngine = context.particles;

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
}

void _enemyManager::updateEnemies(double dt) {
    time += dt;

    std::uniform_real_distribution<float> roll(0.0f,1.0f);
    std::uniform_real_distribution<float> pos_dist(-8.0f,8.0f);

    // Iterate backwards to removal safety
    if (enemyList.size() <= 0) return; // Empty list - no need to run loop
    if (!player) return; 
    for (int i = enemyList.size()-1; i >= 0; i--) {
        _enemy* enemy = enemyList[i].get();
        enemy->setPosition(enemy->pos);
        enemy->updateUnit(dt);
        
        // -- SOUND -- //
        switch (enemy->eType) {
            case ENEMY_GATLING:
                sounds->updateSpatialLooped("GATLING_REV", enemy->getID(), enemy->pos);
                sounds->updateSpatialLooped("GATLING_FIRE", enemy->getID(), enemy->pos);
                break;
            case ENEMY_ORC:
                sounds->updateSpatialLooped("ORC_WALK", enemy->getID(), enemy->pos);
                break;
            case ENEMY_TURRET:
                break;
            case ENEMY_VAMPIRE:
                break;
            case ENEMY_VAMPIRE_MINION1:
                break;
            case ENEMY_VAMPIRE_MINION2:
                break;
            default:
                break;
        }

        // Kill enemy event //
        if (enemy->isDead() && enemy->deathTime > enemy->timeInDeathAnimation) {
            // Get a list of sprites registered to the enemy
            const std::vector<_sprite*>& enemySpriteList = enemy->getSpriteList();
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
                        std::vector<_sprite*> &spriteVector = mapIt->second;      // Pull vector out of the map
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
                    continue;
                }

                if (enemy->isDead() && !enemy->inDeathAnimation) {
                    enemy->inDeathAnimation = true;
                    sprite->setFPS(4);
                    sprite->setIdleFrame(3,1);
                    sprite->playAction("DEATH");
                    enemy->deathTime = 0.0;
                    ParticleEngine->spawnEffect({enemy->pos.x, enemy->pos.y}, "turret_death");
                    sounds->playSound("ENEMY_DEATH", enemy->pos);
                    continue;
                } else if (enemy->isDead() && enemy->deathTime > enemy->timeInDeathAnimation) {
                    Vec2f offset_pos = {pos_dist(rng), pos_dist(rng)};
                    
                    scenePickupManager->add(enemy->pos + offset_pos, pickups::PICKUP_XP, 5.0f);
                    
                    if (roll(rng) > 0.5) {
                        offset_pos = {pos_dist(rng), pos_dist(rng)};
                        scenePickupManager->add(enemy->pos + offset_pos, pickups::PICKUP_AMMO, 20.0f);
                    }
                    
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
                            ParticleEngine->spawnEffect({enemy->pos.x, enemy->pos.y}, "turret_bullet_casing");
                            // if (sounds) sounds->playSfx3D("ENEMY_SHOOT", enemy->pos);
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
                    continue;
                }

                if (enemy->isDead() && !enemy->inDeathAnimation) {
                    orc->triggerDeath(sounds);
                    continue;
                } else if (enemy->isDead() && enemy->deathTime > enemy->timeInDeathAnimation) {
                    Vec2f offset_pos = {pos_dist(rng), pos_dist(rng)};

                    for (int i = 0; i < 2; i++) {
                        offset_pos = {pos_dist(rng), pos_dist(rng)};
                        scenePickupManager->add(enemy->pos + offset_pos, pickups::PICKUP_XP, 4.0f);
                    }

                    if (roll(rng) > 0.7f) {
                        offset_pos = {pos_dist(rng), pos_dist(rng)};
                        scenePickupManager->add(enemy->pos + offset_pos, pickups::PICKUP_HEALTH, 15.0f);
                    }

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
                    continue;
                }

                if (enemy->isDead() && !enemy->inDeathAnimation) {
                    vampire->triggerDeath(sounds);
                    continue;
                } else if (enemy->isDead() && enemy->deathTime > enemy->timeInDeathAnimation) {
                    if (vampire->isBoss) {
                        bossKilledEvent = true;
                    }
                    enemyList.erase(enemyList.begin() + i);
                    sounds->resumeSpatialLooped("ORC_WALK", enemy->getID());
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
                    continue;
                }

                // Initial death event
                if (enemy->isDead() && !enemy->inDeathAnimation) {
                    enemy->inDeathAnimation = true;
                    sprite->setFPS(12);
                    sprite->setIdleFrame(8,1);
                    sprite->playAction("DEATH");
                    enemy->deathTime = 0.0;
                    sounds->playSound("ENEMY_DEATH", enemy->pos);
                    ParticleEngine->spawnEffect({enemy->pos.x, enemy->pos.y}, "gatling_death");
                    ParticleEngine->spawnEffect({enemy->pos.x, enemy->pos.y}, "gatling_death_smoke");
                    
                    sounds->stopSpatialLooped("GATLING_REV", enemy->getID());
                    sounds->stopSpatialLooped("GATLING_FIRE", enemy->getID());
                    
                    continue;
                // Final death event (removes enemy)
                } else if (enemy->isDead() && enemy->deathTime > enemy->timeInDeathAnimation) {
                    Vec2f offset_pos = {pos_dist(rng), pos_dist(rng)};
                    
                    for (int i = 0; i < 4; i++) {
                        offset_pos = {pos_dist(rng), pos_dist(rng)};
                        scenePickupManager->add(offset_pos + enemy->pos, pickups::PICKUP_XP, 6.0f);
                    }

                    if (roll(rng) > 0.8f) {
                        offset_pos = {pos_dist(rng), pos_dist(rng)};
                        scenePickupManager->add(offset_pos + enemy->pos, pickups::PICKUP_FIRERATE, 10.0f);
                    }

                    if (roll(rng) > 0.2f) {
                        offset_pos = {pos_dist(rng), pos_dist(rng)};
                        scenePickupManager->add(offset_pos + enemy->pos, pickups::PICKUP_AMMO, 40.0f);
                    }

                    if (roll(rng) > 0.2f) {
                        offset_pos = {pos_dist(rng), pos_dist(rng)};
                        scenePickupManager->add(offset_pos + enemy->pos, pickups::PICKUP_HEALTH, 20.0f);
                    }

                    enemyList.erase(enemyList.begin() + i);
                    continue;
                }

                // If dead update death time to final death event
                if (enemy->isDead()) {
                    enemy->deathTime += dt;
                    continue;
                }

                sounds->updateSpatialLooped("GATLING_FIRE", enemy->getID(), enemy->pos);
                sounds->updateSpatialLooped("GATLING_REV", enemy->getID(), enemy->pos);

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
                            ParticleEngine->spawnEffect({enemy->pos.x, enemy->pos.y}, "gatling_bullet_casing");
                            enemy->firingTime = 0;
                        }
                        sounds->resumeSpatialLooped("GATLING_FIRE", enemy->getID());
                        sounds->pauseSpatialLooped("GATLING_REV", enemy->getID());
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
                        sounds->resumeSpatialLooped("GATLING_REV", enemy->getID());
                        sounds->pauseSpatialLooped("GATLING_FIRE", enemy->getID());
                    }
                }  else {
                    enemy->revTime = 0.0;
                    sprite->loadSpriteAction("IDLE");
                    sprite->setFPS(12);
                    sounds->pauseSpatialLooped("GATLING_REV", enemy->getID());
                    sounds->pauseSpatialLooped("GATLING_FIRE", enemy->getID());
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
            const std::vector<_sprite*> &spriteVector = it->second;    // List of sprites mapped to texture ID

            // cout << "Texture ID: " << textureID << "\n";
            // cout << " - Num Sprites: " << spriteVector.size() << "\n";

            if (spriteVector.size() <= 0) {
                // Texture ID is empty, skip
                continue;
            } 

            // BUILD BATCH VBO //
            const int spriteBatchCount = spriteVector.size();
            std::vector<float> vboData(spriteBatchCount * 7 * 4);
            int vIndex = 0;

            for (auto &sprite : spriteVector) {
                if (!sprite->hidden && sprite->ocapacity > 0.0f) { // Ignore hidden sprites
                    sprite->buildSpriteVBO(vboData.data(),vIndex);
                }
            }

            glBindBuffer(GL_ARRAY_BUFFER, vboID);
            glBufferData(GL_ARRAY_BUFFER, vboData.size() * sizeof(float), vboData.data(), GL_DYNAMIC_DRAW);

            // BUILD BATCH EBO //
            std::vector<uint32_t> eboData(spriteBatchCount * 6);
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
}

void _enemyManager::addEnemy(const Vec2f &_pos, const enemy_config &config) {
    std::unique_ptr<_enemy> newEnemy;
    if (config.type == ENEMY_ORC) {
        std::unique_ptr<_orc> orc = std::make_unique<_orc>();
        orc->initOrc(sceneTextureManager);
        newEnemy = move(orc);
        sounds->createSpatialLooped("ORC_WALK",newEnemy->getID(), newEnemy->pos);
    } else if (config.type == ENEMY_VAMPIRE) {
        std::unique_ptr<_vampire> vampire = std::make_unique<_vampire>();
        vampire->initVampire(sceneTextureManager, VAMPIRE_BOSS);
        newEnemy = move(vampire);
    } else if (config.type == ENEMY_VAMPIRE_MINION1) {
        std::unique_ptr<_vampire> vampire = std::make_unique<_vampire>();
        vampire->initVampire(sceneTextureManager, VAMPIRE_MINION1);
        newEnemy = move(vampire);
    } else if (config.type == ENEMY_VAMPIRE_MINION2) {
        std::unique_ptr<_vampire> vampire = std::make_unique<_vampire>();
        vampire->initVampire(sceneTextureManager, VAMPIRE_MINION2);
        newEnemy = move(vampire);
    } else {
        newEnemy = std::make_unique<_enemy>();
        newEnemy->initEnemy(config, sceneTextureManager);
        if (config.type == ENEMY_GATLING) {
            sounds->createSpatialLooped("GATLING_REV",newEnemy->getID(), newEnemy->pos);
            sounds->createSpatialLooped("GATLING_FIRE",newEnemy->getID(), newEnemy->pos);
        }
    }
    newEnemy->pos = _pos;

    const std::vector<_sprite*>& enemySpriteList = newEnemy->getSpriteList();
    for (const auto &sprite : enemySpriteList) {
        const GLuint textureID = sprite->getTextureID();
        const int layer = sprite->getLayer(); 

        layerMap[layer][textureID].push_back(sprite);
    }

    spriteCount += newEnemy->getNumSprites();   // Increase sprite count

    // Push_back last to avoid weird move stuff
    enemyList.push_back(move(newEnemy));
}

std::vector<enemy_serial_data> _enemyManager::exportSerializedEnemies() const {
    std::vector<enemy_serial_data> enemy_data;
    for (size_t i = 0; i < enemyList.size(); i++) {
        const _enemy* enemy = enemyList[i].get();
        if (enemy->inDeathAnimation || enemy->isDead()) continue; // Skip dead enemies
        enemy_data.push_back(enemy->serializeEnemy());
    }
    return enemy_data;
}

bool _enemyManager::importSerializedEnemies(const std::vector<enemy_serial_data> &enemy_data) {
    if (enemy_data.empty()) {
        std::cout << "ERROR: Cannot import enemies as the data is empty\n";
        return false;
    }
    enemyList.reserve(enemy_data.size());
    enemy_config tempConfig; // This config is modified and fed repeatably for adding enemies
    for (size_t i = 0; i < enemy_data.size(); i++) {
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
    for (size_t i = 0; i < enemyList.size(); i++) {
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
            ParticleEngine->spawnEffect({enemy->pos.x, enemy->pos.y}, "turret_hit");
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