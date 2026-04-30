#include <_fob.h>

_fob::_fob() {

}

_fob::~_fob() {

}

void _fob::initFob(_player* currentPlayer) {
    player = currentPlayer;
    setCollisionBox({32.0f,32.0f});
    setupSprite("MAIN");
    _sprite* fob_sprite = getSprite("MAIN");
    if (fob_sprite) {
        fob_sprite->initSprite("images/fob/idle.png",8,1,sprite_direction::LEFT,12);
        fob_sprite->createSpriteAction(sprite_action("IDLE",0,0,7));
        fob_sprite->loadSpriteAction("IDLE");
        fob_sprite->startAnimation();
    }
}
