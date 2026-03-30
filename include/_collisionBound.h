#ifndef _COLLISION_BOUND_H
#define _COLLISION_BOUND_H

#include <_common.h>

class _collisionBound {
    public:
        _collisionBound();
        virtual ~_collisionBound();

        /**
         * Establishes a pointer to a position that keeps the local collisionBound position locked to the pointer
         * 
         * @param _pos Pointer to a position value to track
         */
        void setLockedPosition(const Vec2f* _pos);
        
        // Frees the collision bound from its pointer position. Allows the setPositon function to work if locked
        void freeLockedPosition();

        // Returns true if the collisionBound is currently locked to a pointer position
        bool isLocked() const;

        /**
         * Sets the position of the collision bound
         * 
         * @param _pos Position to set
         */
        void setPosition(const Vec2f &_pos);

        /**
         * Sets the position of the collision bound
         * 
         * @param _pos Position to set
         */
        void setPositionOffset(const Vec2f &_posOffset);

        /**
         * Sets the size of the collision bound
         * 
         * @param _pos Size to set
         */
        void setSize(const Vec2f &_size);

        /**
         * Performs AABB collision check on one other collision bound
         * 
         * @param other Collision box to compare against
         * @return True if colliding
         */
        bool isColliding(_collisionBound &other);

        /**
         * Perfroms penetration collision check on one other collision bound
         * 
         * +X = LEFT 
         * 
         * -X = RIGHT 
         * 
         * +Y = BOTTOM 
         * 
         * -Y = TOP 
         * 
         * @param other Collision box to compare penetration on
         * @return Vec2f for x/y components of penetration
         */
        Vec2f getPenetration(_collisionBound &other);

        // Draws the bounds of the box in red
        void drawBound();

        // Returns the locked position if locked, else the free position
        Vec2f getPos() const;
    protected:
    private:
        const Vec2f* posLocked = nullptr; // Const because we dont want to change the pos were tracking
        Vec2f pos = {0.0f,0.0f};
        Vec2f posOffset = {0.0f,0.0f};
        Vec2f size = {0.0f,0.0f};

        // Sends the values of min/max to their respective values
        void setBounds();

        float xmin = 0.0f;
        float xmax = 0.0f;
        float ymax = 0.0f;
        float ymin = 0.0f;
};

#endif // _COLLISION_BOUND_H