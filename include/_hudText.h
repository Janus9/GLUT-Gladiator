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

#ifndef _HUDTEXT_H
#define _HUDTEXT_H

#include <_common.h>

class _hudText {
    public:
        _hudText();
        virtual ~_hudText();

        // Initialize a textbox of a given text, positinon, size
        void initText(const string &_text, const Vec2f &pos, const Vec2f &size);
        void setScreenDimensions(double width, double height);

        void drawText();

        // Position (in pixels)
        Vec2f pos_pixels = {0.0f, 0.0f};
        // Position in % relative to top-left of viewport
        Vec2f pos_percentage = {0.0f, 0.0f};
        // Size of the textbox in pixels
        Vec2f size_pixels = {0.0f, 0.0f};
        // Size of the textbox as a % of the viewport x/y
        Vec2f size_percentage = {0.0f, 0.0f};

        Col3f color = {0.0f, 0.0f, 0.0f}; // Default to black

        string text = ""; 

        bool backgroundEnabled = false; // If true draws a background behind text
        bool borderEnabled = false;     // If true draws a border behind text
    protected:
    private:
        double screenWidth = 0.0;
        double screenHeight = 0.0;
};

#endif // _HUDTEXT_H