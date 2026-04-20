#include <_player.h>

_player::_player() {
    // ctor
}

_player::~_player() {
    // dtor
}

void _player::initPlayer() {
    scale = {0.8f, 0.8f};
    pos = {0.0f, 0.0f}; // Start player in the center of the screen

    bloodParticles->initParticleManager("images/player/blood_particle.png");
    player_hit_effect.amount = 15;

    player_hit_effect.minVelX = -8.0f;
    player_hit_effect.maxVelX = 8.0f;
    player_hit_effect.minVelY = 10.0f;
    player_hit_effect.maxVelY = 25.0f;

    player_hit_effect.minRadius = 2.0f;
    player_hit_effect.maxRadius = 3.5f;

    player_hit_effect.minLifeTime = 0.5f;
    player_hit_effect.maxLifeTime = 1.1f;

    player_hit_effect.minSpawnOffsetX = -3.0f;
    player_hit_effect.maxSpawnOffsetX = 3.0f;
    player_hit_effect.minSpawnOffsetY = -3.0f;
    player_hit_effect.maxSpawnOffsetY = 3.0f;

    // -- ANIMATIONS -- //

    setupSprite("NULL");
    _sprite* null_sprite = getSprite("NULL");
    if (null_sprite) {
        null_sprite->initSprite("images/missing_texture.png",1,1, sprite_direction::LEFT,12);

        null_sprite->createSpriteAction(sprite_action("NULL_NULL",0,0,0));
    }

    animationTable[PLAYER_ACTION_NULL][PLAYER_FACE_NULL] = {"NULL","NULL_NULL",{0,0},false};
    currentResult = animationTable[PLAYER_ACTION_NULL][PLAYER_FACE_NULL];
    setSingleSprite(null_sprite);

    // Walk Animation //
    setupSprite("WALK");
    _sprite* walk_sprite = getSprite("WALK");
    if (walk_sprite) {
        walk_sprite->initSprite("images/player/Walk/Normal/walk.png", 8, 6, sprite_direction::LEFT,12); // No natural direction due to top down
    
        walk_sprite->createSpriteAction(sprite_action("WALK_N",3,0,7));
        walk_sprite->createSpriteAction(sprite_action("WALK_NE",4,0,7));
        walk_sprite->createSpriteAction(sprite_action("WALK_E",5,0,7));
        walk_sprite->createSpriteAction(sprite_action("WALK_SE",5,0,7));
        walk_sprite->createSpriteAction(sprite_action("WALK_S",0,0,7));
        walk_sprite->createSpriteAction(sprite_action("WALK_SW",1,0,7));
        walk_sprite->createSpriteAction(sprite_action("WALK_W",1,0,7));
        walk_sprite->createSpriteAction(sprite_action("WALK_NW",2,0,7));
        walk_sprite->offsetPoint = {0.0f, 8.0f};
    }

    animationTable[PLAYER_ACTION_WALK][PLAYER_FACE_N] = {"WALK","WALK_N",{0,3}};
    animationTable[PLAYER_ACTION_WALK][PLAYER_FACE_NE] = {"WALK","WALK_NE",{0,4}};
    animationTable[PLAYER_ACTION_WALK][PLAYER_FACE_E] = {"WALK","WALK_E",{0,5}};
    animationTable[PLAYER_ACTION_WALK][PLAYER_FACE_SE] = {"WALK","WALK_SE",{0,5}};
    animationTable[PLAYER_ACTION_WALK][PLAYER_FACE_S] = {"WALK","WALK_S",{0,0}};
    animationTable[PLAYER_ACTION_WALK][PLAYER_FACE_SW] = {"WALK","WALK_SW",{0,1}};
    animationTable[PLAYER_ACTION_WALK][PLAYER_FACE_W] = {"WALK","WALK_W",{0,2}};
    animationTable[PLAYER_ACTION_WALK][PLAYER_FACE_NW] = {"WALK","WALK_NW",{0,2}};

    // Walk Gun Animation //
    setupSprite("WALK_GUN");
    _sprite* walk_gun_sprite = getSprite("WALK_GUN");
    if (walk_gun_sprite) {
        walk_gun_sprite->initSprite("images/player/Walk/Gun/Walk_Gun.png", 8, 6, sprite_direction::LEFT,12); // No natural direction due to top down
        
        walk_gun_sprite->createSpriteAction(sprite_action("WALK_GUN_N",3,0,7));
        walk_gun_sprite->createSpriteAction(sprite_action("WALK_GUN_NE",4,0,7));
        walk_gun_sprite->createSpriteAction(sprite_action("WALK_GUN_E",5,0,7));
        walk_gun_sprite->createSpriteAction(sprite_action("WALK_GUN_SE",5,0,7));
        walk_gun_sprite->createSpriteAction(sprite_action("WALK_GUN_S",0,0,7));
        walk_gun_sprite->createSpriteAction(sprite_action("WALK_GUN_SW",1,0,7));
        walk_gun_sprite->createSpriteAction(sprite_action("WALK_GUN_W",1,0,7));
        walk_gun_sprite->createSpriteAction(sprite_action("WALK_GUN_NW",2,0,7));
        walk_gun_sprite->offsetPoint = {0.0f, 8.0f};
    }

    animationTable[PLAYER_ACTION_WALK_GUN][PLAYER_FACE_N] = {"WALK_GUN","WALK_GUN_N",{0,3}};
    animationTable[PLAYER_ACTION_WALK_GUN][PLAYER_FACE_NE] = {"WALK_GUN","WALK_GUN_NE",{0,4}};
    animationTable[PLAYER_ACTION_WALK_GUN][PLAYER_FACE_E] = {"WALK_GUN","WALK_GUN_E",{0,5}};
    animationTable[PLAYER_ACTION_WALK_GUN][PLAYER_FACE_SE] = {"WALK_GUN","WALK_GUN_SE",{0,5}};
    animationTable[PLAYER_ACTION_WALK_GUN][PLAYER_FACE_S] = {"WALK_GUN","WALK_GUN_S",{0,0}};
    animationTable[PLAYER_ACTION_WALK_GUN][PLAYER_FACE_SW] = {"WALK_GUN","WALK_GUN_SW",{0,1}};
    animationTable[PLAYER_ACTION_WALK_GUN][PLAYER_FACE_W] = {"WALK_GUN","WALK_GUN_W",{0,2}};
    animationTable[PLAYER_ACTION_WALK_GUN][PLAYER_FACE_NW] = {"WALK_GUN","WALK_GUN_NW",{0,2}};

    // Walk Shoot Animation //
    setupSprite("WALK_SHOOT");
    _sprite* walk_shoot_sprite = getSprite("WALK_SHOOT");
    if (walk_shoot_sprite) {
        walk_shoot_sprite->initSprite("images/player/Walk_while_Shooting - NEW/Walk_while_Shooting.png", 8, 8, sprite_direction::LEFT,12); // No natural direction due to top down
    
        walk_shoot_sprite->createSpriteAction(sprite_action("WALK_SHOOT_N",3,0,7));
        walk_shoot_sprite->createSpriteAction(sprite_action("WALK_SHOOT_NE",4,0,7));
        walk_shoot_sprite->createSpriteAction(sprite_action("WALK_SHOOT_E",6,0,7));
        walk_shoot_sprite->createSpriteAction(sprite_action("WALK_SHOOT_SE",5,0,7));
        walk_shoot_sprite->createSpriteAction(sprite_action("WALK_SHOOT_S",0,0,7));
        walk_shoot_sprite->createSpriteAction(sprite_action("WALK_SHOOT_SW",1,0,7));
        walk_shoot_sprite->createSpriteAction(sprite_action("WALK_SHOOT_W",7,0,7));
        walk_shoot_sprite->createSpriteAction(sprite_action("WALK_SHOOT_NW",2,0,7));
        walk_shoot_sprite->offsetPoint = {0.0f, 8.0f};
    }

    animationTable[PLAYER_ACTION_WALK_SHOOT][PLAYER_FACE_N] = {"WALK_SHOOT","WALK_SHOOT_N",{0,3}};
    animationTable[PLAYER_ACTION_WALK_SHOOT][PLAYER_FACE_NE] = {"WALK_SHOOT","WALK_SHOOT_NE",{0,4}};
    animationTable[PLAYER_ACTION_WALK_SHOOT][PLAYER_FACE_E] = {"WALK_SHOOT","WALK_SHOOT_E",{0,6}};
    animationTable[PLAYER_ACTION_WALK_SHOOT][PLAYER_FACE_SE] = {"WALK_SHOOT","WALK_SHOOT_SE",{0,5}};
    animationTable[PLAYER_ACTION_WALK_SHOOT][PLAYER_FACE_S] = {"WALK_SHOOT","WALK_SHOOT_S",{0,0}};
    animationTable[PLAYER_ACTION_WALK_SHOOT][PLAYER_FACE_SW] = {"WALK_SHOOT","WALK_SHOOT_SW",{0,1}};
    animationTable[PLAYER_ACTION_WALK_SHOOT][PLAYER_FACE_W] = {"WALK_SHOOT","WALK_SHOOT_W",{0,7}};
    animationTable[PLAYER_ACTION_WALK_SHOOT][PLAYER_FACE_NW] = {"WALK_SHOOT","WALK_SHOOT_NW",{0,2}};

    // Idle Animation //
    setupSprite("IDLE");
    _sprite* idle_sprite = getSprite("IDLE");
    if (idle_sprite) {
        idle_sprite->initSprite("images/player/Idle/Normal/idle.png", 8, 6, sprite_direction::LEFT,12); // No natural direction due to top down
    
        idle_sprite->createSpriteAction(sprite_action("IDLE_N",3,0,7));
        idle_sprite->createSpriteAction(sprite_action("IDLE_NE",4,0,7));
        idle_sprite->createSpriteAction(sprite_action("IDLE_E",5,0,7));
        idle_sprite->createSpriteAction(sprite_action("IDLE_SE",5,0,7));
        idle_sprite->createSpriteAction(sprite_action("IDLE_S",0,0,7));
        idle_sprite->createSpriteAction(sprite_action("IDLE_SW",1,0,7));
        idle_sprite->createSpriteAction(sprite_action("IDLE_W",1,0,7));
        idle_sprite->createSpriteAction(sprite_action("IDLE_NW",2,0,7));
        idle_sprite->offsetPoint = {0.0f, 8.0f};
    }

    animationTable[PLAYER_ACTION_IDLE][PLAYER_FACE_N] = {"IDLE","IDLE_N",{0,3}};
    animationTable[PLAYER_ACTION_IDLE][PLAYER_FACE_NE] = {"IDLE","IDLE_NE",{0,4}};
    animationTable[PLAYER_ACTION_IDLE][PLAYER_FACE_E] = {"IDLE","IDLE_E",{0,5}};
    animationTable[PLAYER_ACTION_IDLE][PLAYER_FACE_SE] = {"IDLE","IDLE_SE",{0,5}};
    animationTable[PLAYER_ACTION_IDLE][PLAYER_FACE_S] = {"IDLE","IDLE_S",{0,0}};
    animationTable[PLAYER_ACTION_IDLE][PLAYER_FACE_SW] = {"IDLE","IDLE_SW",{0,1}};
    animationTable[PLAYER_ACTION_IDLE][PLAYER_FACE_W] = {"IDLE","IDLE_W",{0,2}};
    animationTable[PLAYER_ACTION_IDLE][PLAYER_FACE_NW] = {"IDLE","IDLE_NW",{0,2}};

    // Idle Gun Animation //
    setupSprite("IDLE_GUN");
    _sprite* idle_gun_sprite = getSprite("IDLE_GUN");
    if (idle_gun_sprite) {
        idle_gun_sprite->initSprite("images/player/Idle/Gun/Idle_Gun.png", 8, 6, sprite_direction::LEFT,12); // No natural direction due to top down
    
        idle_gun_sprite->createSpriteAction(sprite_action("IDLE_GUN_N",3,0,7));
        idle_gun_sprite->createSpriteAction(sprite_action("IDLE_GUN_NE",4,0,7));
        idle_gun_sprite->createSpriteAction(sprite_action("IDLE_GUN_E",5,0,7));
        idle_gun_sprite->createSpriteAction(sprite_action("IDLE_GUN_SE",5,0,7));
        idle_gun_sprite->createSpriteAction(sprite_action("IDLE_GUN_S",0,0,7));
        idle_gun_sprite->createSpriteAction(sprite_action("IDLE_GUN_SW",1,0,7));
        idle_gun_sprite->createSpriteAction(sprite_action("IDLE_GUN_W",1,0,7));
        idle_gun_sprite->createSpriteAction(sprite_action("IDLE_GUN_NW",2,0,7));
        idle_gun_sprite->offsetPoint = {0.0f, 8.0f};
    }

    animationTable[PLAYER_ACTION_IDLE_GUN][PLAYER_FACE_N] = {"IDLE_GUN","IDLE_GUN_N",{0,3}};
    animationTable[PLAYER_ACTION_IDLE_GUN][PLAYER_FACE_NE] = {"IDLE_GUN","IDLE_GUN_NE",{0,4}};
    animationTable[PLAYER_ACTION_IDLE_GUN][PLAYER_FACE_E] = {"IDLE_GUN","IDLE_GUN_E",{0,5}};
    animationTable[PLAYER_ACTION_IDLE_GUN][PLAYER_FACE_SE] = {"IDLE_GUN","IDLE_GUN_SE",{0,5}};
    animationTable[PLAYER_ACTION_IDLE_GUN][PLAYER_FACE_S] = {"IDLE_GUN","IDLE_GUN_S",{0,0}};
    animationTable[PLAYER_ACTION_IDLE_GUN][PLAYER_FACE_SW] = {"IDLE_GUN","IDLE_GUN_SW",{0,1}};
    animationTable[PLAYER_ACTION_IDLE_GUN][PLAYER_FACE_W] = {"IDLE_GUN","IDLE_GUN_W",{0,2}};
    animationTable[PLAYER_ACTION_IDLE_GUN][PLAYER_FACE_NW] = {"IDLE_GUN","IDLE_GUN_NW",{0,2}};

    // Idle Shoot Animation //
    setupSprite("IDLE_SHOOT");
    _sprite* idle_shoot_sprite = getSprite("IDLE_SHOOT");
    if (idle_shoot_sprite) {
        idle_shoot_sprite->initSprite("images/player/Attack/Gun/Shooting.png", 8, 8, sprite_direction::LEFT,12); // No natural direction due to top down
    
        idle_shoot_sprite->createSpriteAction(sprite_action("IDLE_SHOOT_N",3,0,7));
        idle_shoot_sprite->createSpriteAction(sprite_action("IDLE_SHOOT_NE",4,0,7));
        idle_shoot_sprite->createSpriteAction(sprite_action("IDLE_SHOOT_E",6,0,7));
        idle_shoot_sprite->createSpriteAction(sprite_action("IDLE_SHOOT_SE",5,0,7));
        idle_shoot_sprite->createSpriteAction(sprite_action("IDLE_SHOOT_S",0,0,7));
        idle_shoot_sprite->createSpriteAction(sprite_action("IDLE_SHOOT_SW",1,0,7));
        idle_shoot_sprite->createSpriteAction(sprite_action("IDLE_SHOOT_W",7,0,7));
        idle_shoot_sprite->createSpriteAction(sprite_action("IDLE_SHOOT_NW",2,0,7));
        idle_shoot_sprite->offsetPoint = {0.0f, 8.0f};
    }

    animationTable[PLAYER_ACTION_IDLE_SHOOT][PLAYER_FACE_N] = {"IDLE_SHOOT","IDLE_SHOOT_N",{0,3}};
    animationTable[PLAYER_ACTION_IDLE_SHOOT][PLAYER_FACE_NE] = {"IDLE_SHOOT","IDLE_SHOOT_NE",{0,4}};
    animationTable[PLAYER_ACTION_IDLE_SHOOT][PLAYER_FACE_E] = {"IDLE_SHOOT","IDLE_SHOOT_E",{0,6}};
    animationTable[PLAYER_ACTION_IDLE_SHOOT][PLAYER_FACE_SE] = {"IDLE_SHOOT","IDLE_SHOOT_SE",{0,5}};
    animationTable[PLAYER_ACTION_IDLE_SHOOT][PLAYER_FACE_S] = {"IDLE_SHOOT","IDLE_SHOOT_S",{0,0}};
    animationTable[PLAYER_ACTION_IDLE_SHOOT][PLAYER_FACE_SW] = {"IDLE_SHOOT","IDLE_SHOOT_SW",{0,1}};
    animationTable[PLAYER_ACTION_IDLE_SHOOT][PLAYER_FACE_W] = {"IDLE_SHOOT","IDLE_SHOOT_W",{0,7}};
    animationTable[PLAYER_ACTION_IDLE_SHOOT][PLAYER_FACE_NW] = {"IDLE_SHOOT","IDLE_SHOOT_NW",{0,2}};

    // Death Gun Animation //
    setupSprite("DEATH_GUN");
    _sprite* death_gun_sprite = getSprite("DEATH_GUN");
    if (death_gun_sprite) {
        death_gun_sprite->initSprite("images/player/Death/Gun/death_Gun.png", 8, 6, sprite_direction::LEFT,12); // No natural direction due to top down
    
        death_gun_sprite->createSpriteAction(sprite_action("DEATH_GUN_N",3,0,7));
        death_gun_sprite->createSpriteAction(sprite_action("DEATH_GUN_NE",4,0,7));
        death_gun_sprite->createSpriteAction(sprite_action("DEATH_GUN_E",5,0,7));
        death_gun_sprite->createSpriteAction(sprite_action("DEATH_GUN_SE",5,0,7));
        death_gun_sprite->createSpriteAction(sprite_action("DEATH_GUN_S",0,0,7));
        death_gun_sprite->createSpriteAction(sprite_action("DEATH_GUN_SW",1,0,7));
        death_gun_sprite->createSpriteAction(sprite_action("DEATH_GUN_W",1,0,7));
        death_gun_sprite->createSpriteAction(sprite_action("DEATH_GUN_NW",2,0,7));
        death_gun_sprite->offsetPoint = {0.0f, 8.0f};
    }

    animationTable[PLAYER_ACTION_DEATH_GUN][PLAYER_FACE_N] = {"DEATH_GUN","DEATH_GUN_N",{7,3}};
    animationTable[PLAYER_ACTION_DEATH_GUN][PLAYER_FACE_NE] = {"DEATH_GUN","DEATH_GUN_NE",{7,4}};
    animationTable[PLAYER_ACTION_DEATH_GUN][PLAYER_FACE_E] = {"DEATH_GUN","DEATH_GUN_E",{7,5}};
    animationTable[PLAYER_ACTION_DEATH_GUN][PLAYER_FACE_SE] = {"DEATH_GUN","DEATH_GUN_SE",{7,5}};
    animationTable[PLAYER_ACTION_DEATH_GUN][PLAYER_FACE_S] = {"DEATH_GUN","DEATH_GUN_S",{7,0}};
    animationTable[PLAYER_ACTION_DEATH_GUN][PLAYER_FACE_SW] = {"DEATH_GUN","DEATH_GUN_SW",{7,1}};
    animationTable[PLAYER_ACTION_DEATH_GUN][PLAYER_FACE_W] = {"DEATH_GUN","DEATH_GUN_W",{7,1}};
    animationTable[PLAYER_ACTION_DEATH_GUN][PLAYER_FACE_NW] = {"DEATH_GUN","DEATH_GUN_NW",{7,2}};
}

