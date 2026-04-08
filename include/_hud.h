/**
 * _hudElement.h
 * 
 * A hudElement is a multi-use container for drawing elements as UI components for the game. 
 * hudElements can be text, sprites, or primitive shapes and feature interactivity including click events. 
 * They can feature animations by usage of sprites.
 * 
 * A hudElement 
 * 
 */

#ifndef _HUD_H
#define _HUD_H

#include <_common.h>
#include <_sprite.h>

class _hudElement {
    public:
        _hudElement();
        virtual ~_hudElement();

        // Base implementation -- does nothing
        virtual void draw();

        Col3f color {1.0f,1.0f,1.0f}; // Default to white
        Vec2f position = {0.0f, 0.0f};
        Vec2f size = {0.0f, 0.0f};
        float opacity = 1.0f;
        // Overrides ocapcity
        bool visible = true;
    protected:
    private:
};

class _hudText : public _hudElement {
    public:
        _hudText();
        virtual ~_hudText();

        virtual void draw() override;

        /**
         * @param void* GLUT font definition (ex/ GLUT_BITMAP_9_BY_15)
         */
        void setFont(void* _font);

        // Sets text and automatically updates the size by the contents of the text and font
        void setText(const string &_text);

        // Returns the width of the current text content + font
        int getTextWidth() const;
        // Returns the height of the current text content + font
        int getTextHeight() const;

        // Returns a readonly reference to the current text
        const string& getText() const;
    protected:
    private:
        string text = ""; 
        void* font = GLUT_BITMAP_9_BY_15;
};

/**
 * Features a sprite as an interactive
 */
class _hudSprite : public _hudElement {
    public:
        _hudSprite();
        virtual ~_hudSprite();

        // Returns a pointer to the sprite
        _sprite* getSprite() const;

        /**
         * Sets the sprite's position to element position and draws the sprite
         */
        virtual void draw() override;

        /**
         * @param mousePos Position of the mouse (in screen coordinates)
         * 
         * @return True if mouse is hovering the sprite
         */
        bool isMouseHovering(const Vec2f &mousePos) const;
    protected:
    private:
        _sprite* sprite = new _sprite();
};

class _hud {
    public:
        _hud();
        virtual ~_hud();

        // -- Hud Element -- //

        /**
         * Removes a hud element associated with the provided name
         * 
         * @param name Name of the hud element
         * 
         * @return True if found and removed. False if not found
         */
        bool removeHudElement(const string &name);

        // -- Hud Text -- //

        /**
         * Adds a new hud text child to the hud
         * 
         * Overwrites existing hud element associated with the provided name if it exists. 
         * Safe to use same name for different hud element variants.
         * 
         * @param name Name associated to new hud text for future lookups
         */
        void addHudText(const string &name);

        /**
         * Looks up a hud text by name in the table
         * 
         * @param name Name associated with the hud text
         * 
         * @return Pointer to the hud text or nullptr if not found
         */
        _hudText* getHudText(const string &name) const;

        // -- Hud Sprite -- //

        /**
         * Adds a new hud sprite child to the hud
         * 
         * Overwrites existing hud element associated with the provided name if it exists. 
         * Safe to use same name for different hud element variants.
         * 
         * @param name Name associated to new hud sprite for future lookups
         */
        void addHudSprite(const string &name);

        /**
         * Looks up a hud sprite by name in the table
         * 
         * @param name Name associated with the hud sprite
         * 
         * @return Pointer to the hud sprite or nullptr if not found
         */
        _hudSprite* getHudSprite(const string &name) const;

        // -- Other -- //

        /**
         * Statically sets the width/height of the viewport. 
         * 
         * @param width Window width (in pixels)
         * @param height Window height (in pixels)
         */
        static void setHudViewportDimensions(double width, double height);


        // Returns number of children the hud has
        int getNumChildren() const;

        // Draws all children of the hud
        void drawHud();

    protected:
    private:
        vector<_hudElement*> childrenList; // For fast iterative looping 
        unordered_map<string,_hudElement*> childrenMap; // For lookups

        // Static Viewport Dimensions
        static double Wwidth;
        static double Wheight;

        const string text_prefix = "TEXT_";  // Applied to name for the hashmap to prevent collisions with other hudElement variants
        const string sprite_prefix = "SPRITE_";  // Applied to name for the hashmap to prevent collisions with other hudElement variants
        
        enum elementType { HUD_SPRITE, HUD_TEXT };
        void addHudElement(const string &name, elementType type);
};

#endif // _HUD_H