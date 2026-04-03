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

        // Draws all children
        virtual void draw();

        /**
         * Adds a child
         * 
         * @param child Hud Element to add as a child to this object
         */
        void addChild(_hudElement* child);

        /**
         * Removes a child
         * 
         * @param child Hud element to remove
         * 
         * @return True if successful, False if not (child not found)
         */
        bool removeChild(_hudElement* child);

        /**
         * Returns parent of current hud element
         * 
         * @return Pointer to _hudElement 
         */
        _hudElement* getParent() const;

        /**
         * Returns child at index of current hud element
         * 
         * @return Pointer to _hudElement or nullptr if not found
         */
        _hudElement* getChild(int index) const;

        // Returns number of children the element has
        int getNumChildren() const;

        /**
         * Statically sets the width/height of the viewport. 
         * 
         * @param width Window width (in pixels)
         * @param height Window height (in pixels)
         */
        static void setHudViewportDimensions(double width, double height);

        Col3f color {1.0f,1.0f,1.0f}; // Default to white
        Vec2f position = {0.0f, 0.0f};
        Vec2f size = {0.0f, 0.0f};
        float ocapacity = 1.0f;
        // Overrides ocapcity
        bool visible = true;
    protected:
        // Static Viewport Dimensions
        static double Wwidth;
        static double Wheight;

        // Who owns this element
        _hudElement* parent = nullptr;
        // Children of the element
        vector<_hudElement*> children;
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
        void setFont(const void* font);

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

#endif // _HUD_H