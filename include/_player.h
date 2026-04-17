#ifndef _PLAYER_H
#define _PLAYER_H

#include <_common.h>
#include <_unit.h>

enum player_face {
    PLAYER_N,
    PLAYER_NE,
    PLAYER_E,
    PLAYER_SE,
    PLAYER_S,
    PLAYER_SW,
    PLAYER_W,
    PLAYER_NW
};

enum player_action {
    PLAYER_WALK,
    PLAYER_WALK_GUN
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

    protected:
    private:
};

#endif // _PLAYER_H