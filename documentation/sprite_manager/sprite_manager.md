# Sprite Manager System Diagram

## Flow Chart

TODO

## Class Diagram

``` mermaid
    classDiagram

        namespace texture {
            class TextureEntry {
                <<struct>>
                id : GLuint
                width : int
                height : int
            }

            class TextureManager {
                <<singleton instance>>
                +reload : bool
                -textureMap : unorderedMap[string, TextureEntry]
            }
        }


        namespace sprite {
            class SpriteSystem["System"] {
                <<class>>
                System()
                ~System()

                +manager : Manager
                +engine : Engine
            }

            class SpriteManager["Manager"] {
                <<class>>
                +createSprite() uint64
                +removeSprite(uint64 spriteID) bool

                -engine : EnginePtr
                -spriteMap : unordered_set[int]
                -spriteList : vector[uint64]
            }

            class SpriteEngine["Engine"] {
                <<class>>
                +update() void
                +draw() void

                -manager : ManagerPtr
            }
        }

        namespace common {
            class CommonUVMap["UVMap"] {
                <<struct>>
                u0 : float
                u1 : float
                v0 : float
                v1 : float
            }
        }

        namespace animation {

            class Direction {
                <<enum class>>
                NONE
                NORTH
                NORTH_EAST
                EAST
                SOUTH_EAST
                SOUTH
                SOUTH_WEST
                WEST
                NORTH_WEST
            }

            class AnimationData["Data"] {
                <<struct>>
                config : Config
                textureEntry : TextureEntry
                frameData : vector[UVMap]
            }

            class AnimationTextureData["TextureData"] {
                <<struct>>
                textureEntry : TextureEntry
                uvMap : UVMap             
            }

            class AnimationInstance["Instance"] {
                <<struct>>
                animationID : string
                spriteID : uint64
                frameIndex : int
                frameChangeDelta : float
            }

            class Config {
                <<struct>>
                string texturePath
                
                int textureNumColumns
                int textureNumRows

                int startColumn
                int stopColumn

                int startRow
                int stopRow

                Direction direction

                int defaultFPS

                bool reverseAnimation
                bool pingPongAnimation
            }

            class AnimationSystem["System"] {
                <<class>>
                System()
                ~System()

                +manager : AnimationManager
                +engine : Engine
            }

            class AnimationManager["Manager"] {
                <<class>>
                Manager(TextureManager &textureManager)
                ~Manager()

                +registerAnimation(string animationID, Config config) bool
                +playSingleAnimation(uint64 spriteID, string animationID) bool
                +playSingleAnimation(uint64 spriteID, string animationID, int animationFPS) bool
                +playLoopedAnimation(uint64 spriteID, string animationID) bool
                +playLoopedAnimation(uint64 spriteID, string animationID, int animationFPS) bool
                +stopLoopedAnimation(uint64 spriteID, string animationID) bool

                -registrationMap : unordered_map[string, Data]
                -instanceList : vector[Instance]
                -instanceMap : unordered_map[spriteID, int]
                -engine : EnginePtr
            }

            class AnimationEngine["Engine"] {
                <<class>>
                Engine() 
                ~Engine() 

                +update(double dt) void
                +getTextureData(spriteID) TextureDataPtr

                -manager : ManagerPtr
            }
    }


    AnimationManager --> TextureManager : Requires
    AnimationManager --> AnimationEngine : Requires
    AnimationEngine --> AnimationManager : Requires
    
    Config ..> Direction : Uses type
    AnimationData "1" *.. "*" CommonUVMap : Owns
    AnimationData ..> Config : Uses type
    AnimationData ..> TextureEntry : Uses type
    AnimationTextureData ..> CommonUVMap : Uses type
    AnimationEngine ..> AnimationTextureData : Uses type
    AnimationTextureData ..> TextureEntry : Uses type

    AnimationManager "1" *-- "*" AnimationData : Owns
    AnimationManager "1" *-- "*" AnimationInstance : Owns

    AnimationSystem "1" *-- "1" AnimationManager : Owns
    AnimationSystem "1" *-- "1" AnimationEngine : Owns

    SpriteSystem "1" *-- "1" SpriteManager : Owns
    SpriteSystem "1" *-- "1" SpriteEngine : Owns

    TextureManager "1" *-- "*" TextureEntry : Owns

    note for SpriteManager "The vector owns the memory, the map hold indices for the vector"
    note for AnimationManager "The vector owns the memory, the map hold indices for the vector"
```