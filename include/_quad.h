#ifndef _QUAD_H
#define _QUAD_H

#include <_common.h>
#include <_texture.h>

class _quad {
    public:
        _quad();
        virtual ~_quad();

        Vec3f rot;
        Vec3f pos;
        Vec3f scale;
        Col3f color;

        float size = 32.0f;

        void initQuad(std::string fileName);
        void drawQuad();
        void updateQuad();

        GLuint vboPos, vboTex;

        /*
        Texture coordinate positions for the quad
        */
        float xMin = 0;
        float xMax = 0;
        float yMin = 0;
        float yMax = 0;

        float verticies[12] = {
            size, size, 0.0f,    // Top Right
            size, -size, 0.0f,   // Bottom Right
            -size, -size, 0.0f,  // Bottom Left
            -size, size, 0.0f    // Top Left 
        };

        float texCoord[8] = {
            xMax, yMax, //  
            xMax, yMin, //
            xMin, yMin, //
            xMin, yMax  //
        };

    protected:
    private:
        _texture* quadTexture = new _texture();
};

#endif // _QUAD_H