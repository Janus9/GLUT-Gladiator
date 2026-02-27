#ifndef _SPRITE_H
#define _SPRITE_H

#include <_common.h>
#include <_texture.h>
#include <_timerPlusPlus.h>

class _sprite {
    public:
        _sprite();
        virtual ~_sprite();

        const float spriteWidth = 32.0f; // Width of the sprite in world units
        const float spriteHeight = 32.0f; // Height of the sprite in world units

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
           - fps: the framerate of the animation when played (default is 24fps)
        */
        void spriteInit(const std::string& fileName, int _xFrames, int _yFrames, int _FPS = 24);
        void drawSprite();
        // Update loop handles primarily animations but can be configured for physics etc as well
        // Requires the row's index (so starting at 0) to be passed to decide which animation (for now)
        void updateSprite(const int rowIndex);
        void stopAnimation();
    protected:
    private:
        _timerPlusPlus* animationTimer = new _timerPlusPlus();
        _texture* texture = new _texture();

        int frameX = 0;   // Which X frame the animation is currently on
        int frameY = 0;   // Which "reel" the animation is on (just which row)

        int xFrames = 0;  // How many frames we are wide
        int yFrames = 0;  // How many frames we are tall

        int FPS = 0;

        double fpsDelay = 0.0; // Delay for the animation loop (in ms)
};

#endif // _SPRITE_H