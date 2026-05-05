#include <_fob.h>

_fob::_fob() {

}

_fob::~_fob() {

}

void _fob::initFob(_player* currentPlayer, _lightManager* currentLightManager) {
    player = currentPlayer;
    sceneLightManager = currentLightManager;

    // PARTICLES //
    particleManager->initParticleManager("images/fob/particles.png",2,sceneLightManager);

    // SPRITES //
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

void _fob::updateFob(double dt) {
    particleManager->updateParticleManger(dt);

    float distanceToPlayer = pos.distance(player->pos);
    if (distanceToPlayer < 128.0f) {
        if (resupplyDt > resupplyTime) {
            player->resupply(10,15);
            resupplyDt = 0;
        } else {
            resupplyDt += dt;
        }
    } else {
        resupplyDt = 0;            
    }
}

void _fob::drawFob() {
    drawUnit();
    particleManager->drawParticleManager();
}