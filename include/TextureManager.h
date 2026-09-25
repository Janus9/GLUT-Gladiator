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
         * Calls `reload()` on the texture manager. 
         * 
         * Check `getInitSuccessState()` to see if initialization was successfull.
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

        /**
         * @return Initialization success state. False means program should shutdown as no textures were able to be loaded.
         */
        bool getInitSuccessState() const { return initSuccess; }
    protected:
    private:
        const std::string CONFIG_PATH = "configs/texture.toml";

        bool addTexture(const std::string &fileName);

        bool initSuccess;

        std::unordered_map<std::string, TextureEntry> textureMap;   // Map of all the unique IDs (string is the file directory)
};

#endif // TEXTURE_MANAGER_H