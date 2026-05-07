#include <_vampire.h>
#include <_world.h>

#include <cmath>

namespace {
    constexpr int VAMPIRE_IDLE_FPS   = 6;
    constexpr int VAMPIRE_WALK_FPS   = 10;
    constexpr int VAMPIRE_ATTACK_FPS = 14;
    constexpr int VAMPIRE_HURT_FPS   = 10;
    constexpr int VAMPIRE_DEATH_FPS  = 8;

    constexpr int IDLE_FRAMES   = 4;
    constexpr int WALK_FRAMES   = 6;
    constexpr int ATTACK_FRAMES = 12;
    constexpr int ATTACK_HIT_FRAME = 8;     // damage connects on this 0-indexed frame
    constexpr int HURT_FRAMES   = 4;
    constexpr int DEATH_FRAMES  = 11;

    // Sheet rows (per user): 0=DOWN, 1=UP, 2=LEFT, 3=RIGHT.
    constexpr int ROW_DOWN  = 0;
    constexpr int ROW_UP    = 1;
    constexpr int ROW_LEFT  = 2;
    constexpr int ROW_RIGHT = 3;
}

_vampire::_vampire() {}
_vampire::~_vampire() {}

void _vampire::initVampire(const _textureManager* sceneTextureManager, vampire_variant variant) {
    if (!sceneTextureManager) {
        cout << "ERROR: Cannot initialize the vampire enemy as the texture is missing\n";
        return;
    }

    // -- Per-variant config -- //
    string spritePrefix;
    float  vScale, vBox, vMaxHP, vDamage, vReach, vSpeed, vPitch;
    enemy_type vType;
    switch (variant) {
        case VAMPIRE_MINION1:
            spritePrefix = "images/enemy/vampire/minion/Minion1_";
            vScale = 0.35f; vBox = 6.0f; vMaxHP = 40.0f; vDamage = 6.0f;
            vReach = 10.0f; vSpeed = 55.0f; vPitch = 1.6f;
            vType = ENEMY_VAMPIRE_MINION1;
            break;
        case VAMPIRE_MINION2:
            spritePrefix = "images/enemy/vampire/minion/Minion2_";
            vScale = 0.35f; vBox = 6.0f; vMaxHP = 40.0f; vDamage = 6.0f;
            vReach = 10.0f; vSpeed = 55.0f; vPitch = 1.55f;
            vType = ENEMY_VAMPIRE_MINION2;
            break;
        case VAMPIRE_BOSS:
            isBoss = true;
        default:
            spritePrefix = "images/enemy/vampire/Vampire_";
            vScale = 1.5f; vBox = 24.0f; vMaxHP = 500.0f; vDamage = 30.0f;
            vReach = 32.0f; vSpeed = 40.0f; vPitch = 1.0f;
            vType = ENEMY_VAMPIRE;
            break;
    }

    // -- Stats / type tag -- //
    setMaxHealth(vMaxHP);
    resetHealth();
    eType = vType;
    detectionRadius = 300.0f;
    team = _team::ENEMY;
    scale = {vScale, vScale};
    setCollisionBox({vBox, vBox});
    timeInDeathAnimation = 8.0f;
    attackDamage = vDamage;
    hitReach = vReach;
    moveSpeed = vSpeed;
    soundPitch = vPitch;

    // -- IDLE -- //
    setupSprite("IDLE");
    if (_sprite* s = getSprite("IDLE")) {
        const texture_entry &tex = sceneTextureManager->getTextureEntry(spritePrefix + "Idle.png");
        s->initSprite(tex, IDLE_FRAMES, 4, 0, VAMPIRE_IDLE_FPS);
        s->createSpriteAction(sprite_action("IDLE_DOWN",  ROW_DOWN,  0, IDLE_FRAMES - 1));
        s->createSpriteAction(sprite_action("IDLE_UP",    ROW_UP,    0, IDLE_FRAMES - 1));
        s->createSpriteAction(sprite_action("IDLE_LEFT",  ROW_LEFT,  0, IDLE_FRAMES - 1));
        s->createSpriteAction(sprite_action("IDLE_RIGHT", ROW_RIGHT, 0, IDLE_FRAMES - 1));
        Vec2f sz = s->getSize();
        s->pivotPoint = {sz.x / 2.0f, sz.y / 2.0f};
    }

    // -- WALK -- //
    setupSprite("WALK");
    if (_sprite* s = getSprite("WALK")) {
        const texture_entry &tex = sceneTextureManager->getTextureEntry(spritePrefix + "Walk.png");
        s->initSprite(tex, WALK_FRAMES, 4, 0, VAMPIRE_WALK_FPS);
        s->createSpriteAction(sprite_action("WALK_DOWN",  ROW_DOWN,  0, WALK_FRAMES - 1));
        s->createSpriteAction(sprite_action("WALK_UP",    ROW_UP,    0, WALK_FRAMES - 1));
        s->createSpriteAction(sprite_action("WALK_LEFT",  ROW_LEFT,  0, WALK_FRAMES - 1));
        s->createSpriteAction(sprite_action("WALK_RIGHT", ROW_RIGHT, 0, WALK_FRAMES - 1));
        Vec2f sz = s->getSize();
        s->pivotPoint = {sz.x / 2.0f, sz.y / 2.0f};
    }

    // -- ATTACK -- //
    setupSprite("ATTACK");
    if (_sprite* s = getSprite("ATTACK")) {
        const texture_entry &tex = sceneTextureManager->getTextureEntry(spritePrefix + "Attack.png");
        s->initSprite(tex, ATTACK_FRAMES, 4, 0, VAMPIRE_ATTACK_FPS);
        s->createSpriteAction(sprite_action("ATTACK_DOWN",  ROW_DOWN,  0, ATTACK_FRAMES - 1));
        s->createSpriteAction(sprite_action("ATTACK_UP",    ROW_UP,    0, ATTACK_FRAMES - 1));
        s->createSpriteAction(sprite_action("ATTACK_LEFT",  ROW_LEFT,  0, ATTACK_FRAMES - 1));
        s->createSpriteAction(sprite_action("ATTACK_RIGHT", ROW_RIGHT, 0, ATTACK_FRAMES - 1));
        Vec2f sz = s->getSize();
        s->pivotPoint = {sz.x / 2.0f, sz.y / 2.0f};
    }

    // -- HURT -- //
    setupSprite("HURT");
    if (_sprite* s = getSprite("HURT")) {
        const texture_entry &tex = sceneTextureManager->getTextureEntry(spritePrefix + "Hurt.png");
        s->initSprite(tex, HURT_FRAMES, 4, 0, VAMPIRE_HURT_FPS);
        s->createSpriteAction(sprite_action("HURT_DOWN",  ROW_DOWN,  0, HURT_FRAMES - 1));
        s->createSpriteAction(sprite_action("HURT_UP",    ROW_UP,    0, HURT_FRAMES - 1));
        s->createSpriteAction(sprite_action("HURT_LEFT",  ROW_LEFT,  0, HURT_FRAMES - 1));
        s->createSpriteAction(sprite_action("HURT_RIGHT", ROW_RIGHT, 0, HURT_FRAMES - 1));
        Vec2f sz = s->getSize();
        s->pivotPoint = {sz.x / 2.0f, sz.y / 2.0f};
    }

    // -- DEATH -- //
    setupSprite("DEATH");
    if (_sprite* s = getSprite("DEATH")) {
        const texture_entry &tex = sceneTextureManager->getTextureEntry(spritePrefix + "Death.png");
        s->initSprite(tex, DEATH_FRAMES, 4, 0, VAMPIRE_DEATH_FPS);
        s->createSpriteAction(sprite_action("DEATH_DOWN",  ROW_DOWN,  0, DEATH_FRAMES - 1));
        s->createSpriteAction(sprite_action("DEATH_UP",    ROW_UP,    0, DEATH_FRAMES - 1));
        s->createSpriteAction(sprite_action("DEATH_LEFT",  ROW_LEFT,  0, DEATH_FRAMES - 1));
        s->createSpriteAction(sprite_action("DEATH_RIGHT", ROW_RIGHT, 0, DEATH_FRAMES - 1));
        Vec2f sz = s->getSize();
        s->pivotPoint = {sz.x / 2.0f, sz.y / 2.0f};
    }

    // _enemyManager renders sprites via a batched path that reads each sprite's
    // own scale (it never calls drawUnitSingular), so push the unit scale down once.
    for (_sprite* s : getSpriteList()) {
        if (s) s->scale = scale;
    }

    // Animation lookup table.
    animationTable[VAMPIRE_IDLE_DOWN]    = {"IDLE",   "IDLE_DOWN",    {0, ROW_DOWN}};
    animationTable[VAMPIRE_IDLE_UP]      = {"IDLE",   "IDLE_UP",      {0, ROW_UP}};
    animationTable[VAMPIRE_IDLE_LEFT]    = {"IDLE",   "IDLE_LEFT",    {0, ROW_LEFT}};
    animationTable[VAMPIRE_IDLE_RIGHT]   = {"IDLE",   "IDLE_RIGHT",   {0, ROW_RIGHT}};

    animationTable[VAMPIRE_WALK_DOWN]    = {"WALK",   "WALK_DOWN",    {0, ROW_DOWN}};
    animationTable[VAMPIRE_WALK_UP]      = {"WALK",   "WALK_UP",      {0, ROW_UP}};
    animationTable[VAMPIRE_WALK_LEFT]    = {"WALK",   "WALK_LEFT",    {0, ROW_LEFT}};
    animationTable[VAMPIRE_WALK_RIGHT]   = {"WALK",   "WALK_RIGHT",   {0, ROW_RIGHT}};

    animationTable[VAMPIRE_ATTACK_DOWN]  = {"ATTACK", "ATTACK_DOWN",  {0, ROW_DOWN}};
    animationTable[VAMPIRE_ATTACK_UP]    = {"ATTACK", "ATTACK_UP",    {0, ROW_UP}};
    animationTable[VAMPIRE_ATTACK_LEFT]  = {"ATTACK", "ATTACK_LEFT",  {0, ROW_LEFT}};
    animationTable[VAMPIRE_ATTACK_RIGHT] = {"ATTACK", "ATTACK_RIGHT", {0, ROW_RIGHT}};

    animationTable[VAMPIRE_HURT_DOWN]    = {"HURT",   "HURT_DOWN",    {0, ROW_DOWN}};
    animationTable[VAMPIRE_HURT_UP]      = {"HURT",   "HURT_UP",      {0, ROW_UP}};
    animationTable[VAMPIRE_HURT_LEFT]    = {"HURT",   "HURT_LEFT",    {0, ROW_LEFT}};
    animationTable[VAMPIRE_HURT_RIGHT]   = {"HURT",   "HURT_RIGHT",   {0, ROW_RIGHT}};

    // Death idles on the last frame so corpse stays still.
    animationTable[VAMPIRE_DEATH_DOWN]   = {"DEATH",  "DEATH_DOWN",   {DEATH_FRAMES - 1, ROW_DOWN}};
    animationTable[VAMPIRE_DEATH_UP]     = {"DEATH",  "DEATH_UP",     {DEATH_FRAMES - 1, ROW_UP}};
    animationTable[VAMPIRE_DEATH_LEFT]   = {"DEATH",  "DEATH_LEFT",   {DEATH_FRAMES - 1, ROW_LEFT}};
    animationTable[VAMPIRE_DEATH_RIGHT]  = {"DEATH",  "DEATH_RIGHT",  {DEATH_FRAMES - 1, ROW_RIGHT}};

    // Boot into IDLE_DOWN, animation running.
    setAction(VAMPIRE_IDLE_DOWN);
    if (_sprite* s = getSprite("IDLE")) s->startAnimation();
}

