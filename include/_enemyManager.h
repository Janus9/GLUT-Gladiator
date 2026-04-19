#ifndef _ENEMY_MANAGER_H
#define _ENEMY_MANAGER_H

#include <_common.h>
#include <_enemy.h>

class _enemyManager {
    public:
        _enemyManager();
        virtual ~_enemyManager();
    protected:
    private:
        vector<_enemy> enemyList;
};

#endif // _ENEMY_MANAGER_H
