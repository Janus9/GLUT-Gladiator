#ifndef _UNIT_H
#define _UNIT_H

#include <_common.h>
#include <_sprite.h>
#include <_collisionBound.h>

enum class collision_direction {
    LEFT = 0,
    RIGHT = 1,
    TOP = 2,
    BOTTOM = 3
};

class _unit {
    public:
        _unit();
        virtual ~_unit();

        /**
         * Sets up a new sprite by name
         * 
         * Unit can have multiple sprites, organized by name
         * 
         * @param spriteName Name for the sprite to be set. Used in getSprite(name)
         */
        void setupSprite(const string &spriteName);

        /**
         * 
         *  * DEPRICATED *  
         * 
         * NOT HOOKED UP TO SHADER PIPELINE -- DEPRICATE SOON
         * 
         * Draws a unit
         * 
         * If setup with a sprite then overwrites pos/scale/color before drawing sprite. Rotation exists only on sprite level.
         * 
         * Draws all sprites in the unit. Use "drawUnitSingular" to draw a single sprite at a time.
         */
        void drawUnit();

        /**
         * 
         *  * DEPRICATED *  
         * 
         * NOT HOOKED UP TO SHADER PIPELINE -- DEPRICATE SOON
         * 
         * Draws a unit
         * 
         * If setup with a sprite then overwrites pos/scale/color before drawing sprite. Rotation exists only on sprite level.
         * 
         * Draws the set singular sprite. Does NOT draw all sprites when used.
         */
        void drawUnitSingular();

        /** Updates all sprites registered to the unit */
        void updateUnit(double dt);

        /**
         * Applies physics calculations (acceleration/velocity/position) to unit
         * 
         * @param dt Delta time (in seconds) for physics calculations
         */
        void applyPhysics(double dt);

        /**
         * Gets sprite tied to unit by name
         * 
         * @return _sprite pointer or nullptr if never set
         */ 
        _sprite* getSprite(const string &spriteName);

        /**
         * Creates a collision box for the current object. Position is automatically locked to the unit's position.
         * 
         * @param size Vec2f size for the box
         */
        void setCollisionBox(const Vec2f &size, const Vec2f &offset = {0.0f, 0.0f});

        /**
         * Performs an AABB collision check on another unit
         * 
         * @param other Other unit to check collision against
         * @return True if colliding with other unit
         */
        bool isColliding(const _unit &other);

        /**
         * Performs a penetration collision test on another unit
         *          
         * +X = LEFT 
         * 
         * -X = RIGHT 
         * 
         * +Y = BOTTOM 
         * 
         * -Y = TOP 
         * 
         * @param other Other unit to check collision against
         * @return Vec2f for x/y components of penetration
         */
        Vec2f getPenetration(const _unit &other);
        
        // Returns the collision box of the current element
        _collisionBound* getCollisionBound() const;

        /**
         * Rotates only the single sprite to focus on the position selected. 
         * Does not apply to the whole unit.
         * 
         * @param _pos Position to focus on
         * @param speed Speed unit rotates (in degrees/s). Negative or 0.0 means instant rotation.
         * @param degreeTolerance Amount of tolerance in focusing for degrees +/- (defaults to +/- 5.0 degrees)
         * @param sprite Sprite to apply rotation on (leave blank for nullptr which applies to single sprite)
         * 
         * @return True when focused on target 
         */
        bool focusOn(const Vec2f &_pos, float speed, float degreeTolerance = 5.0f, _sprite* sprite = nullptr);

        /**
         * Sets the singular sprite.
         * 
         * Usage ex/ unit.setSingleSprite(unit.getSprite("TEST"));
         *
         * Can clear by entering nullptr, not recommended.
         */
        void setSingleSprite(_sprite* sprite);

        // Returns the single sprite pointer. Or nullptr if not found.
        _sprite* getSingleSprite() const;

        // Clears the single sprite to nullptr
        void clearSingleSprite();
        
        // Sets max health to amount
        void setMaxHealth(float amount);
        
        // Returns max health
        float getMaxHealth() const;
        
        // Returns current health
        float getHealth() const;

        // Returns health as a % from 0 - 1
        float getHealthPct() const; 

        // Sets health to an amount (negatives are rounded to 0)
        void setHealth(float amount);

        // Applies damage
        void impulseDamage(float amount);
        
        // Applies healing
        void impulseHealing(float amount);

        // Sets current health to max health
        void resetHealth();

        // Returns true if unit health is less than or equal to 0.0
        bool isDead() const;

        // Returns te unit's unique ID 
        int getID() const;

        /**
         * Adds data from each sprite in the unit
         * 
         * @param vboData Pointer to vboData to add to
         * @param vIndex Index in the array
         */
        void buildUnitVBO(float* vboData, int &vIndex) const;

        /** Returns a reference to the unit's list of sprites (readonly) */
        const vector<_sprite*>& getSpriteList() const;

        /** Returns the number of sprites registered to the unit */
        int getNumSprites() const;

        /** Sets the position of the unit and all it's sprites */
        void setPosition(const Vec2f &_pos);

        // OPEARTOR OVERLOADS  //
        bool operator==(const _unit &other) const;

        // Disable copying
        _unit(const _unit &other) = delete;
        _unit& operator=(const _unit &other) = delete;

        // Move Operations
        // noexcept tells the compiler that this function will never fail. It prevents vectors from defaulting to copy when moving instances
        _unit(_unit&& other) noexcept;
        _unit& operator=(_unit&& other) noexcept;

        // DEBUGGING FLAGS //
        bool DEBUG_display_collision_bounds = false; // If true displays the display bounds

        // UNIT VARIABLES //

        Vec2f scale = {1.0f, 1.0f};  // Scale of a unit in multiplier (so 2,2 is a unit 2x width by 2x height). Default unit width/height is # of pixels
        Vec2f pos = {0.0f, 0.0f};    // Position of a unit in world space 
        Vec2f offsetPos = {0.0f, 0.0f};    // Offset Position of a unit in world space for side scrollers
        Col3f color = {1.0f, 1.0f, 1.0f}; // Color of the given unit 

        Vec2f acc = {0.0f, 0.0f};
        Vec2f vel = {0.0f, 0.0f};

        _team team = _team::NEUTRAL;
    protected:
    private:
        float currentHealth = 100.0f;   // Current health unit sits at
        float maxHealth = 100.0f;       // Max health unit can hold

        _sprite* singleSprite = nullptr;             // Pointer to single sprite if unit only draws on at a time (common)
        vector<_sprite*> spriteList;                 // Vector for draw iteration
        unordered_map<string,_sprite*> spriteMap;    // Hashmap for lookups

        unique_ptr<_collisionBound> collisionBox = nullptr;

        int unitID;
        static int nextID;
};

#endif // _UNIT_H