void _vampire::updateVampire(double dt, _player* player, _world* world, _sounds* sounds) {
    if (!player) return;
    if (isDead()) return;

    bool resolved = false;

    // -- HURT -- //
    if (inHurt) {
        vel = {0.0f, 0.0f};
        hurtTimer += dt;
        const double hurtDuration = double(HURT_FRAMES) / double(VAMPIRE_HURT_FPS);
        if (hurtTimer >= hurtDuration) {
            inHurt = false;
            hurtTimer = 0.0;
        } else {
            resolved = true;
        }
    }

    // -- ATTACK swing in progress -- //
    if (!resolved && inAttack) {
        vel = {0.0f, 0.0f};
        cooldownTimer += dt;

        const double hitTime  = double(ATTACK_HIT_FRAME) / double(VAMPIRE_ATTACK_FPS);
        const double swingEnd = double(ATTACK_FRAMES)    / double(VAMPIRE_ATTACK_FPS);

        if (!damageDealtThisSwing && cooldownTimer >= hitTime) {
            float dist = pos.distance(player->pos);
            if (dist <= hitReach && !player->isDead()) {
                player->impulseDamage(attackDamage);
                player->playerTookDamage = true;
                if (sounds) sounds->playSfx("PLAYER_HURT");
            }
            damageDealtThisSwing = true;
        }

        if (cooldownTimer >= swingEnd) {
            inAttack = false;
            damageDealtThisSwing = false;
            cooldownTimer = 0.0;
        }
        resolved = true;
    }

    // -- Normal AI: idle / chase -- //
    if (!resolved) {
        Vec2f delta = player->pos - pos;
        float dist  = std::sqrt(delta.x * delta.x + delta.y * delta.y);

        if (dist > detectionRadius || player->isDead()) {
            vel = {0.0f, 0.0f};
            setAction(idleActionFor(face));
            if (_sprite* s = getSprite("IDLE")) {
                s->setFPS(VAMPIRE_IDLE_FPS);
                s->startAnimation();
            }
            cooldownTimer += dt;
        } else {
            face = faceFromVec(delta);
            Vec2f dir = (dist > 0.0001f) ? Vec2f{delta.x / dist, delta.y / dist} : Vec2f{0.0f, 0.0f};
            vel = {dir.x * moveSpeed, dir.y * moveSpeed};
            setAction(walkActionFor(face));
            if (_sprite* s = getSprite("WALK")) {
                s->setFPS(VAMPIRE_WALK_FPS);
                s->startAnimation();
            }
            cooldownTimer += dt;
        }
    }

    // Per-axis world-tile probe (mirrors _orc collision logic).
    float stepX = vel.x * (float)dt;
    float stepY = vel.y * (float)dt;
    _collisionBound* box = getCollisionBound();
    if (world && box) {
        const Vec2f boxSize = box->getSize();
        const float halfW = boxSize.x * 0.5f;
        const float halfH = boxSize.y * 0.5f;
        const float inset = 0.5f;

        auto blockedX = [&](float dx) {
            float edgeX = (dx > 0.0f ? halfW : -halfW) + dx;
            const _tile* a = world->getTileAtWorld(pos + Vec2f(edgeX,  halfH - inset));
            const _tile* b = world->getTileAtWorld(pos + Vec2f(edgeX, -halfH + inset));
            return (a && a->hasCollision) || (b && b->hasCollision);
        };
        auto blockedY = [&](float dy) {
            float edgeY = (dy > 0.0f ? halfH : -halfH) + dy;
            const _tile* a = world->getTileAtWorld(pos + Vec2f( halfW - inset, edgeY));
            const _tile* b = world->getTileAtWorld(pos + Vec2f(-halfW + inset, edgeY));
            return (a && a->hasCollision) || (b && b->hasCollision);
        };

        if (stepX != 0.0f && !blockedX(stepX)) pos.x += stepX;
        if (stepY != 0.0f && !blockedY(stepY)) pos.y += stepY;
    } else {
        pos.x += stepX;
        pos.y += stepY;
    }

    // Contact check: push out and optionally promote to ATTACK.
    bool madeContact = isColliding(*player);
    if (madeContact) {
        Vec2f p = getPenetration(*player);
        if (std::fabs(p.x) < std::fabs(p.y)) {
            pos.x -= p.x;
        } else {
            pos.y -= p.y;
        }

        if (!resolved && !inAttack && !inHurt && !player->isDead()
            && cooldownTimer >= attackCooldown) {
            vel = {0.0f, 0.0f};
            inAttack = true;
            damageDealtThisSwing = false;
            cooldownTimer = 0.0;
            setAction(attackActionFor(face));
            if (_sprite* s = getSprite("ATTACK")) {
                s->setFPS(VAMPIRE_ATTACK_FPS);
                s->startAnimation();
            }
            if (sounds) sounds->playSfx("VAMPIRE_ATTACK", soundPitch);
        }
    }
}

