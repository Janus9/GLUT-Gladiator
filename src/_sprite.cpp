#include <_sprite.h>

_sprite::_sprite()
{
    //ctor
    pos = {0.0f, 0.0f};
    rot = {0.0f, 0.0f};
    color = {1.0f, 1.0f, 1.0f};
    
    velocity = {0.0f, 0.0f};
    acceleration = {0.0f, 0.0f};
}

_sprite::~_sprite()
{
    delete animationTimer;
    animationTimer = nullptr;    

    delete texture;
    texture = nullptr;
}

void _sprite::spriteInit(const std::string& fileName, int _xFrames, int _yFrames, int _FPS)
{
    xFrames = _xFrames;
    yFrames = _yFrames;

    FPS = _FPS;

    fpsDelay = 1.0/FPS * 1000; // Sets FPS to a ms delay

    if (texture->loadTexture(fileName) == 0) {
        Logger.LogError("Failed to initialize sprite with texture: " + fileName, LOG_BOTH);
        texture->loadTexture("images/missing_texture.png"); // Load a default missing texture
    } else {
        Logger.LogInfo("Sprite initialized with texture: " + fileName, LOG_BOTH);
    }

    animationTimer->reset();
}

void _sprite::drawSprite()
{
    glPushMatrix();
        texture->bindTexture();

        glTranslatef(pos.x, pos.y, 0.0f);

        glColor3f(color.r, color.g, color.b);

        glRotatef(rot.x,1,0,0);
        glRotatef(rot.y,0,1,0);

        float u0 = frameX * (1.0f/xFrames);
        float v0 = frameY * (1.0f/yFrames);
        float u1 = (frameX+1) * (1.0f/xFrames);
        float v1 = (frameY+1) * (1.0f/yFrames);

        glBegin(GL_QUADS);
            // bottom-left
            glTexCoord2f(u0, v1); glVertex2f(0.0f, 0.0f);
            // bottom-right
            glTexCoord2f(u1, v1); glVertex2f(spriteWidth, 0.0f);
            // top-right
            glTexCoord2f(u1, v0); glVertex2f(spriteWidth, spriteHeight);
            // top-left
            glTexCoord2f(u0, v0); glVertex2f(0.0f, spriteHeight);
        glEnd();
    glPopMatrix();
}

void _sprite::updateSprite(const int rowIndex) 
{
    if (frameY != rowIndex) frameY = rowIndex;

    if (animationTimer->getTicks() > fpsDelay) {
        frameX = (frameX + 1) % xFrames;
        animationTimer->reset();
    }
}

void _sprite::stopAnimation()
{
    // This needs to be changed to whatever sprite default is (TODO)
    frameX = 0;
    animationTimer->reset();
}