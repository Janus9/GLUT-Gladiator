#include <_enemyManager.h>


// -- PUBLIC -- //

_enemyManager::_enemyManager() {
    // ctor
}

_enemyManager::~_enemyManager() {
    // dtor
}

void _enemyManager::initEnemyManager(_player* currentPlayer) {
    player = currentPlayer;
}

void _enemyManager::updateEnemies(double dt) {
    // Iterate backwards to removal safety
    if (enemyList.size() <= 0) return; // Empty list - no need to run loop
    if (!player) return; // No player, required for updation of enemies
    for (int i = enemyList.size()-1; i >= 0; i--) {
        _enemy* enemy = enemyList[i].get();
        if (enemy->isDead()) {
            enemyList.erase(enemyList.begin() + i);
            continue;
        }
        float distance = enemy->pos.distance(player->pos);
        if (distance < 256.0f) {
            // Enemy in range
            enemy->focusOn(player->pos,45.0f);
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

int _enemyManager::getNumEnemies() {
    return enemyList.size();
}

// -- PRIVATE -- //