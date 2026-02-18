#include "_texture.h"

_texture::_texture()
{
    //ctor
    width = 0;
    height = 0;

    textID = 0;
    image = nullptr;
}

_texture::~_texture()
{
    //dtor
    if (textID != 0)
    {
        // Releases the texture from GPU memory
        glDeleteTextures(1, &textID);
    }
}

GLuint _texture::loadTexture(const std::string& fileName)
{
    glGenTextures(1,&textID);               // Creates an OpenGL texture and stores the ID in textID
    glBindTexture(GL_TEXTURE_2D,textID);    // Makes the texture "active"

    image = SOIL_load_image(fileName.c_str(),&width,&height,0,SOIL_LOAD_RGBA); 
    if (!image) {
        Logger.LogError("Couldn't load texture: " + std::string(fileName), LOG_BOTH);
        glBindTexture(GL_TEXTURE_2D, 0);
        textID = 0;     // mark invalid
        return 0;
    }
    Logger.LogInfo("Loaded texture: " + std::string(fileName) + " with width: " + std::to_string(width) + " and height: " + std::to_string(height), LOG_BOTH);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,image); // Copies the image data into GPU memory
    SOIL_free_image_data(image); // Free the image data from CPU memory since it's now in GPU memory

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return textID;
}

void _texture::bindTexture()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,textID);
}
