#include <_fob.h>

_fob::_fob() {

}

_fob::~_fob() {

}

void _fob::initFob(
    _player* currentPlayer, 
    _lightManager* currentLightManager,
    _sounds* currentSoundManager,
    particles::Engine* particles
) {
    ParticleEngine = particles;
    player = currentPlayer;
    sceneLightManager = currentLightManager;
    sceneSoundManager = currentSoundManager;

    // SPRITES //
    setCollisionBox({32.0f,32.0f});
    setupSprite("MAIN");
    _sprite* fob_sprite = getSprite("MAIN");
    if (fob_sprite) {
        fob_sprite->initSprite("images/fob/fob.png",15,5,sprite_direction::LEFT,12);
        fob_sprite->createSpriteAction(sprite_action("IDLE_HP_3",0,0,7));
        fob_sprite->createSpriteAction(sprite_action("IDLE_HP_2",1,0,7));
        fob_sprite->createSpriteAction(sprite_action("IDLE_HP_1",2,0,7));
        fob_sprite->createSpriteAction(sprite_action("IDLE_HP_0",3,0,7));
        fob_sprite->createSpriteAction(sprite_action("DEATH",4,0,14));
    }

    evaluatePlayer();
}

void _fob::updateFob(double dt) {
    sceneSoundManager->playSfx3DLooped("FOB_AMBIENT",0,pos);

    if (player->playerRespawnedEvent) {
        evaluatePlayer();
        player->playerRespawnedEvent = false;
    }

    float distanceToPlayer = pos.distance(player->pos);
    if (distanceToPlayer < 128.0f) {
        if (resupplyDt > resupplyTime && !player->isDead() && !player->isRealDead) {
            player->resupply(10,15);
            resupplyDt = 0;
        } else {
            resupplyDt += dt;
        }
    } else {
        resupplyDt = 0;            
    }
}

void _fob::evaluatePlayer() {
    _sprite* fob_sprite = getSprite("MAIN");
    if (!fob_sprite) {
        std::cout << "ERROR: Cannot get FOB sprite\n";
        return;
    }

    if (player->lives <= -1) {
        // Death Event //
        ParticleEngine->spawnEffect({pos.x, pos.y}, "fob_death_1");
        ParticleEngine->spawnEffect({pos.x, pos.y}, "fob_death_2");
        ParticleEngine->spawnEffect({pos.x, pos.y}, "fob_death_3");
        ParticleEngine->spawnEffect({pos.x, pos.y}, "fob_death_4");
        fob_sprite->setIdleFrame(14,4);
        fob_sprite->playAction("DEATH");
        sceneSoundManager->playSfx("ENEMY_DEATH");
        return;
    }

    switch (player->lives) {
        case 3:
            fob_sprite->loadSpriteAction("IDLE_HP_3");
            break;
        case 2:
            fob_sprite->loadSpriteAction("IDLE_HP_2");
            break;
        case 1:
            fob_sprite->loadSpriteAction("IDLE_HP_1");
            break;
        case 0:
            fob_sprite->loadSpriteAction("IDLE_HP_0");
            break;
    }
    fob_sprite->startAnimation();
}

void _fob::drawFob() {
    drawUnit();
}