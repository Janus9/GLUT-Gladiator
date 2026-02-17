#ifndef _SPRITE_H
#define _SPRITE_H

#include <_common.h>
#include <_texture.h>

class _sprite {
    private:
    protected:
    public:
        _sprite();
        virtual ~_sprite();

        float tileW = 16.0f; // Width of a single tile in world units
        float tileH = 16.0f; // Height of a single tile in world units

        // needs texture loader
        GLuint textureId; // Texture ID for the sprite's texture
        // world //
        Vec2f pos; 
        Vec2f rot;
        Col3f color;
        // physics //
        Vec2f velocity; 
        Vec2f acceleration;

        /* Initializatin of a sprite
           - fileName: the file name of the texture to be used for the sprite
           - xFrames: the number of horizontal frames for the sprite sheet
           - yFrames: the number of vertical frames for the sprite sheet
        */
        void spriteInit(const std::string& fileName, int xFrames, int yFrames);
        void drawSprite();
};

#endif // _SPRITE_H