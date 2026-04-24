#include <_enemyManager.h>

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

void _enemy::initEnemy(enemy_type type) {

    switch (type) {
        // -- DEFAULT TURRET -- //
        case ENEMY_TURRET: {
            // Data Setup //
            setMaxHealth(75.0f);
            resetHealth();

            fireRate = 300.0f;
            slewRate = 90.0f;
            bulletDamage = 20.0f;
            bulletPenetration = 50.0f;

            detectionRadius = 256.0f;

            eType = type;
            // Sprite Setup //
            setupSprite("MAIN");
            _sprite* main_sprite = getSprite("MAIN");
            if (main_sprite) {
                main_sprite->initSprite("images/enemy/turret.png",4,2,sprite_direction::LEFT,12);
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
            setMaxHealth(400.0f);
            resetHealth();

            fireRate = 1200.0f;
            slewRate = 45.0f;
            bulletDamage = 5.0f;
            bulletPenetration = 0.0f;
            
            detectionRadius = 400.0f;

            eType = type;
            // Sprite Setup //
            setupSprite("BASE");
            _sprite* base_sprite = getSprite("BASE");
            if (base_sprite) {
                base_sprite->initSprite("images/enemy/gatling_gun/gatling_base.png",1,1,sprite_direction::LEFT,12);
                base_sprite->setIdleFrame(0,0);
                base_sprite->stopAnimation();
            }

            setupSprite("TURRET");
            _sprite* turret_sprite = getSprite("TURRET");
            if (turret_sprite) {
                turret_sprite->initSprite("images/enemy/gatling_gun/gatling_turret.png",9,2,sprite_direction::LEFT,12);
                turret_sprite->createSpriteAction(sprite_action("SHOOT",0,0,3));
                turret_sprite->createSpriteAction(sprite_action("DEATH",1,0,8));
                turret_sprite->loadSpriteAction("SHOOT");
                turret_sprite->setIdleFrame(0,0);
                turret_sprite->stopAnimation();
                Vec2f size = turret_sprite->getSize();
                turret_sprite->pivotPoint = {size.x/2.0f, size.y/2.0f};
            }
            break;
        }
    }
}

bool _enemy::operator==(const _enemy &other) const {
    return enemyID == other.enemyID;
}

// -- -- ENEMY MANAGER -- -- //

// -- PUBLIC -- //

_enemyManager::_enemyManager() {
    // ctor
}

_enemyManager::~_enemyManager() {
    // dtor
}

void _enemyManager::initEnemyManager(_player* currentPlayer, _world* currentWorld, _bulletManager* currentBulletManager,_bullet_config* _bullet_1, _sounds* currentSounds) {
    player = currentPlayer;
    world = currentWorld;
    bulletManager = currentBulletManager;
    bullet_1 = _bullet_1;
    sounds = currentSounds;

    particleManager->initParticleManager("images/enemy/hit_particle.png");
    // turret hit effect
    turret_hit_effect.amount = 15;

    turret_hit_effect.minVelX = -8.0f;
    turret_hit_effect.maxVelX = 8.0f;
    turret_hit_effect.minVelY = 10.0f;
    turret_hit_effect.maxVelY = 25.0f;

    turret_hit_effect.minRadius = 1.0f;
    turret_hit_effect.maxRadius = 2.0f;

    turret_hit_effect.minLifeTime = 0.5f;
    turret_hit_effect.maxLifeTime = 1.1f;

    turret_hit_effect.minSpawnOffsetX = -3.0f;
    turret_hit_effect.maxSpawnOffsetX = 3.0f;
    turret_hit_effect.minSpawnOffsetY = -3.0f;
    turret_hit_effect.maxSpawnOffsetY = 3.0f;
}

void _enemyManager::updateEnemies(double dt) {
    // Iterate backwards to removal safety
    particleManager->updateParticleManger(dt);
    if (enemyList.size() <= 0) return; // Empty list - no need to run loop
    if (!player || player->isDead()) return; // No player, or player is dead
    for (int i = enemyList.size()-1; i >= 0; i--) {
        _enemy* enemy = enemyList[i].get();
        switch(enemy->eType) {
            
            // -- DEFAULT TURRET -- //
            case ENEMY_TURRET: {
                if (enemy->isDead() && !enemy->inDeathAnimation) {
                    enemy->inDeathAnimation = true;
                    enemy->getSingleSprite()->setFPS(4);
                    enemy->getSingleSprite()->setIdleFrame(3,1);
                    enemy->getSingleSprite()->playAction("DEATH");
                    enemy->deathTime = 0.0;
                    if (sounds) sounds->playSfx("ENEMY_DEATH");
                    continue;
                } else if (enemy->isDead() && enemy->deathTime > 5.0f) {
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
                            if (sounds) sounds->playSfx("ENEMY_SHOOT");
                            enemy->getSingleSprite()->setFPS(enemy->fireRate / 60.0f);
                            enemy->firingTime = 0;
                        }
                        enemy->getSingleSprite()->startAnimation();
                    } else {
                        enemy->getSingleSprite()->setFPS(12);
                        enemy->getSingleSprite()->stopAnimation();
                    }
                }  else {
                    enemy->getSingleSprite()->setFPS(12);
                    enemy->getSingleSprite()->stopAnimation();
                }
                break;
            }

            // -- GATLING TURRET -- //
            case ENEMY_GATLING: {
                _sprite* sprite = enemy->getSprite("TURRET");
                if (enemy->isDead() && !enemy->inDeathAnimation) {
                    enemy->inDeathAnimation = true;
                    sprite->setFPS(12);
                    sprite->setIdleFrame(1,8);
                    sprite->playAction("DEATH");
                    enemy->deathTime = 0.0;
                    if (sounds) sounds->playSfx("ENEMY_DEATH");
                    continue;
                } else if (enemy->isDead() && enemy->deathTime > 6.0f) {
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
                    bool focused = enemy->focusOn(player->pos,enemy->slewRate,5.0f,sprite);
                    if (focused) {
                        // Focused on player -- ready to fire
                        enemy->firingTime += dt;
                        if (enemy->firingTime > 1.0f/(enemy->fireRate/60.0f)) {
                            bulletManager->spawnBulletEffect(enemy->pos,player->pos,_team::ENEMY,*bullet_1);
                            if (sounds) sounds->playSfx("ENEMY_SHOOT");
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
        }
    }
}

void _enemyManager::drawEnemies() {
    for (int i = 0; i < enemyList.size(); i++) {
        switch(enemyList[i]->eType) {
            case ENEMY_TURRET:
                enemyList[i]->drawUnitSingular();
                break;
            case ENEMY_GATLING:
                enemyList[i]->drawUnit();
                break;
        }
    }
    particleManager->drawParticleManager();
}

void _enemyManager::addEnemy(const Vec2f &_pos, enemy_type type) {
    unique_ptr<_enemy> newEnemy = make_unique<_enemy>();
    newEnemy->initEnemy(type);
    newEnemy->pos = _pos;
    enemyList.push_back(move(newEnemy));
}

_enemy* _enemyManager::isColliding(const Vec2f &pos, float registerDistance) const {
    for (int i = 0; i < enemyList.size(); i++) {
        _enemy* enemy = enemyList[i].get();
        float distance = enemy->pos.distance(pos);
        if (distance <= registerDistance) {
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