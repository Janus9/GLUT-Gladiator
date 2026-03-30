#ifndef _SPRITE_H
#define _SPRITE_H

#include <_common.h>
#include <_texture.h>
#include <_timerPlusPlus.h>

// Enum for a sprite's direction
enum class sprite_direction {
    LEFT = 0,
    RIGHT = 1
};

// Structure for a sprite action
struct sprite_action {
    string action_name = "";                // A name of the action for usage
    int frame_row_index = 0;                // Which row of the sprite sheet is being used
    // We use start/stop positions so we can use a partial row
    int frame_column_start_index = 0;       // The start location of a row for animation
    int frame_column_stop_index = 0;        // The stop location of a row for animation

    // Defaults name to "", others to 0
    sprite_action() {
        // ctor
    }

    /**
     * Constructor
     * 
     * @param _action_name Name of sprite action to use when loading an action (acts as an index)
     * @param _frame_row_index Frame index for the row 
     * @param _frame_column_start_index Frame index animation starts at (min)
     * @param _frame_column_stop_index Frame index animation stops at (max)
     */
    sprite_action(const string &_action_name, int _frame_row_index, int _frame_column_start_index, int _frame_column_stop_index) {
        // Name setter cant fail, so set on top
        action_name = _action_name;
        if (_frame_row_index < 0 || _frame_column_start_index < 0 || _frame_column_stop_index < 0) {
            // Ensure on creation we have valid indexes, cant bound check for positive values until inside the sprite
            cerr << "ERROR: Values entered for " << action_name << "must be greater than 0\n";
            return;
        }
        frame_row_index = _frame_row_index;
        frame_column_start_index = _frame_column_start_index;
        frame_column_stop_index = _frame_column_stop_index; 
    }
};

/*
Sprite actions are normalized to the following
 - An action may be of any length, but must occupy only a single row
 - Each action will be assigned a length and a name by the implementer
 - Actions are normalized to the same FPS (for now)
 - Actions all must exist inside one texture map, does not support multiple textures.
*/

class _sprite {
    public:
        _sprite();
        virtual ~_sprite();

        /**
         * Initialize sprite from a texture atlas.
         * 
         * If the sprite is top-down or has no natural "direction" then the input does not matter
         *
         * @param fileName Path to the sprite sheet
         * @param _framesX Number of columns in the sheet
         * @param _framesY Number of rows in the sheet
         * @param direction Direction sprite sheet is facing
         * @param _FPS Animation rate (frames per second, default = 24)
         */
        void initSprite(const string &fileName, int _framesX, int _framesY, const sprite_direction &direction, int _FPS = 24);
        
        // Creates or overwrites a sprite action
        void createSpriteAction(const sprite_action &action);

        // Removes a given sprite action from the list of sprite actions
        void removeSpriteAction(const string &actionName);
        
        /**
         * Sets the current sprite action by name and sets the animation frames to start index
         * 
         * Safe to call repeatedly; nothing happens if already active
         * 
         * Safe to be called while animation active
         * 
         * @param actionName Name of action to load
         * @return true if action exists and was not already loaded
         */
        bool loadSpriteAction(const string &actionName);
        
        // Sets a direction for the sprite to face
        void setDirection(const sprite_direction &direction);
        
        // Draws the current sprite for its given animation (if in animation)
        void drawSprite();
        
        // Begins the animation loop on the current sprite (loop safe if ran while sprite is already animating it does nothing)
        /**
         * Starts animation playing
         * 
         * Safe to call repeatedly; nothing happens if animation already started
         * 
         * @return true if animation was not started and currentAction is valid
         */
        bool startAnimation();
        
        // Stops animation sequence. Sets draw frame to idle frame.
        void stopAnimation();

        /**
         * Plays an action a single time. On end stops animation and shows idleFrame.
         * 
         * Calling again on same action resets loop. Cannot be called in a loop without stalling animation.
         * 
         * @param actionName Name of action to play
         */
        void playAction(const string &actionName);
        
