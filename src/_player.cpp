#include <_player.h>

_player::_player() {
    // ctor
}

_player::~_player() {
    // dtor
}

void _player::initPlayer(_lightManager* lightManager) {
    sceneLightManager = lightManager;
    scale = {0.8f, 0.8f};

    // AABB used by enemies (e.g. _orc) to prevent overlap and decide melee contact.
    setCollisionBox({18.0f, 24.0f});

    particleManger->initParticleManager("images/player/particles.png",3, sceneLightManager, 1000);

    // Player Hit Blood Effect //
    player_hit_effect.amount = 35;
    player_hit_effect.imageIndex = 2;

    player_hit_effect.minVelX = -8.0f;
    player_hit_effect.maxVelX = 8.0f;
    player_hit_effect.minVelY = 10.0f;
    player_hit_effect.maxVelY = 25.0f;

    player_hit_effect.minRadius = 2.0f;
    player_hit_effect.maxRadius = 3.5f;

    player_hit_effect.minLifeTime = 5.0f;
    player_hit_effect.maxLifeTime = 5.0f;

    player_hit_effect.minSpawnOffsetX = -3.0f;
    player_hit_effect.maxSpawnOffsetX = 3.0f;
    player_hit_effect.minSpawnOffsetY = -3.0f;
    player_hit_effect.maxSpawnOffsetY = 3.0f;

    player_hit_effect.minRotation = 30.0f;
    player_hit_effect.minRotation = 180.0f;

    player_hit_effect.hasFloor = true;
    player_hit_effect.floorOffset = -4.0f;

    // Resupply Health Effect //
    resupply_health_effect.amount = 35;
    resupply_health_effect.imageIndex = 0;

    resupply_health_effect.minVelX = -3.0f;
    resupply_health_effect.maxVelX = 3.0f;
    resupply_health_effect.minVelY = 3.0f;
    resupply_health_effect.maxVelY = 5.0f;

    resupply_health_effect.minRadius = 2.0f;
    resupply_health_effect.maxRadius = 4.0f;

    resupply_health_effect.minLifeTime = 2.0f;
    resupply_health_effect.maxRadius = 3.0f;

    resupply_health_effect.minSpawnOffsetX = -12.0f;
    resupply_health_effect.maxSpawnOffsetX = 12.0f;
    resupply_health_effect.minSpawnOffsetY = -6.0f;
    resupply_health_effect.maxSpawnOffsetY = 8.0f;

    resupply_health_effect.hasGravity = false;

    resupply_health_effect.waveAmplitudeMin = 1.0f;
    resupply_health_effect.waveAmplitudeMax = 3.3f;
    resupply_health_effect.waveFrequencyMin = 0.2f;
    resupply_health_effect.waveFrequencyMax = 1.0f;

    // Resupply Ammo Effect //
    resupply_ammo_effect.amount = 35;
    resupply_ammo_effect.imageIndex = 1;

    resupply_ammo_effect.minVelX = -3.0f;
    resupply_ammo_effect.maxVelX = 3.0f;
    resupply_ammo_effect.minVelY = 3.0f;
    resupply_ammo_effect.maxVelY = 5.0f;

    resupply_ammo_effect.minRadius = 2.0f;
    resupply_ammo_effect.maxRadius = 4.0f;

    resupply_ammo_effect.minLifeTime = 2.0f;
    resupply_ammo_effect.maxRadius = 3.0f;

    resupply_ammo_effect.minSpawnOffsetX = -12.0f;
    resupply_ammo_effect.maxSpawnOffsetX = 12.0f;
    resupply_ammo_effect.minSpawnOffsetY = -6.0f;
    resupply_ammo_effect.maxSpawnOffsetY = 8.0f;

    resupply_ammo_effect.hasGravity = false;

    resupply_ammo_effect.waveAmplitudeMin = 1.0f;
    resupply_ammo_effect.waveAmplitudeMax = 3.3f;
    resupply_ammo_effect.waveFrequencyMin = 0.2f;
    resupply_ammo_effect.waveFrequencyMax = 1.0f;

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

    // Idle Reload Animation //
    setupSprite("IDLE_RELOAD");
    _sprite* reload_gun_sprite = getSprite("IDLE_RELOAD");
    if (reload_gun_sprite) {
        reload_gun_sprite->initSprite("images/player/Reloading/Reloading.png", 8, 6, sprite_direction::LEFT,12); // No natural direction due to top down
    
        reload_gun_sprite->createSpriteAction(sprite_action("IDLE_RELOAD_N",3,0,7));
        reload_gun_sprite->createSpriteAction(sprite_action("IDLE_RELOAD_NE",4,0,7));
        reload_gun_sprite->createSpriteAction(sprite_action("IDLE_RELOAD_E",5,0,7));
        reload_gun_sprite->createSpriteAction(sprite_action("IDLE_RELOAD_SE",5,0,7));
        reload_gun_sprite->createSpriteAction(sprite_action("IDLE_RELOAD_S",0,0,7));
        reload_gun_sprite->createSpriteAction(sprite_action("IDLE_RELOAD_SW",1,0,7));
        reload_gun_sprite->createSpriteAction(sprite_action("IDLE_RELOAD_W",1,0,7));
        reload_gun_sprite->createSpriteAction(sprite_action("IDLE_RELOAD_NW",2,0,7));
        reload_gun_sprite->offsetPoint = {0.0f, 8.0f};
    }

    animationTable[PLAYER_ACTION_IDLE_RELOAD][PLAYER_FACE_N] = {"IDLE_RELOAD","IDLE_RELOAD_N",{7,3}};
    animationTable[PLAYER_ACTION_IDLE_RELOAD][PLAYER_FACE_NE] = {"IDLE_RELOAD","IDLE_RELOAD_NE",{7,4}};
    animationTable[PLAYER_ACTION_IDLE_RELOAD][PLAYER_FACE_E] = {"IDLE_RELOAD","IDLE_RELOAD_E",{7,5}};
    animationTable[PLAYER_ACTION_IDLE_RELOAD][PLAYER_FACE_SE] = {"IDLE_RELOAD","IDLE_RELOAD_SE",{7,5}};
    animationTable[PLAYER_ACTION_IDLE_RELOAD][PLAYER_FACE_S] = {"IDLE_RELOAD","IDLE_RELOAD_S",{7,0}};
    animationTable[PLAYER_ACTION_IDLE_RELOAD][PLAYER_FACE_SW] = {"IDLE_RELOAD","IDLE_RELOAD_SW",{7,1}};
    animationTable[PLAYER_ACTION_IDLE_RELOAD][PLAYER_FACE_W] = {"IDLE_RELOAD","IDLE_RELOAD_W",{7,1}};
    animationTable[PLAYER_ACTION_IDLE_RELOAD][PLAYER_FACE_NW] = {"IDLE_RELOAD","IDLE_RELOAD_NW",{7,2}};
    // Walk Reload Animation //
    setupSprite("WALK_RELOAD");
    _sprite* walk_reload_sprite = getSprite("WALK_RELOAD");
    if (walk_reload_sprite) {
        walk_reload_sprite->initSprite("images/player/Walk_while_Reloading - NEW/Walk_while_reloading.png", 8, 8, sprite_direction::LEFT,12); // No natural direction due to top down
    
        walk_reload_sprite->createSpriteAction(sprite_action("WALK_RELOAD_N",3,0,7));
        walk_reload_sprite->createSpriteAction(sprite_action("WALK_RELOAD_NE",4,0,7));
        walk_reload_sprite->createSpriteAction(sprite_action("WALK_RELOAD_E",6,0,7));
        walk_reload_sprite->createSpriteAction(sprite_action("WALK_RELOAD_SE",5,0,7));
        walk_reload_sprite->createSpriteAction(sprite_action("WALK_RELOAD_S",0,0,7));
        walk_reload_sprite->createSpriteAction(sprite_action("WALK_RELOAD_SW",1,0,7));
        walk_reload_sprite->createSpriteAction(sprite_action("WALK_RELOAD_W",7,0,7));
        walk_reload_sprite->createSpriteAction(sprite_action("WALK_RELOAD_NW",2,0,7));
        walk_reload_sprite->offsetPoint = {0.0f, 8.0f};
    }

    animationTable[PLAYER_ACTION_WALK_RELOAD][PLAYER_FACE_N] = {"WALK_RELOAD","WALK_RELOAD_N",{0,3}};
    animationTable[PLAYER_ACTION_WALK_RELOAD][PLAYER_FACE_NE] = {"WALK_RELOAD","WALK_RELOAD_NE",{0,4}};
    animationTable[PLAYER_ACTION_WALK_RELOAD][PLAYER_FACE_E] = {"WALK_RELOAD","WALK_RELOAD_E",{0,6}};
    animationTable[PLAYER_ACTION_WALK_RELOAD][PLAYER_FACE_SE] = {"WALK_RELOAD","WALK_RELOAD_SE",{0,5}};
    animationTable[PLAYER_ACTION_WALK_RELOAD][PLAYER_FACE_S] = {"WALK_RELOAD","WALK_RELOAD_S",{0,0}};
    animationTable[PLAYER_ACTION_WALK_RELOAD][PLAYER_FACE_SW] = {"WALK_RELOAD","WALK_RELOAD_SW",{0,1}};
    animationTable[PLAYER_ACTION_WALK_RELOAD][PLAYER_FACE_W] = {"WALK_RELOAD","WALK_RELOAD_W",{0,7}};
    animationTable[PLAYER_ACTION_WALK_RELOAD][PLAYER_FACE_NW] = {"WALK_RELOAD","WALK_RELOAD_NW",{0,2}};

}

