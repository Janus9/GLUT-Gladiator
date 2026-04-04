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


class _hud {
    public:
        _hud();
        virtual ~_hud();

        /**
         * Statically sets the width/height of the viewport. 
         * 
         * @param width Window width (in pixels)
         * @param height Window height (in pixels)
         */
        static void setHudViewportDimensions(double width, double height);

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
         * Removes a hud text associated with the provided name
         * 
         * @param name Name of the hud text
         * 
         * @return True if found and removed. False if not found
         */
        bool removeHudText(const string &name);
        // todo add sprite support

        /**
         * Looks up a hud text by name in the table
         * 
         * @param name Name associated with the hud text
         * 
         * @return Pointer to the hud text or nullptr if not found
         */
        _hudText* getHudText(const string &name) const;

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
};

#endif // _HUD_H