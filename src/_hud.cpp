#include <_hud.h>

// -- HUD ELEMENT -- //
// ----------------- //

_hudElement::_hudElement() {
    // ctor
}

_hudElement::~_hudElement() {
    // ctor
}

void _hudElement::draw() {
    if (!visible || ocapacity == 0.0f) {
        return;
    }
    for (auto child : children) {
        if (child) {
            glColor4f(color.r,color.g,color.b,ocapacity); // Apply color to each
            child->draw();
        }
    }
}

void _hudElement::addChild(_hudElement* child) {
    if (!child) return;
    child->parent = this;
    children.push_back(child);
}


bool _hudElement::removeChild(_hudElement* child) {
    if (!child) return;
    for (int i = 0; i < children.size(); i++) {
        if (children[i] == child) {
            children.erase(children.begin()+i);
            return true;
        }
    }
    return false; // Not found
}

_hudElement* _hudElement::getParent() const {
    return parent;
}

_hudElement* _hudElement::getChild(int index) const {
    if (index >= children.size() || index < 0) {
        // Index out of range
        return nullptr;
    }
    return children[index];
}

int _hudElement::getNumChildren() const {
    return children.size();
}

void _hudElement::setHudViewportDimensions(double width, double height) {
    Wwidth = width;
    Wheight = height;
}

// -- HUD TEXT -- //
// -------------- //

_hudText::_hudText() {
    // ctor
}

_hudText::~_hudText() {
    // dtor
}

void _hudText::setFont(const void* font) {

}

void _hudText::setText(const string &_text) {
    text = _text;
    
    size.x = getTextWidth();  // Set the width
    size.y = getTextHeight(); // Set the height
}

int _hudText::getTextWidth() const {
    int width = 0;

    for (int i = 0; i < text.size(); i++)
    {
        width += glutBitmapWidth(font, text[i]);
    }

    return width;
}

int _hudText::getTextHeight() const {
    // glut fonts roughly define their width in pixels from the type
    if (font == GLUT_BITMAP_8_BY_13)
    {
        return 13;
    }
    else if (font == GLUT_BITMAP_9_BY_15)
    {
        return 15;
    }
    else if (font == GLUT_BITMAP_TIMES_ROMAN_10)
    {
        return 10;
    }
    else if (font == GLUT_BITMAP_TIMES_ROMAN_24)
    {
        return 24;
    }
    else if (font == GLUT_BITMAP_HELVETICA_10)
    {
        return 10;
    }
    else if (font == GLUT_BITMAP_HELVETICA_12)
    {
        return 12;
    }
    else if (font == GLUT_BITMAP_HELVETICA_18)
    {
        return 18;
    }

    return 0;
}

const string& _hudText::getText() const {
    return text;
}

void _hudText::draw() {
    
    glRasterPos2f(position.x, position.y);

    for (int i = 0; i < (int)text.size(); i++) {
        glutBitmapCharacter(font, text[i]);
    }
}