void _vampire::drawVampire() {
    drawUnitSingular();
}

void _vampire::triggerDeath(_sounds* sounds) {
    if (inDeathAnimation) return;
    inDeathAnimation = true;
    inAttack = false;
    inHurt = false;
    vel = {0.0f, 0.0f};
    setAction(deathActionFor(face));
    const vampire_action deathAct = deathActionFor(face);
    if (_sprite* s = getSprite("DEATH")) {
        s->setFPS(VAMPIRE_DEATH_FPS);
        s->setIdleFrame(animationTable[deathAct].idleFrame.x, animationTable[deathAct].idleFrame.y);
        s->playAction(animationTable[deathAct].action);
    }
    if (sounds) sounds->playSfx("VAMPIRE_DEATH", soundPitch);
    deathTime = 0.0;
}

void _vampire::notifyDamaged(_sounds* sounds) {
    if (isDead()) return;
    if (inHurt) return;
    inAttack = false;
    damageDealtThisSwing = false;
    inHurt = true;
    hurtTimer = 0.0;
    setAction(hurtActionFor(face));
    if (_sprite* s = getSprite("HURT")) {
        s->setFPS(VAMPIRE_HURT_FPS);
        s->startAnimation();
    }
    if (sounds) sounds->playSfx("VAMPIRE_HURT", soundPitch);
}

