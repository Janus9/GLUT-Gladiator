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
    //dtor
}

void _sprite::spriteInit(const std::string& fileName, int xFrames, int yFrames)
{
    // todo
    static _texture spriteTexture = _texture();
    textureId = spriteTexture.loadTexture(fileName);
    if (textureId == 0) {
        Logger.LogError("Failed to initialize sprite with texture: " + fileName, LOG_BOTH);
        textureId = spriteTexture.loadTexture("images/missing_texture.png"); // Load a default missing texture
    } else {
        Logger.LogInfo("Sprite initialized with texture: " + fileName, LOG_BOTH);
    }
}

void _sprite::drawSprite()
{
    // todo
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();

        glTranslatef(pos.x, pos.y, 0.0f);

        glRotatef(rot.x, 1.0f, 0.0f, 0.0f);
        glRotatef(rot.y, 0.0f, 1.0f, 0.0f);

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glColor4f(color.r, color.g, color.b, 1.0f);

        glBindTexture(GL_TEXTURE_2D, textureId);

        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 1.0f); glVertex2f(-tileW/2.0f, -tileH/2.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex2f( tileW/2.0f, -tileH/2.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex2f( tileW/2.0f,  tileH/2.0f);
            glTexCoord2f(0.0f, 0.0f); glVertex2f(-tileW/2.0f,  tileH/2.0f);
        glEnd();

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}