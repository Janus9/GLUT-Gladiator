#ifndef _VAMPIRE_H
#define _VAMPIRE_H

#include <_common.h>
#include <_enemyManager.h>
#include <_player.h>
#include <_sounds.h>
#include <_textureManager.h>

class _world;

enum vampire_variant : uint8_t {
    VAMPIRE_BOSS,
    VAMPIRE_MINION1,
    VAMPIRE_MINION2,
};

enum vampire_action {
    VAMPIRE_ACTION_NULL,
    VAMPIRE_IDLE_DOWN,   VAMPIRE_IDLE_UP,   VAMPIRE_IDLE_LEFT,   VAMPIRE_IDLE_RIGHT,
    VAMPIRE_WALK_DOWN,   VAMPIRE_WALK_UP,   VAMPIRE_WALK_LEFT,   VAMPIRE_WALK_RIGHT,
    VAMPIRE_ATTACK_DOWN, VAMPIRE_ATTACK_UP, VAMPIRE_ATTACK_LEFT, VAMPIRE_ATTACK_RIGHT,
    VAMPIRE_HURT_DOWN,   VAMPIRE_HURT_UP,   VAMPIRE_HURT_LEFT,   VAMPIRE_HURT_RIGHT,
    VAMPIRE_DEATH_DOWN,  VAMPIRE_DEATH_UP,  VAMPIRE_DEATH_LEFT,  VAMPIRE_DEATH_RIGHT,
    // KEEP AT BACK //
    VAMPIRE_ACTION_COUNT
};

enum vampire_face {
    VAMPIRE_FACE_NULL,
    VAMPIRE_FACE_DOWN,
    VAMPIRE_FACE_UP,
    VAMPIRE_FACE_LEFT,
    VAMPIRE_FACE_RIGHT,
    // KEEP AT BACK //
    VAMPIRE_FACE_COUNT
};

class _vampire : public _enemy {
    public:
        _vampire();
        virtual ~_vampire();

        void initVampire(const _textureManager* sceneTextureManager, vampire_variant variant = VAMPIRE_BOSS);
        void updateVampire(double dt, _player* player, _world* world, _sounds* sounds);
        void drawVampire();

        void notifyDamaged(_sounds* sounds) override;
        void triggerDeath(_sounds* sounds);

        // -- Stats (defaults are boss-tier; initVampire overrides per variant) -- //
        float moveSpeed      = 40.0f;
        float attackRange    = 28.0f;
        float attackDamage   = 30.0f;
        float attackCooldown = 1.5f;
        float soundPitch     = 1.0f;

        // -- FSM state -- //
        vampire_face   face   = VAMPIRE_FACE_DOWN;
        vampire_action action = VAMPIRE_ACTION_NULL;

        double cooldownTimer        = 0.0;
        bool   inAttack             = false;
        bool   damageDealtThisSwing = false;
        bool   inHurt               = false;
        double hurtTimer            = 0.0;

    private:
        struct VampireAnim {
            string sprite;     // "IDLE" | "WALK" | "ATTACK" | "HURT" | "DEATH"
            string action;     // sprite_action name (e.g. "WALK_DOWN")
            Vec2i  idleFrame = {0, 0};
        };
        VampireAnim animationTable[VAMPIRE_ACTION_COUNT];

        void setAction(vampire_action a);

        vampire_face   faceFromVec(const Vec2f& delta) const;
        vampire_action idleActionFor(vampire_face f)   const;
        vampire_action walkActionFor(vampire_face f)   const;
        vampire_action attackActionFor(vampire_face f) const;
        vampire_action hurtActionFor(vampire_face f)   const;
        vampire_action deathActionFor(vampire_face f)  const;

        string spriteGroupFor(vampire_action a) const;
};

#endif // _VAMPIRE_H
