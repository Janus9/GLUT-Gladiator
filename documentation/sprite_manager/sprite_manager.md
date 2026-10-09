# Sprite Manager System Diagram

## Flow Chart

``` mermaid
    flowchart TD
        SR["Sprite Registry"] --> SD["Shared Sprite Definition"]
        EM["Enemy Manager"] --> E["Enemy"]
        E --> U["Unit"]
        U --> AI["Animation Instance"]
        AI --> SD
        EM --> R["Sprite Renderer"]
        R --> SR
```

## Class Diagram

``` mermaid
    classDiagram
        class TextureManager {
            <<singleton instance>>
        }

        namespace sprite {
            class SpriteManager["Manager"] {
                <<class>>
                +createSprite() : uint64
                +removeSprite(uint64 spriteID) : bool

                -spriteMap : unordered_set[uint64]
                -spriteList : vector[uint64]
            }

            class SpriteEngine["Engine"] {
                <<class>>
                +update() : void
                +draw() : void
            }
        }

        namespace animation {

            class AnimationFrame["Frame"] {
                <<struct>>
                uint8 frameX
                uint8 frameY
            }

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
                string animationID
                int frameX
                int frameY
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

            class AnimationRegistration["Registration"] {
                <<struct>>
                Config config
                std::array[Frame] reel  
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
                //+setIdleFrame(int frame) void
                //+getCurrentAnimation() string
                //+isPlayingAnimation() bool

                -animationMap : unordered_map[string, Config]
                -filmRoll : array[AnimationFrame]
            }
    }

    note for AnimationManager "Todo"
    note for AnimationData "Holds data of animation active frame etc"
    note for AnimationFrame "IDEA: Subject to change"

    note for later "For Later: Preference making things setup during initialization or pre-baked (files etc) to save update/draw performance for many animations"

    Config --> AnimationManager
    Direction --> AnimationManager
    AnimationManager ..> TextureManager : Requires
    SpriteEngine --> AnimationManager : Requires
```