void _player::updatePlayer(double dt) {
    bloodParticles->updateParticleManger(dt);
    if (inDeathAnimation) {
        deathTimeElapsed += dt;
    }
}

void _player::drawPlayer() {
    drawUnitSingular();
    bloodParticles->drawParticleManager();
}

void _player::setAction(player_action action, player_face face) {
    // cout << "Action: " << action << " and Face: " << face << "\n";
    if (inDeathAnimation) return; // Player is dead, skip animation procs
    if (currentResult == animationTable[action][face]) return; // Skip result the same
    PlayerAnimationResult result = getAnimationResult(action,face);
    if (result.valid) {
        string s_sprite = result.sprite;
        string s_action = result.action;
        Vec2i idleFrame = result.idleFrame;
        
        _sprite* sprite = getSprite(s_sprite);
        if (sprite) {
            sprite->setFPS(FPS);
            sprite->loadSpriteAction(s_action);
            sprite->setIdleFrame(idleFrame.x,idleFrame.y);
            sprite->startAnimation();
            setSingleSprite(sprite);
        }
    } else {
        setSingleSprite(getSprite(result.sprite));
    }
    
    currentResult = result;
}

void _player::stopAction(player_action action) {
    _sprite* sprite = getSprite(currentResult.sprite);
    if (sprite) {
        sprite->stopAnimation();
    }
}

