#include <_textureManager.h>

_textureManager::_textureManager() {

}

_textureManager::~_textureManager() {
    for (auto& it : textureMap) {
        if (it.second.ID != 0) {
            glDeleteTextures(1, &it.second.ID);
        }
    }
    textureMap.clear();
}

bool _textureManager::addTexture(const string &fileName) {
    GLuint textureID = 0;
    int width;
    int height;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D,textureID);

    unsigned char* image = SOIL_load_image(fileName.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
    if (!image) {
        cerr << "ERROR: Unable to load texture: " << fileName << "\n";
        return false;
    }

    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,image); // Copies the image data into GPU memory
    SOIL_free_image_data(image); // Free the image data from CPU memory since it's now in GPU memory

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    texture_entry newEntry;
    newEntry.ID = textureID;
    newEntry.width = width;
    newEntry.height = height;

    textureMap[fileName] = newEntry;

    return true;
}

bool _textureManager::removeTexture(const string &fileName) {
    auto it = textureMap.find(fileName);
    if (it == textureMap.end()) {
        cout << "ERROR: Unable to remove texture: " << fileName << " as it does not exist\n";
        return false;
    }
    glDeleteTextures(1, &it->second.ID);
    textureMap.erase(it);
    return true;
}

GLuint _textureManager::getTexture(const string &fileName) const {
    auto it = textureMap.find(fileName);
    if (it == textureMap.end()) {
        cout << "ERROR: Unable to retrieve texture: " << fileName << "\n";
        return 0;
    }
    return it->second.ID;
}

Vec2f _textureManager::getTextureDimensions(const string &fileName) const {
    auto it = textureMap.find(fileName);
    if (it == textureMap.end()) {
        cout << "WARNING: Unable to retrieve dimensions of texture: " << fileName << "\n";
        return Vec2f(0.0f,0.0);
    }
    return Vec2f(static_cast<float>(it->second.width),static_cast<float>(it->second.height));
}
