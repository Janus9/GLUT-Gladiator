#ifndef _TEXTURE_MANAGER_H
#define _TEXTURE_MANAGER_H

#include <_common.h>
#include<SOIL2.h>

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
         * Gets a textureID from the manager
         * 
         * @param fileName Image file to get ID from
         * @return Texture ID (0 if texture was not found)
         */
        GLuint getTexture(const string &fileName) const;

        /**
         * Get the image dimensions in pixels
         * 
         * If image does not exist returned vector is {0.0, 0.0}
         * 
         * @param fileName Image file to get dimensions from
         * @return Vec2f of the image size (w/h in pixels)
         */
        Vec2f getTextureDimensions(const string &fileName) const;

    protected:
    private:
        struct texture_entry {
            GLuint ID;      // Unique ID
            int width;      // Image Width (in pixels)
            int height;     // Image Height (in pixels)
        };

        unordered_map<string, texture_entry> textureMap;   // Map of all the unique IDs (string is the file directory)
};

#endif // _TEXTURE_MANAGER_H