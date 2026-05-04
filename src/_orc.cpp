#include <_orc.h>
#include <_world.h>

#include <cmath>

namespace {
    // Animation tuning constants — kept here so they're easy to tweak.
    constexpr int   ORC_WALK_FPS   = 10;
    constexpr int   ORC_ATTACK_FPS = 12;
    constexpr int   ORC_HURT_FPS   = 12;
    constexpr int   ORC_DEATH_FPS  = 8;

    constexpr int   ATTACK_FRAMES  = 8;
    constexpr int   ATTACK_HIT_FRAME = 4;       // 0-indexed frame the swing connects on
    constexpr int   HURT_FRAMES    = 3;
    constexpr int   DEATH_FRAMES   = 8;

    // Sheet rows (per user): 0=DOWN, 1=UP, 2=LEFT, 3=RIGHT.
    constexpr int   ROW_DOWN  = 0;
    constexpr int   ROW_UP    = 1;
    constexpr int   ROW_LEFT  = 2;
    constexpr int   ROW_RIGHT = 3;
}

_orc::_orc() {
    // ctor
}

_orc::~_orc() {
    // dtor
}

void _orc::initOrc(const _textureManager* sceneTextureManager) {
    if (!sceneTextureManager) {
        cout << "ERROR: Cannot initialize the orc enemy as the texture is missing\n";
        return;
    }

    // -- Stats / type tag -- //
    setMaxHealth(120.0f);
    resetHealth();
    eType = ENEMY_ORC;
    detectionRadius = 400.0f;
    team = _team::ENEMY;
    scale = {1.0f, 1.0f};
    setCollisionBox({16.0f, 16.0f});

    // -- WALK -- //
    setupSprite("WALK");
    if (_sprite* s = getSprite("WALK")) {
        const texture_entry &tex = sceneTextureManager->getTextureEntry("images/enemy/orc/orc_walk.png");
        s->initSprite(tex, 6, 4, 0, ORC_WALK_FPS);
        s->createSpriteAction(sprite_action("WALK_DOWN",  ROW_DOWN,  0, 5));
        s->createSpriteAction(sprite_action("WALK_UP",    ROW_UP,    0, 5));
        s->createSpriteAction(sprite_action("WALK_LEFT",  ROW_LEFT,  0, 5));
        s->createSpriteAction(sprite_action("WALK_RIGHT", ROW_RIGHT, 0, 5));
        Vec2f sz = s->getSize();
        s->pivotPoint = {sz.x / 2.0f, sz.y / 2.0f};
    }

    // -- ATTACK -- //
    setupSprite("ATTACK");
    if (_sprite* s = getSprite("ATTACK")) {
        const texture_entry &tex = sceneTextureManager->getTextureEntry("images/enemy/orc/orc_attack.png");
        s->initSprite(tex, 8, 4, 0, ORC_ATTACK_FPS);
        s->createSpriteAction(sprite_action("ATTACK_DOWN",  ROW_DOWN,  0, 7));
        s->createSpriteAction(sprite_action("ATTACK_UP",    ROW_UP,    0, 7));
        s->createSpriteAction(sprite_action("ATTACK_LEFT",  ROW_LEFT,  0, 7));
        s->createSpriteAction(sprite_action("ATTACK_RIGHT", ROW_RIGHT, 0, 7));
        Vec2f sz = s->getSize();
        s->pivotPoint = {sz.x / 2.0f, sz.y / 2.0f};
    }

    // -- HURT -- //
    setupSprite("HURT");
    if (_sprite* s = getSprite("HURT")) {
        const texture_entry &tex = sceneTextureManager->getTextureEntry("images/enemy/orc/orc_hurt.png");
        s->initSprite(tex, 6, 4, 0, ORC_HURT_FPS);
        s->createSpriteAction(sprite_action("HURT_DOWN",  ROW_DOWN,  0, 2));
        s->createSpriteAction(sprite_action("HURT_UP",    ROW_UP,    0, 2));
        s->createSpriteAction(sprite_action("HURT_LEFT",  ROW_LEFT,  0, 2));
        s->createSpriteAction(sprite_action("HURT_RIGHT", ROW_RIGHT, 0, 2));
        Vec2f sz = s->getSize();
        s->pivotPoint = {sz.x / 2.0f, sz.y / 2.0f};
    }

    // -- DEATH -- //
    setupSprite("DEATH");
    if (_sprite* s = getSprite("DEATH")) {
        const texture_entry &tex = sceneTextureManager->getTextureEntry("images/enemy/orc/orc_death.png");
        s->initSprite(tex, 8, 4, 0, ORC_DEATH_FPS);
        s->createSpriteAction(sprite_action("DEATH_DOWN",  ROW_DOWN,  0, 7));
        s->createSpriteAction(sprite_action("DEATH_UP",    ROW_UP,    0, 7));
        s->createSpriteAction(sprite_action("DEATH_LEFT",  ROW_LEFT,  0, 7));
        s->createSpriteAction(sprite_action("DEATH_RIGHT", ROW_RIGHT, 0, 7));
        Vec2f sz = s->getSize();
        s->pivotPoint = {sz.x / 2.0f, sz.y / 2.0f};
    }

    // Animation lookup table.
    animationTable[ORC_WALK_DOWN]    = {"WALK",   "WALK_DOWN",    {0, ROW_DOWN}};
    animationTable[ORC_WALK_UP]      = {"WALK",   "WALK_UP",      {0, ROW_UP}};
    animationTable[ORC_WALK_LEFT]    = {"WALK",   "WALK_LEFT",    {0, ROW_LEFT}};
    animationTable[ORC_WALK_RIGHT]   = {"WALK",   "WALK_RIGHT",   {0, ROW_RIGHT}};

    animationTable[ORC_ATTACK_DOWN]  = {"ATTACK", "ATTACK_DOWN",  {0, ROW_DOWN}};
    animationTable[ORC_ATTACK_UP]    = {"ATTACK", "ATTACK_UP",    {0, ROW_UP}};
    animationTable[ORC_ATTACK_LEFT]  = {"ATTACK", "ATTACK_LEFT",  {0, ROW_LEFT}};
    animationTable[ORC_ATTACK_RIGHT] = {"ATTACK", "ATTACK_RIGHT", {0, ROW_RIGHT}};

    animationTable[ORC_HURT_DOWN]    = {"HURT",   "HURT_DOWN",    {0, ROW_DOWN}};
    animationTable[ORC_HURT_UP]      = {"HURT",   "HURT_UP",      {0, ROW_UP}};
    animationTable[ORC_HURT_LEFT]    = {"HURT",   "HURT_LEFT",    {0, ROW_LEFT}};
    animationTable[ORC_HURT_RIGHT]   = {"HURT",   "HURT_RIGHT",   {0, ROW_RIGHT}};

    // Death idles on the last frame so corpse stays still.
    animationTable[ORC_DEATH_DOWN]   = {"DEATH",  "DEATH_DOWN",   {DEATH_FRAMES - 1, ROW_DOWN}};
    animationTable[ORC_DEATH_UP]     = {"DEATH",  "DEATH_UP",     {DEATH_FRAMES - 1, ROW_UP}};
    animationTable[ORC_DEATH_LEFT]   = {"DEATH",  "DEATH_LEFT",   {DEATH_FRAMES - 1, ROW_LEFT}};
    animationTable[ORC_DEATH_RIGHT]  = {"DEATH",  "DEATH_RIGHT",  {DEATH_FRAMES - 1, ROW_RIGHT}};

    // Boot into idle WALK_DOWN, animation paused.
    setAction(ORC_WALK_DOWN);
    if (_sprite* s = getSprite("WALK")) s->stopAnimation();
}

