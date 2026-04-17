#ifndef _PLAYER_H
#define _PLAYER_H

#include <_common.h>
#include <_unit.h>

enum player_face {
    PLAYER_FACE_NULL,
    PLAYER_FACE_N,
    PLAYER_FACE_NE,
    PLAYER_FACE_E,
    PLAYER_FACE_SE,
    PLAYER_FACE_S,
    PLAYER_FACE_SW,
    PLAYER_FACE_W,
    PLAYER_FACE_NW,
    PLAYER_FACE_COUNT // KEEP AT BACK
};

enum player_action {
    PLAYER_ACTION_NULL,
    PLAYER_ACTION_WALK,
    PLAYER_ACTION_WALK_GUN,
    PLAYER_ACTION_WALK_SHOOT,
    PLAYER_ACTION_COUNT // KEY AT BACK
};

class _player : public _unit {
    public:
        _player();
        virtual ~_player();

        // Sets up player sprites, animations, etc.
        void initPlayer();

        // Draw function for set action corresponding sprite/animation
        void drawPlayer();

        // Sets action for player (walk,run,shoot, etc)
        void setAction(player_action action, player_face face);

        // Stops current action
        void stopAction(player_action action);

        bool isShooting = false;
    protected:
    private:
    struct PlayerAnimationResult {
        string sprite = ""; // Which sprite
        string action = ""; // Which action (direction facing)
        Vec2i idleFrame = {0,0};
        
        bool operator==(const PlayerAnimationResult &other) const {
            return sprite == other.sprite && action == other.action;
        }
    };
    
    PlayerAnimationResult currentResult;

    PlayerAnimationResult animationTable[PLAYER_ACTION_COUNT][PLAYER_FACE_COUNT];
};

#endif // _PLAYER_H