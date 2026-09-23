# Texture Manager System Design

## Class Diagram
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

## Flow Chart
``` mermaid
    flowchart TD
        Start(["`**Texture Manager Instantiation**`"])
        LogStart["Log info initialization start"]
        ReadConfig["Read 'config/textures.toml'"]
        ParseTable["Parse config into a TOML table object"]
        TableEnd_C["Another entry exists?"]
        TableEnd_C_F["`**- Debug Only -**
        Log debug count of entries`"]
        LogEnd["Log info manager initialization complete"]
        End(["`**End**`"])
        EntryValid_C{"Is entry valid?"}
        EntryValid_C_F["Log error"]
        AddTexture_M["addTexture(string texturePath)"]
        AddTexture_M_1["Generate texture ID"]
        AddTexture_M_2["Bind texture to ID"]
        AddTexture_M_3["Upload texture to VRAM"]
        AddTexture_M_4["Free texture from RAM"]
        AddTexture_M_5["Create TextureEntry object"]
        AddTexture_M_6["Add entry to entry map"]

        Start --> LogStart --> ReadConfig 

        ReadConfig --> ParseTable

        ParseTable --> TableEnd_C
        TableEnd_C --> |No| TableEnd_C_F
        TableEnd_C_F --> LogEnd
        LogEnd --> End
        TableEnd_C --> |Yes| EntryValid_C


        EntryValid_C
        EntryValid_C --> |No| EntryValid_C_F
        EntryValid_C --> |Yes| AddTexture_M
        
        AddTexture_M --> AddTexture_M_1 --> AddTexture_M_2 --> AddTexture_M_3 --> AddTexture_M_4 --> AddTexture_M_5 --> AddTexture_M_6
        AddTexture_M_6 --> TableEnd_C

        EntryValid_C_F --> TableEnd_C
```