void _orc::updateOrc(double dt, _player* player, _world* world, _sounds* sounds) {
    if (!player) return;
    if (isDead()) return;     // Manager handles death/despawn timing.

    bool resolved = false; // mid-frame state finalized; skip remaining branches

    // -- HURT (interrupts everything but death) -- //
    if (inHurt) {
        vel = {0.0f, 0.0f};
        hurtTimer += dt;
        const double hurtDuration = double(HURT_FRAMES) / double(ORC_HURT_FPS);
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
        cooldownTimer += dt; // measured from start of swing in this state

        const double hitTime  = double(ATTACK_HIT_FRAME) / double(ORC_ATTACK_FPS);
        const double swingEnd = double(ATTACK_FRAMES)    / double(ORC_ATTACK_FPS);

        if (!damageDealtThisSwing && cooldownTimer >= hitTime) {
            // Hit lands if the player is still in melee contact at the hit
            // frame. Strict box-overlap fails when boxes are flush after
            // push-out, so we use a center-to-center distance threshold that
            // covers touching plus a small forgiveness margin.
            //   orc box 16x16  (half ~8) + player box 18x24 (half 9/12)
            //   touching distance ≈ 17 horizontal / 20 vertical → 28 pad
            constexpr float HIT_REACH = 28.0f;
            float dist = pos.distance(player->pos);
            if (dist <= HIT_REACH && !player->isDead()) {
                player->impulseDamage(attackDamage);
                player->playerTookDamage = true;
                if (sounds) sounds->playSfx("PLAYER_HURT");
            }
            damageDealtThisSwing = true;
        }

        if (cooldownTimer >= swingEnd) {
            inAttack = false;
            damageDealtThisSwing = false;
            cooldownTimer = 0.0; // restart cooldown clock
        }
        resolved = true;
    }

    // -- Normal AI: chase / idle. Attack decision is deferred to post-move. -- //
    if (!resolved) {
        Vec2f delta = player->pos - pos;
        float dist  = std::sqrt(delta.x * delta.x + delta.y * delta.y);

        if (dist > detectionRadius || player->isDead()) {
            // Idle — out of range or no target.
            vel = {0.0f, 0.0f};
            if (_sprite* s = getSprite(animationTable[action].sprite)) s->stopAnimation();
            cooldownTimer += dt;
        } else {
            face = faceFromVec(delta);
            // Always chase here; if we make contact this frame the post-move
            // step below will swap us into ATTACK.
            Vec2f dir = (dist > 0.0001f) ? Vec2f{delta.x / dist, delta.y / dist} : Vec2f{0.0f, 0.0f};
            vel = {dir.x * moveSpeed, dir.y * moveSpeed};
            setAction(walkActionFor(face));
            if (_sprite* s = getSprite("WALK")) {
                s->setFPS(ORC_WALK_FPS);
                s->startAnimation();
            }
            cooldownTimer += dt;
        }
    }

    // Per-axis world-tile probe so orcs are blocked by walls just like the
    // player. Probe at the leading edge of the collision box (plus the two
    // perpendicular corners) so the sprite stops at the wall instead of
    // sinking in until its center crosses the tile boundary.
    float stepX = vel.x * (float)dt;
    float stepY = vel.y * (float)dt;
    _collisionBound* box = getCollisionBound();
    if (world && box) {
        const Vec2f boxSize = box->getSize();
        const float halfW = boxSize.x * 0.5f;
        const float halfH = boxSize.y * 0.5f;
        const float inset = 0.5f; // keep corner probes off the perpendicular tile seam

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

    // If we ended up overlapping the player, this is a "contact" frame.
    // Push out so boxes only touch, then optionally promote chase->attack.
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
            // Promote to attack state — overrides the chase vel/animation we just set.
            vel = {0.0f, 0.0f};
            inAttack = true;
            damageDealtThisSwing = false;
            cooldownTimer = 0.0;
            setAction(attackActionFor(face));
            if (_sprite* s = getSprite("ATTACK")) {
                s->setFPS(ORC_ATTACK_FPS);
                s->startAnimation();
            }
            if (sounds) sounds->playSfx("ORC_ATTACK");
        }
    }
}

