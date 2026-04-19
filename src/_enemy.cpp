#include <_enemy.h>

_enemy::_enemy() {
    // ctor
}

_enemy::~_enemy() {
    // dtor
}

void _enemy::initEnemy() {
    setupSprite("MAIN");
    _sprite* main_sprite = getSprite("MAIN");
    if (main_sprite) {
        main_sprite->initSprite("images/m2_50.png",4,1,sprite_direction::LEFT,12);
        main_sprite->createSpriteAction(sprite_action("SHOOT",0,0,3));
        main_sprite->loadSpriteAction("SHOOT");
        main_sprite->stopAnimation();
        setSingleSprite(main_sprite); // Only one sprite
    }
}