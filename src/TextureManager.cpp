#include <TextureManager.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TOML_HEADER_ONLY 1
#include <toml.hpp>

// -- PUBLIC -- //

TextureManager::TextureManager() {
    
}

TextureManager::~TextureManager() {
    GG_LOG_INFO(
        LOG_TEXTURE,
        "Cleaning up the texture manager"
    );

    cleanTextures();
   
    GG_LOG_INFO(
        LOG_TEXTURE,
        "Finished cleaning up the texture manager"
    );
}

bool TextureManager::reload() {
    GG_LOG_INFO(
        LOG_TEXTURE, 
        "Reloading the texture manager"
    );

    // Remove old entries & clean VRAM
    cleanTextures();

    // Parse into TOML table
    toml::table config;

    try {
        config = toml::parse_file(CONFIG_PATH);
    } catch (const toml::parse_error &err) {
        GG_LOG_CRITICAL(
            LOG_TEXTURE, 
            "Failed to read '%s', reason: '%s'",
            CONFIG_PATH.c_str(), 
            err.what()
        );

        return false;
    }

    // Parse into a flat array
    toml::array* textures = config["textures"].as_array();
    if (!textures) {
        GG_LOG_CRITICAL(
            LOG_TEXTURE,
            "Failed to parse config into an array"
        );

        return false;
    }

    GG_LOG_DEBUG(
        LOG_TEXTURE, 
        "Read [%llu] images from '%s'",
        textures->size(),
        CONFIG_PATH.c_str()
    );

    for (size_t i = 0; i < textures->size(); i++) {
        toml::node& item = textures->at(i);
        
        if (!item.is_string()) {
            GG_LOG_ERROR(
                LOG_TEXTURE,
                "Failed to read texture at index [%llu] because it is not a string",
                i
            );

            continue;
        }

        std::string path = item.value_or<std::string>("");

        if (path.empty()) {
            GG_LOG_ERROR(
                LOG_TEXTURE,
                "Failed to read texture at index [%llu] because it is an empty string",
                i
            );

            continue;
        }

        if (!addTexture(path)) {
            GG_LOG_ERROR(
                LOG_TEXTURE,
                "Unable to add texture '%s'",
                path.c_str()
            );
        }
    }

    GG_LOG_INFO(
        LOG_TEXTURE, 
        "Successfully finished reloading the texture manager"
    );

    return true;
}

TextureEntry TextureManager::getTextureEntry(const std::string &fileName) const {
    TextureEntry entry;
    
    auto it = textureMap.find(fileName);

    if (it == textureMap.end()) {
        GG_LOG_ERROR(
            LOG_TEXTURE,
            "Unable to retrieve texture: '%s'",
            fileName.c_str()
        );
        return entry; // Empty entry (all zeros)
    }

    entry.id = it->second.id;
    entry.width = it->second.width;
    entry.height = it->second.height;

    return entry;
}

// -- PRIVATE -- //

void TextureManager::cleanTextures() {
    // Delete textures from GPU VRAM
    for (auto& it : textureMap) {
        if (it.second.id != 0) {
            glDeleteTextures(1, &it.second.id);
        }
    }

    GG_LOG_DEBUG(
        LOG_TEXTURE,
        "Cleaned [%llu] textures from GPU VRAM",
        textureMap.size()
    );

    textureMap.clear();
}

bool TextureManager::addTexture(const std::string &fileName) {
    GG_LOG_DEBUG(
        LOG_TEXTURE,
        "Adding texture '%s' to the texture manager",
        fileName.c_str()
    );

    GLuint textureID = 0;

    int width = 0;
    int height = 0;
    int channels = 0;

    glGenTextures(1, &textureID);               // Generate unique texture ID
    glBindTexture(GL_TEXTURE_2D,textureID);     // Set API to use this ID for proceeding commands

    unsigned char* image = stbi_load(fileName.c_str(), &width, &height, &channels, 4);

    if (!image) {
        GG_LOG_ERROR(
            LOG_TEXTURE,
            "Unable to stbi_load texture: '%s'",
            fileName.c_str()
        );

        return false;
    }

    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,image); // Copies the image data into GPU memory
    
    stbi_image_free(image); // Free the image data from CPU memory since it's now in GPU memory

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    TextureEntry newEntry;
    newEntry.id = textureID;
    newEntry.width = width;
    newEntry.height = height;

    textureMap[fileName] = newEntry;

    return true;
}