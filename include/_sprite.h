#ifndef _SPRITE_H
#define _SPRITE_H

#include <_common.h>

class _sprite {
    private:
    protected:
    public:
        _sprite();
        virtual ~_sprite();

        // needs texture loader

        // world //
        Vec3f pos;
        Vec3f rot;
        Vec3f scale;
        Col3f color;
        // physics //
        Vec2f velocity; 
        Vec2f acceleration;

        /* Initializatin of a sprite
        fileName: the file name of the texture to be used for the sprite
        xFrames: the number of horizontal frames for the sprite sheet
        yFrames: the number of vertical frames for the sprite sheet
        */
        void spriteInit(const std::string& fileName, int xFrames, int yFrames);
        void drawSprite();
};

#endif // _SPRITE_H