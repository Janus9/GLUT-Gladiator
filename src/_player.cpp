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
    
    getSprite("WALK")->createSpriteAction(sprite_action("WALK_N",3,0,7));
    getSprite("WALK")->createSpriteAction(sprite_action("WALK_NE",4,0,7));
    getSprite("WALK")->createSpriteAction(sprite_action("WALK_E",5,0,7));
    getSprite("WALK")->createSpriteAction(sprite_action("WALK_SE",5,0,7));
    getSprite("WALK")->createSpriteAction(sprite_action("WALK_S",0,0,7));
    getSprite("WALK")->createSpriteAction(sprite_action("WALK_SW",1,0,7));
    getSprite("WALK")->createSpriteAction(sprite_action("WALK_W",1,0,7));
    getSprite("WALK")->createSpriteAction(sprite_action("WALK_NW",2,0,7));
    getSprite("WALK")->offsetPoint = {0.0f, 8.0f};

    animationTable[PLAYER_ACTION_WALK][PLAYER_FACE_N] = {"WALK","WALK_N",{0,3}};
    animationTable[PLAYER_ACTION_WALK][PLAYER_FACE_NE] = {"WALK","WALK_NE",{0,4}};
    animationTable[PLAYER_ACTION_WALK][PLAYER_FACE_E] = {"WALK","WALK_E",{0,5}};
    animationTable[PLAYER_ACTION_WALK][PLAYER_FACE_SE] = {"WALK","WALK_SE",{0,5}};
    animationTable[PLAYER_ACTION_WALK][PLAYER_FACE_S] = {"WALK","WALK_S",{0,0}};
    animationTable[PLAYER_ACTION_WALK][PLAYER_FACE_SW] = {"WALK","WALK_SW",{0,1}};
    animationTable[PLAYER_ACTION_WALK][PLAYER_FACE_W] = {"WALK","WALK_W",{0,2}};
    animationTable[PLAYER_ACTION_WALK][PLAYER_FACE_NW] = {"WALK","WALK_NW",{0,2}};
}

void _player::drawPlayer() {
    drawUnitSingular();
}

void _player::setAction(player_action action, player_face face) {
    if (currentResult == animationTable[action][face]) return; // Already loaded the same action, skip
    currentResult = animationTable[action][face];
    if (action < PLAYER_ACTION_COUNT && face < PLAYER_FACE_COUNT) {
        string s_sprite = animationTable[action][face].sprite;
        string s_action = animationTable[action][face].action;
        Vec2i idleFrame = animationTable[action][face].idleFrame;
        
        _sprite* sprite = getSprite(s_sprite);
        if (sprite) {
            sprite->loadSpriteAction(s_action);
            sprite->setIdleFrame(idleFrame.x,idleFrame.y);
            sprite->startAnimation();
            setSingleSprite(sprite);
        }
    }
}

void _player::stopAction(player_action action) {
    _sprite* sprite = getSprite(currentResult.sprite);
    if (sprite) {
        sprite->stopAnimation();
    }
}