void _orc::drawOrc() {
    drawUnitSingular();
}

void _orc::triggerDeath(_sounds* sounds) {
    if (inDeathAnimation) return;
    inDeathAnimation = true;
    inAttack = false;
    inHurt = false;
    vel = {0.0f, 0.0f};
    setAction(deathActionFor(face));
    const orc_action deathAct = deathActionFor(face);
    if (_sprite* s = getSprite("DEATH")) {
        s->setFPS(ORC_DEATH_FPS);
        s->setIdleFrame(animationTable[deathAct].idleFrame.x, animationTable[deathAct].idleFrame.y);
        s->playAction(animationTable[deathAct].action);
    }
    if (sounds) sounds->playSfx("ORC_DEATH");
    deathTime = 0.0;
}

void _orc::notifyDamaged(_sounds* sounds) {
    if (isDead()) return;
    if (inHurt) return;
    inAttack = false;
    damageDealtThisSwing = false;
    inHurt = true;
    hurtTimer = 0.0;
    setAction(hurtActionFor(face));
    if (_sprite* s = getSprite("HURT")) {
        s->setFPS(ORC_HURT_FPS);
        s->startAnimation();
    }
    if (sounds) sounds->playSfx("ORC_HURT");
}

// -- PRIVATE -- //

void _orc::setAction(orc_action a) {
    if (a == ORC_ACTION_NULL || a >= ORC_ACTION_COUNT) return;
    if (a == action) return;
    const OrcAnim& anim = animationTable[a];
    _sprite* sprite = getSprite(anim.sprite);
    if (!sprite) return;

    sprite->loadSpriteAction(anim.action);
    sprite->setIdleFrame(anim.idleFrame.x, anim.idleFrame.y);
    setSingleSprite(sprite);
    action = a;
}

