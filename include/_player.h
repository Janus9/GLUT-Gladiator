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
    // KEEP AT BACK //
    PLAYER_FACE_COUNT 
};

enum player_action {
    PLAYER_ACTION_NULL,
    PLAYER_ACTION_WALK,
    PLAYER_ACTION_WALK_GUN,
    PLAYER_ACTION_WALK_SHOOT,
    PLAYER_ACTION_IDLE,
    PLAYER_ACTION_IDLE_GUN,
    PLAYER_ACTION_IDLE_SHOOT,
    // KEEP AT BACK //
    PLAYER_ACTION_COUNT 
};

class _player : public _unit {
    public:
        _player();
        virtual ~_player();

        /**
         * Sets up player sprites, animations, etc.
         */
        void initPlayer();

        /**
         * Update loop for player including inputs/animations/movement/etc
         * 
         * @param dt Delta time (in seconds)
         */
        void updatePlayer(double dt);

        // Draw function for set action corresponding sprite/animation
        void drawPlayer();

        // Sets action for player (walk,run,shoot, etc)
        void setAction(player_action action, player_face face);

        // Stops current action
        void stopAction(player_action action);
        
        bool isMoving = false;      // Is player actively moving
        bool hasGun = false;        // Does player have gun equipped
        bool isShooting = false;    // Is the player actively shooting
    protected:
    private:
        struct PlayerAnimationResult {
            string sprite = ""; // Which sprite
            string action = ""; // Which action (direction facing)
            Vec2i idleFrame = {0,0};
            bool valid = true;  // Wether action is valid (not null)
            
            bool operator==(const PlayerAnimationResult &other) const {
                return sprite == other.sprite && action == other.action;
            }
        };
        
        PlayerAnimationResult animationTable[PLAYER_ACTION_COUNT][PLAYER_FACE_COUNT];
        
        PlayerAnimationResult currentResult;

        // Gets animation with error checking + NULL handling
        PlayerAnimationResult getAnimationResult(player_action action, player_face face);
};

#endif // _PLAYER_H