// -- PRIVATE -- //

void _vampire::setAction(vampire_action a) {
    if (a == VAMPIRE_ACTION_NULL || a >= VAMPIRE_ACTION_COUNT) return;
    if (a == action) return;
    const VampireAnim& anim = animationTable[a];
    _sprite* sprite = getSprite(anim.sprite);
    if (!sprite) return;

    sprite->loadSpriteAction(anim.action);
    sprite->setIdleFrame(anim.idleFrame.x, anim.idleFrame.y);
    setSingleSprite(sprite);
    action = a;
}

vampire_face _vampire::faceFromVec(const Vec2f& delta) const {
    if (std::fabs(delta.x) > std::fabs(delta.y)) {
        return (delta.x >= 0.0f) ? VAMPIRE_FACE_RIGHT : VAMPIRE_FACE_LEFT;
    }
    return (delta.y >= 0.0f) ? VAMPIRE_FACE_UP : VAMPIRE_FACE_DOWN;
}

vampire_action _vampire::idleActionFor(vampire_face f) const {
    switch (f) {
        case VAMPIRE_FACE_DOWN:  return VAMPIRE_IDLE_DOWN;
        case VAMPIRE_FACE_UP:    return VAMPIRE_IDLE_UP;
        case VAMPIRE_FACE_LEFT:  return VAMPIRE_IDLE_LEFT;
        case VAMPIRE_FACE_RIGHT: return VAMPIRE_IDLE_RIGHT;
        default:                 return VAMPIRE_IDLE_DOWN;
    }
}

