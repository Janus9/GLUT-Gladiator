``` mermaid
    classDiagram
        class Direction {
            <<enum>>
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

        class Config {
            <<struct>>
            int startColumn
            int stopColumn

            int startRow
            int stopRow

            Direction direction

            int defaultFPS

            bool reverseAnimation
            bool pingPongAnimation
        }

        class AnimationManager {
            +registerAnimation(string animationID, string filePath, int columns, int rows, Config config) bool
            +playSingleAnimation(uint64 spriteID, string animationID) bool
            +playSingleAnimation(uint64 spriteID, string animationID, int animationFPS) bool
            +playLoopedAnimation(uint64 spriteID, string animationID) bool
            +playLoopedAnimation(uint64 spriteID, string animationID, int animationFPS) bool
            +stopLoopedAnimation(uint64 spriteID, string animationID) bool
            +setIdleFrame(int frame) void
            +getCurrentAnimation() string
            +isPlayingAnimation() bool
        }

        note for AnimationManager "Todo"
```