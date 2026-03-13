#ifndef _HUDTEXT_H
#define _HUDTEXT_H

#include <_common.h>

class _hudText {
    public:
        _hudText();
        virtual ~_hudText();

        // Initialize a textbox of a given text, positinon, size
        void initText(const string &_text, const Vec2f &pos, const Vec2f &size);
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
};

#endif _HUDTEXT_H