vampire_action _vampire::walkActionFor(vampire_face f) const {
    switch (f) {
        case VAMPIRE_FACE_DOWN:  return VAMPIRE_WALK_DOWN;
        case VAMPIRE_FACE_UP:    return VAMPIRE_WALK_UP;
        case VAMPIRE_FACE_LEFT:  return VAMPIRE_WALK_LEFT;
        case VAMPIRE_FACE_RIGHT: return VAMPIRE_WALK_RIGHT;
        default:                 return VAMPIRE_WALK_DOWN;
    }
}

vampire_action _vampire::attackActionFor(vampire_face f) const {
    switch (f) {
        case VAMPIRE_FACE_DOWN:  return VAMPIRE_ATTACK_DOWN;
        case VAMPIRE_FACE_UP:    return VAMPIRE_ATTACK_UP;
        case VAMPIRE_FACE_LEFT:  return VAMPIRE_ATTACK_LEFT;
        case VAMPIRE_FACE_RIGHT: return VAMPIRE_ATTACK_RIGHT;
        default:                 return VAMPIRE_ATTACK_DOWN;
    }
}

vampire_action _vampire::hurtActionFor(vampire_face f) const {
    switch (f) {
        case VAMPIRE_FACE_DOWN:  return VAMPIRE_HURT_DOWN;
        case VAMPIRE_FACE_UP:    return VAMPIRE_HURT_UP;
        case VAMPIRE_FACE_LEFT:  return VAMPIRE_HURT_LEFT;
        case VAMPIRE_FACE_RIGHT: return VAMPIRE_HURT_RIGHT;
        default:                 return VAMPIRE_HURT_DOWN;
    }
}

vampire_action _vampire::deathActionFor(vampire_face f) const {
    switch (f) {
        case VAMPIRE_FACE_DOWN:  return VAMPIRE_DEATH_DOWN;
        case VAMPIRE_FACE_UP:    return VAMPIRE_DEATH_UP;
        case VAMPIRE_FACE_LEFT:  return VAMPIRE_DEATH_LEFT;
        case VAMPIRE_FACE_RIGHT: return VAMPIRE_DEATH_RIGHT;
        default:                 return VAMPIRE_DEATH_DOWN;
    }
}

string _vampire::spriteGroupFor(vampire_action a) const {
    if (a >= VAMPIRE_IDLE_DOWN   && a <= VAMPIRE_IDLE_RIGHT)   return "IDLE";
    if (a >= VAMPIRE_WALK_DOWN   && a <= VAMPIRE_WALK_RIGHT)   return "WALK";
    if (a >= VAMPIRE_ATTACK_DOWN && a <= VAMPIRE_ATTACK_RIGHT) return "ATTACK";
    if (a >= VAMPIRE_HURT_DOWN   && a <= VAMPIRE_HURT_RIGHT)   return "HURT";
    if (a >= VAMPIRE_DEATH_DOWN  && a <= VAMPIRE_DEATH_RIGHT)  return "DEATH";
    return "";
}
