#ifndef _TEXTURE_H
#define _TEXTURE_H

#include<_common.h>
#include<SOIL2.h>

/*
Allows for loading textures from image files to be bound into textureIds, and stored in GPU memory.
For more information visit https://learnopengl.com/Getting-started/Textures
*/
class _texture
{
    public:
        _texture();
        virtual ~_texture();

        // Loads a given image file into GPU memory and returns a textureId.
        GLuint loadTexture(char *);    
        // Binds a loaded texture. Used prior to calling draw function on a given object.
        void bindTexture();            

        unsigned char *image;          // To handle image data
        int width,height;              // image width & height

        GLuint textID;                 // Image data buffer handler

    protected:

    private:
};

#endif // _TEXTURE_H
