#ifndef _TEXTURE_MANAGER_H
#define _TEXTURE_MANAGER_H

#include <_common.h>
#include<SOIL2.h>

struct texture_entry {
    GLuint ID = 0;          // Unique ID
    int width = 0;          // Image Width (in pixels)
    int height = 0;         // Image Height (in pixels)
};

class _textureManager {
    public:
        _textureManager();
        virtual ~_textureManager();

        /**
         * Adds a new texture to the manager
         * 
         * Overrides existing textures
         * 
         * @param fileName Image file to add to manager
         * @return True if texture was added successfully
         */
        bool addTexture(const string &fileName);
        
        /**
         * Removes a texture from the manager 
         * 
         * @param fileName Image file to remove from manager
         * @return True if image was removed successfully (false if not)
         */
        bool removeTexture(const string &fileName);
        
        /**
         * Gets a texture entry from the manager
         * 
         * TextureID and dimensions will all be 0 if entry was not found
         * 
         * @param fileName Image file to get ID from
         * @return Texture entry containing texture ID, and dimensions
         */
        texture_entry getTextureEntry(const string &fileName) const;
    protected:
    private:
        unordered_map<string, texture_entry> textureMap;   // Map of all the unique IDs (string is the file directory)
};

#endif // _TEXTURE_MANAGER_H