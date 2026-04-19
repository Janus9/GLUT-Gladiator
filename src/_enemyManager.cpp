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

void _enemy::initEnemy() {
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

void _enemyManager::initEnemyManager(_player* currentPlayer, _world* currentWorld, _bulletManager* currentBulletManager,_bullet_config* _bullet_1) {
    player = currentPlayer;
    world = currentWorld;
    bulletManager = currentBulletManager;
    bullet_1 = _bullet_1;
}

void _enemyManager::updateEnemies(double dt) {
    // Iterate backwards to removal safety
    if (enemyList.size() <= 0) return; // Empty list - no need to run loop
    if (!player || player->isDead()) return; // No player, or player is dead
    for (int i = enemyList.size()-1; i >= 0; i--) {
        _enemy* enemy = enemyList[i].get();
        enemy->firingTime += dt;
        enemy->deathTime += dt;
        if (enemy->isDead() && !enemy->inDeathAnimation) {
            enemy->inDeathAnimation = true;
            enemy->getSingleSprite()->setFPS(4);
            enemy->getSingleSprite()->setIdleFrame(3,1);
            enemy->getSingleSprite()->playAction("DEATH");
            enemy->deathTime = 0.0;
            continue;
        } else if (enemy->isDead() && enemy->deathTime > 5.0f) {
            enemyList.erase(enemyList.begin() + i);
            continue;
        }
        if (enemy->isDead()) {
            continue;
        }
        float distance = enemy->pos.distance(player->pos);
        if (distance < 256.0f) {
            // Enemy in range
            bool focused = enemy->focusOn(player->pos,45.0f);
            if (focused) {
                if (enemy->firingTime > 1.0f/(enemy->fireRate/60.0f)) {
                    bulletManager->spawnBulletEffect(enemy->pos,player->pos,_team::ENEMY,*bullet_1);
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
    }
}

void _enemyManager::drawEnemies() {
    for (int i = 0; i < enemyList.size(); i++) {
        enemyList[i]->drawUnitSingular();
    }
}

void _enemyManager::addEnemy(const Vec2f &_pos) {
    unique_ptr<_enemy> newEnemy = make_unique<_enemy>();
    newEnemy->initEnemy();
    newEnemy->pos = _pos;
    enemyList.push_back(move(newEnemy));
}

_enemy* _enemyManager::isColliding(const Vec2f &pos, float registerDistance) const {
    for (int i = 0; i < enemyList.size(); i++) {
        _enemy* enemy = enemyList[i].get();
        float distance = enemy->pos.distance(pos);
        if (distance <= registerDistance) {
            return enemy;
        }
    }
    return nullptr;
}

int _enemyManager::getNumEnemies() {
    return enemyList.size();
}

// -- PRIVATE -- //