void _player::updatePlayer(double dt) {

    if (isRealDead) return; // Player is really dead -- game over

    // Damage Event //
    if (playerTookDamage) {
        particleManger->spawnEffect(pos,player_hit_effect);
        playerTookDamage = false;
    }
    particleManger->updateParticleManger(dt);
    
    // Death Event //
    if (inDeathAnimation) {
        deathTimeElapsed += dt;
    }

    // Reload Event //
    if (reloading) {
        reloadTimeElapsed += dt;
        if (reloadTimeElapsed >= reloadSpeed) {
            // reload finished //
            cout << "Player reload finished\n";
            int numBulletsFromReserve = 0;  // How many bullets to put INTO the mag from reserve
            int numBulletsIntoMag = magCapacity - magLevel;  // How many bullets to put INTO the mag
            
            int numBulletRemaining = reserveLevel - numBulletsIntoMag; // How many bullets remain in reserve after reload
            if (numBulletRemaining < 0) {
                // Negative means reserve is less than mag capacity, we add the remainder
                reserveLevel = 0;
                numBulletsFromReserve = magCapacity + numBulletRemaining; // num bullet remaining is negative so it takes away bullets
            } else {
                // Excess bullets in reserve
                reserveLevel-=numBulletsIntoMag;
                numBulletsFromReserve = magCapacity;
            }
            magLevel = numBulletsFromReserve;

            reloadTimeElapsed = 0.0;
            reloading = false;
        }
    }

    // Shoot Event //
    if (isShooting) {
        if (magLevel <= 0) {
            // Empty mag
            isShooting = false;
        } else {
            // Has ammo
            shootTimeElapsed += dt;
            if (shootTimeElapsed > 1 / (fireRate / 60)) {
                // Bullet fired
                playerShootEvent = true;
                shootTimeElapsed = 0.0;
                magLevel--;
            }
        }
    }
    
    // Level Change Event //
    const float prevDistance = previousPos.distance({0.0f,0.0f});
    const float distance = pos.distance({0.0f,0.0f});

    if (prevDistance > 8000.0f && distance < 8000.0f) {
        // Entered the middle from outer
        playerLevelEvent = PLAYER_EVENT_LEVEL_MIDDLE;
    } else if (prevDistance < 8000.0f && distance > 8000.0f) {
        // Entered the outer from middle
        playerLevelEvent = PLAYER_EVENT_LEVEL_OUTER;
    } else if (prevDistance > 3000.0f && distance < 3000.0f) {
        // Entered the center from middle
        playerLevelEvent = PLAYER_EVENT_LEVEL_MIDDLE;
    } else if (prevDistance < 3000.0f && distance > 3000.0f) {
        // Entered the middle from center
        playerLevelEvent = PLAYER_EVENT_LEVEL_CENTER;
    } else if (prevDistance > 400.0f && distance < 400.0f) {
        // Entered the boss from center
        playerLevelEvent = PLAYER_EVENT_LEVEL_BOSS;
    } else if (prevDistance < 400.0f && distance > 400.0f) {
        // Entered the center from boss
        playerLevelEvent = PLAYER_EVENT_LEVEL_CENTER;
    }

    previousPos = pos;
}