        // Sets a frame of the sheet to act as the "idle" for when stopAnimation is called
        void setIdleFrame(int _idleFrameX, int _idleFrameY);
        
        // Returns number of actions present
        int getNumAction() const;

        // Sets animation FPS
        void setFPS(int _FPS); 

        // Returns current animation FPS
        int getFPS() const;

        Vec2f scale = {1.0f, 1.0f};  // Scale of a sprite in multiplier (so 2,2 is a sprite 2x width by 2x height). Default sprite width/height is # of pixels
        Vec2f pos = {0.0f, 0.0f};    // Position of a sprite in world space 
        float rot = 0.0f; // Rotation for a 2D element is only 1 dimension (around Z-axis)
        
        Vec2f pivotPoint = {0.0f, 0.0f}; // Point the rot applies from
        Vec2f offsetPoint = {0.0f, 0.0f}; // Point image is offset to for alignment purposes

        Col3f color = {1.0f, 1.0f, 1.0f}; // Color of the given sprite 

        bool debugging = false; // If true performs certain debug logging
        bool DEBUG_showPivotPoint = false; // If true displays pivot location
        bool DEBUG_showOrigin = false; // If true displays sprite origin

        bool reverseAnimation = false; // If true the animation reel goes backwards (decreases rather than increases)
        bool hidden = false; // If false sprite draw is ignored
        /**
         * Sets the pixel drawing bounds to exact pixel values.
         * 
         * Performs bound checks with debugging outputs incase the start/stop are set as impossible to draw values
         * 
         * Pixels are indexes starting at 0 where 0 is the top of the image. 
         * @param _pixelDrawStart Which pixel for the image frame to start drawing from
         * @param _pixelDrawStop Which pixel for the image frame to stop drawing at
         */
        void setDrawBoundsExact(int _pixelDrawStart, int _pixelDrawStop);

        // Resets the draw bounds to be 100% the image frame
        void resetDrawBounds();

        // Returns the current sprite's ID (unique to each instance)
        int getID() const;

        /**
         * Checks the current sprite's ID to another sprite's ID to see if they match
         * 
         * @param other _sprite class to compare
         * @return true if sprite ID matches the other sprite ID
         */
        bool operator==(const _sprite &other) const;
    protected:
    private:
        // These are indexes so they start at 0
        int pixelDrawStart = 0; // Which Y pixel we start drawing at (0 is top of image)
        int pixelDrawStop = 0; // Which Y pixel we stop drawing at (pixelsY is bototm of image)

        _timerPlusPlus* animationTimer = new _timerPlusPlus();
        _texture* texture = new _texture();

        unordered_map<string, sprite_action> spriteActions; // List of actions that can be toggled for animation

        sprite_action* currentAction = nullptr; // The current action we use in draw

        sprite_direction defaultDirection = sprite_direction::LEFT; // Default direction a sprite faces due to the sprite sheet
        bool mirrorImage = false; // Flag to tell draw function to mirror image (for changing directions)

        bool playingAnimation = false; // Wether the sprite is playing an animation or not

        int currentFrameX = 0; // Which column frame we are currently drawing (as an index)
        int currentFrameY = 0; // Which row frame we are currently drawing (as an index)

        int framesX = 0;    // Number of frames wide the sprite sheet is
        int framesY = 0;    // Number of frames tall the sprite sheet is

        int idleFrameX = 0; // Which column frame of the sprite sheet (as an index) is the idle
        int idleFrameY = 0; // Which row frame of the sprite sheet (as an index) is the idle

        // Needed for sizing sprite drawing proper drawing scale
        int pixelsX = 0;    // How many pixels an action box occupies wide
        int pixelsY = 0;    // How many pixels an action box occupies tall

        int FPS = 0; // Refresh rate of the sprite animation loop (defaults to 24 FPS)
        double fpsDelay = 0.0; // Delay for the animation loop (in s)

        bool singleActionInProgress = false;

        int spriteID;
        static int nextID;
};

#endif // _SPRITE_H
