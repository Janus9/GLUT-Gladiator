#ifndef _ORC_H
#define _ORC_H

#include <_common.h>
#include <_enemyManager.h>
#include <_player.h>
#include <_sounds.h>

enum orc_action {
    ORC_ACTION_NULL,
    ORC_WALK_DOWN,   ORC_WALK_UP,   ORC_WALK_LEFT,   ORC_WALK_RIGHT,
    ORC_ATTACK_DOWN, ORC_ATTACK_UP, ORC_ATTACK_LEFT, ORC_ATTACK_RIGHT,
    ORC_HURT_DOWN,   ORC_HURT_UP,   ORC_HURT_LEFT,   ORC_HURT_RIGHT,
    ORC_DEATH_DOWN,  ORC_DEATH_UP,  ORC_DEATH_LEFT,  ORC_DEATH_RIGHT,
    // KEEP AT BACK //
    ORC_ACTION_COUNT
};

enum orc_face {
    ORC_FACE_NULL,
    ORC_FACE_DOWN,
    ORC_FACE_UP,
    ORC_FACE_LEFT,
    ORC_FACE_RIGHT,
    // KEEP AT BACK //
    ORC_FACE_COUNT
};

class _orc : public _enemy {
    public:
        _orc();
        virtual ~_orc();

        // Sets stats, sprites, animation lookup table.
        void initOrc();

        // Per-frame AI + animation FSM. Manager handles dead/despawn timing.
        void updateOrc(double dt, _player* player, _sounds* sounds);

        // Selects active sprite for current action and draws it.
        void drawOrc();

        // Called by _bulletManager (via _enemy::notifyDamaged) when this orc
        // takes a hit. Triggers HURT animation if alive.
        void notifyDamaged() override;

        // -- Stats -- //
        float moveSpeed      = 60.0f;   // units / second
        float attackRange    = 24.0f;   // distance to enter ATTACK
        float attackDamage   = 15.0f;   // dmg dealt per swing
        float attackCooldown = 1.0f;    // seconds between swings

        // -- FSM state -- //
        orc_face   face   = ORC_FACE_DOWN;
        orc_action action = ORC_ACTION_NULL;

        double cooldownTimer       = 0.0;   // counts up; ready when >= attackCooldown
        bool   inAttack            = false; // mid-swing
        bool   damageDealtThisSwing = false;
        bool   inHurt              = false;
        double hurtTimer           = 0.0;

    protected:
    private:
        struct OrcAnim {
            string sprite;       // "WALK" | "ATTACK" | "HURT" | "DEATH"
            string action;       // sprite_action name (e.g. "WALK_DOWN")
            Vec2i  idleFrame = {0, 0};
        };
        OrcAnim animationTable[ORC_ACTION_COUNT];

        // Apply animationTable entry for `a` and store as current `action`.
        void setAction(orc_action a);

        // Pick face from a vector pointing at the player (dominant axis).
        orc_face faceFromVec(const Vec2f& delta) const;

        orc_action walkActionFor(orc_face f)   const;
        orc_action attackActionFor(orc_face f) const;
        orc_action hurtActionFor(orc_face f)   const;
        orc_action deathActionFor(orc_face f)  const;

        // Returns the sprite name ("WALK"/"ATTACK"/...) the action belongs to.
        string spriteGroupFor(orc_action a) const;
};

#endif // _ORC_H
