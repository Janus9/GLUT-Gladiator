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

    // -- ANIMATIONS -- //

    // Walk Animation //
    setupSprite("WALK");
    getSprite("WALK")->initSprite("images/player/Walk/Normal/walk.png", 8, 6, sprite_direction::LEFT,12); // No natural direction due to top down

    getSprite("WALK")->createSpriteAction(sprite_action("WALK_S",0,0,7));
    getSprite("WALK")->createSpriteAction(sprite_action("WALK_SW",1,0,7));
    getSprite("WALK")->createSpriteAction(sprite_action("WALK_NW",2,0,7));
    getSprite("WALK")->createSpriteAction(sprite_action("WALK_N",3,0,7));
    getSprite("WALK")->createSpriteAction(sprite_action("WALK_NE",4,0,7));
    getSprite("WALK")->createSpriteAction(sprite_action("WALK_SE",5,0,7));
    getSprite("WALK")->offsetPoint = {0.0f, 8.0f};

}

void _player::drawPlayer() {
    drawUnitSingular();
}

void _player::setAction(player_action action, player_face face) {
    switch (action) {
        case PLAYER_WALK:
            break;
        case PLAYER_WALK_GUN:
            break;
    }
}