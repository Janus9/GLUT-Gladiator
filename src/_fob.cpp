#include <_fob.h>

_fob::_fob() {

}

_fob::~_fob() {

}

void _fob::initFob(_player* currentPlayer, _lightManager* currentLightManager,_sounds* currentSoundManager) {
    player = currentPlayer;
    sceneLightManager = currentLightManager;
    sceneSoundManager = currentSoundManager;

    // PARTICLES //
    particleManager->initParticleManager("images/fob/particles.png",2,sceneLightManager);
    // Death 1 effect //
    death_1_effect.amount = 0;
    death_1_effect.imageIndex = 0;

    death_1_effect.minVelX = -16.0f;
    death_1_effect.maxVelX = 16.0f;
    death_1_effect.minVelY = 30.0f;
    death_1_effect.maxVelY = 55.0f;

    death_1_effect.minRadius = 1.5f;
    death_1_effect.maxRadius = 3.5f;
    
    death_1_effect.minLifeTime = 7.0f;
    death_1_effect.maxLifeTime = 7.0f;
    
    death_1_effect.minSpawnOffsetX = -12.0f;
    death_1_effect.maxSpawnOffsetX = 12.0f;
    death_1_effect.minSpawnOffsetY = -12.0f;
    death_1_effect.maxSpawnOffsetY = 12.0f;
    
    death_1_effect.minRotation = 30.0f;
    death_1_effect.maxRotation = 200.0f;
    
    death_1_effect.hasGravity = true;
    death_1_effect.hasFloor = true;
    
    death_1_effect.floorOffset = 0.0f;

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
    particleManager->updateParticleManger(dt);
    sceneSoundManager->playSfx3DLooped("FOB_AMBIENT",0,pos);

    if (player->playerRespawnedEvent) {
        evaluatePlayer();
        player->playerRespawnedEvent = false;
    }

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

void _fob::evaluatePlayer() {
    _sprite* fob_sprite = getSprite("MAIN");
    if (!fob_sprite) {
        cout << "ERROR: Cannot get FOB sprite\n";
        return;
    }

    if (player->lives <= -1) {
        // Death Event //
        fob_sprite->setIdleFrame(14,4);
        fob_sprite->playAction("DEATH");
        particleManager->spawnEffect(pos,death_1_effect);
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
    particleManager->drawParticleManager();
}