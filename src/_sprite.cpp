#include <_sprite.h>

_sprite::_sprite()
{
    //ctor
    acceleration = {0, 0};
    velocity = {0, 0};

    rot.x = rot.y = rot.z = 0;

    pos.x =0;
    pos.y =0;
    pos.z = -3;
}

_sprite::~_sprite()
{
    //dtor
}

void _sprite::spriteInit(const std::string& fileName, int xFrames, int yFrames)
{
   // todo
}

void _sprite::drawSprite()
{
    // todo
}