orc_face _orc::faceFromVec(const Vec2f& delta) const {
    if (std::fabs(delta.x) > std::fabs(delta.y)) {
        return (delta.x >= 0.0f) ? ORC_FACE_RIGHT : ORC_FACE_LEFT;
    }
    return (delta.y >= 0.0f) ? ORC_FACE_UP : ORC_FACE_DOWN;
}

orc_action _orc::walkActionFor(orc_face f) const {
    switch (f) {
        case ORC_FACE_DOWN:  return ORC_WALK_DOWN;
        case ORC_FACE_UP:    return ORC_WALK_UP;
        case ORC_FACE_LEFT:  return ORC_WALK_LEFT;
        case ORC_FACE_RIGHT: return ORC_WALK_RIGHT;
        default:             return ORC_WALK_DOWN;
    }
}

orc_action _orc::attackActionFor(orc_face f) const {
    switch (f) {
        case ORC_FACE_DOWN:  return ORC_ATTACK_DOWN;
        case ORC_FACE_UP:    return ORC_ATTACK_UP;
        case ORC_FACE_LEFT:  return ORC_ATTACK_LEFT;
        case ORC_FACE_RIGHT: return ORC_ATTACK_RIGHT;
        default:             return ORC_ATTACK_DOWN;
    }
}

orc_action _orc::hurtActionFor(orc_face f) const {
    switch (f) {
        case ORC_FACE_DOWN:  return ORC_HURT_DOWN;
        case ORC_FACE_UP:    return ORC_HURT_UP;
        case ORC_FACE_LEFT:  return ORC_HURT_LEFT;
        case ORC_FACE_RIGHT: return ORC_HURT_RIGHT;
        default:             return ORC_HURT_DOWN;
    }
}

orc_action _orc::deathActionFor(orc_face f) const {
    switch (f) {
        case ORC_FACE_DOWN:  return ORC_DEATH_DOWN;
        case ORC_FACE_UP:    return ORC_DEATH_UP;
        case ORC_FACE_LEFT:  return ORC_DEATH_LEFT;
        case ORC_FACE_RIGHT: return ORC_DEATH_RIGHT;
        default:             return ORC_DEATH_DOWN;
    }
}

string _orc::spriteGroupFor(orc_action a) const {
    if (a >= ORC_WALK_DOWN  && a <= ORC_WALK_RIGHT)   return "WALK";
    if (a >= ORC_ATTACK_DOWN && a <= ORC_ATTACK_RIGHT) return "ATTACK";
    if (a >= ORC_HURT_DOWN  && a <= ORC_HURT_RIGHT)   return "HURT";
    if (a >= ORC_DEATH_DOWN && a <= ORC_DEATH_RIGHT)  return "DEATH";
    return "";
}
