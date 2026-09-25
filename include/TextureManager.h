#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <_common.h>

struct TextureEntry {
    GLuint id = 0;          // Unique ID
    int width = 0;          // Image Width (in pixels)
    int height = 0;         // Image Height (in pixels)
};

class TextureManager {
    public:
        /**
         * Call `reload()` after instantiation.
         */
        TextureManager();
        virtual ~TextureManager();

        /**
         * Reloads the texture manager's texture map. 
         * 
         * This can be called at runtime to reload new textures. This may stall the program.
         * 
         * @return True if reload was successfull, False if texture file could not be parsed (critical). 
         */
        bool reload();
        
        /**
         * Gets a texture entry from the manager.
         * 
         * The id and width/height will all be 0 if entry was not found.
         * 
         * @param fileName Image file to get ID from.
         * @return Texture entry containing texture ID, and dimensions.
         */
        TextureEntry getTextureEntry(const std::string &fileName) const;

    protected:
    private:
        const std::string CONFIG_PATH = "configs/textures.toml";

        // Removes textures from GPU VRAM and clears the textureMap
        void cleanTextures();

        bool addTexture(const std::string &fileName);

        std::unordered_map<std::string, TextureEntry> textureMap;   // Map of all the unique IDs (string is the file directory)
};

#endif // TEXTURE_MANAGER_H