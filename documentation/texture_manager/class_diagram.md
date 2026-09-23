``` mermaid
    classDiagram
        direction LR
        class TextureEntry {
            <<struct>>
            GLuint id;
            int width;
            int height;
        }

        class TextureManager {
            <<class>>
            +TextureManager()
            +~TextureManager()
            +getTextureEntry(string texturePath) : TextureEntry 

            -addTexture(string texturePath) : bool
            -entryMap : unordered_map[string, TextureEntry]
        } 
        note for TextureManager "Constructor reads 'texture.toml' in configs and calls 'addTexture' on each entry.
        Textures cannot be removed once added."

        TextureManager "1" *-- "*" TextureEntry : owns
```