void _player::setAnimationFPS(int _FPS) {
    FPS = _FPS;
}

void _player::handlePlayerDeath(player_face face) {
    if (inDeathAnimation) return; // Already in player death action -- skip
    PlayerAnimationResult result = getAnimationResult(PLAYER_ACTION_DEATH_GUN,face);
    if (result.valid) {
        inDeathAnimation = true;
        string s_sprite = result.sprite;
        string s_action = result.action;
        Vec2i idleFrame = result.idleFrame;
        
        _sprite* sprite = getSprite(s_sprite);
        if (sprite) {
            setSingleSprite(sprite);
            sprite->setFPS(8);
            sprite->setIdleFrame(idleFrame.x,idleFrame.y);
            sprite->playAction(s_action);
        }
    }
}

void _player::impulseDamage(float amount, const Vec2f &damagePos) {
    if (isDead()) return; // Player dead skip
    health -= amount;
    bloodParticles->spawnEffect(damagePos,player_hit_effect);
}

void _player::setHealth(float amount) {
    health = amount;
}

// -- PRIVATE -- //

_player::PlayerAnimationResult _player::getAnimationResult(player_action action, player_face face) {
    if (action >= PLAYER_ACTION_COUNT || face >= PLAYER_FACE_COUNT || action == PLAYER_ACTION_NULL || face == PLAYER_FACE_NULL) {
        return animationTable[PLAYER_ACTION_NULL][PLAYER_FACE_NULL];
    }
    return animationTable[action][face];
}