void _player::drawPlayer() {
    if (isRealDead) return; // Player dead skip drawing
    drawUnitSingular();
    particleManger->drawParticleManager();
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
    if (deathTimeElapsed > respawnTime) {
        // Handles respawn
        inDeathAnimation = false;
        deathTimeElapsed = 0.0;
        resetHealth();
        pos = spawnPos;
        lives--;
        if (lives <= -1) {
            isRealDead = true;
        }
        setAction(PLAYER_ACTION_IDLE_GUN,PLAYER_FACE_N);
        playerRespawnedEvent = true;
        return;
    }
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

player_serial_data _player::exportSerializedPlayer() const {
    player_serial_data player_data;

    player_data.team = static_cast<uint8_t>(team);
    player_data.maxHP = getMaxHealth();
    player_data.currentHP = getHealth();
    player_data.posX = pos.x;
    player_data.posY = pos.y;
    player_data.fireRate = fireRate;
    player_data.respawnTime = respawnTime;
    player_data.spawnPosX = spawnPos.x;
    player_data.spawnPosY = spawnPos.y;
    player_data.movementSpeed = movementSpeed;
    player_data.numDeaths = abs(lives - 3);
    player_data.magCapacity = magCapacity;
    player_data.magLevel = magLevel;
    player_data.reserveCapacity = reserveCapacity;
    player_data.reserveLevel = reserveLevel;
    player_data.reloadSpeed = reloadSpeed;
    // Padding -- doesnt do anything
    player_data.padding1 = 0;
    player_data.padding2 = 0;

    return player_data;
}

void _player::importSerializedPlayer(const player_serial_data &player_data) {
    team = static_cast<_team>(player_data.team);
    setMaxHealth(player_data.maxHP);
    setHealth(player_data.currentHP);
    pos.x = player_data.posX;
    pos.y = player_data.posY;
    fireRate = player_data.fireRate;
    respawnTime = player_data.respawnTime;
    spawnPos.x = player_data.spawnPosX;
    spawnPos.y = player_data.spawnPosY;
    movementSpeed = player_data.movementSpeed;
    lives = 3 - player_data.numDeaths;
    magCapacity = player_data.magCapacity;
    magLevel = player_data.magLevel;
    reserveCapacity = player_data.reserveCapacity;
    reserveLevel = player_data.reserveLevel;
    reloadSpeed = player_data.reloadSpeed;
}

void _player::resupply(float health, int ammo) {
    if (getHealthPct() == 1.0f && reserveLevel == reserveCapacity) {
        // Player already resupplied, skip event
        return;
    }
    const float hpDiff = getMaxHealth() - getHealth();
    const int ammoDiff = reserveCapacity - reserveLevel;

    int numHealthParticles = 0;
    if (hpDiff > health) {
        numHealthParticles = static_cast<int>(health);
    } else {
        numHealthParticles = static_cast<int>(hpDiff);
    }

    int numAmmoParticles = 0;
    if (ammoDiff > ammo) {
        numAmmoParticles = ammo;
    } else {
        numAmmoParticles = ammoDiff;
    }

    impulseHealing(health);
    reserveLevel += ammo;
    if (reserveLevel > reserveCapacity) {
        reserveLevel = reserveCapacity;
    }
    resupply_health_effect.amount = numHealthParticles;
    particleManger->spawnEffect(pos,resupply_health_effect);
    resupply_ammo_effect.amount = numAmmoParticles;
    particleManger->spawnEffect(pos,resupply_ammo_effect);
}

void _player::procReload() {
    if (reloading) return; // Early return on reloading for call saftey
    if (reserveLevel <= 0 || magLevel == magCapacity) return; // Early return, no bullets to reload or mag is full
    cout << "Reload event started\n";
    reloading = true;
}

bool _player::isReloading() const {
    return reloading;
}

// -- PRIVATE -- //

_player::PlayerAnimationResult _player::getAnimationResult(player_action action, player_face face) {
    if (action >= PLAYER_ACTION_COUNT || face >= PLAYER_FACE_COUNT || action == PLAYER_ACTION_NULL || face == PLAYER_FACE_NULL) {
        return animationTable[PLAYER_ACTION_NULL][PLAYER_FACE_NULL];
    }
    return animationTable[action][face];
}
