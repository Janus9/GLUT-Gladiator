#include <_enemy.h>

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
        main_sprite->initSprite("images/m2_50.png",4,1,sprite_direction::LEFT,12);
        main_sprite->createSpriteAction(sprite_action("SHOOT",0,0,3));
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