#include <_hud.h>
// -- HUD -- //
// --------- //

double _hud::Wwidth = 0.0;
double _hud::Wheight = 0.0;

_hud::_hud() {
    // ctor
}

_hud::~_hud() {
    // Clear memory list points too prior to clearing
    for (int i = 0; i < childrenList.size(); i++) {
        delete childrenList[i];
        childrenList[i] = nullptr;
    }

    childrenMap.clear();
    childrenList.clear();
}

void _hud::setHudViewportDimensions(double width, double height) {
    Wwidth = width;
    Wheight = height;
}

void _hud::addHudText(const string &name) {
    const string fullName = text_prefix + name;
    // Look for element in the hashmap first (have to for adjusting Vector) -- Look in map first for O(1) lookup. Not found means we dont run O(n) vector check
    auto it = childrenMap.find(fullName);
    it = childrenMap.find(fullName);
    if (it != childrenMap.end()) {
        // Found -- need to find inside vector
        bool found = false;
        for (int i = 0; i < childrenList.size(); i++) {
            if (childrenList[i] == it->second) {
                // The list holds a reference, we must delete before removing the reference from the vector or we get a leak
                delete childrenList[i];
                childrenList[i] = nullptr;
                childrenList.erase(childrenList.begin() + i);
                found = true;
                break;
            }
        }
        if (!found) {
            cerr << "WARNING: Element existed in childrenMap but not childrenList. They were desyncronized\n";
        }
    }
    // Regardless if it was found or not we allow overwrites so we create a new one
    _hudText* newSprite = new _hudText();
    childrenMap[fullName] = newSprite;  // Hashmap [] overwrites and adds new by default
    childrenList.push_back(newSprite); 
}

bool _hud::removeHudText(const string &name) {
    const string fullName = text_prefix + name;
    auto it = childrenMap.find(fullName);
    if (it == childrenMap.end()) {
        return false;
    }
    auto list_it = find(childrenList.begin(), childrenList.end(), it->second);
    if (list_it != childrenList.end()) {
        childrenList.erase(list_it);
    } else {
        cerr << "WARNING: Element existed in childrenMap but not childrenList. They were desyncronized\n";
    }

    delete it->second;
    childrenMap.erase(it);

    return true;
}

_hudText* _hud::getHudText(const string &name) const {
    const string fullName = text_prefix + name;
    auto it = childrenMap.find(fullName);
    if (it == childrenMap.end()) {
        return nullptr;
    }
    // Dynamic cast because were using virtual classes (polymorphism) thus we are unsure if this object found is what we expect.
    // Ex/ user sets "TEST" to be a hudText, then later does "TEST" for a hudSprite. Now type has changed.
    return dynamic_cast<_hudText*>(it->second);
}

int _hud::getNumChildren() const {
    return childrenList.size();
}

void _hud::drawHud() {

    glMatrixMode(GL_PROJECTION);    // Set matrix to projection as we need to apply a new orthographic projection for the hud
    
    glPushMatrix(); // Pushes the current matrix (projection) to the stack. This preserves the original projection matrix for later
    
        glLoadIdentity(); // Wipes clear the matrix by setting it to the identity matrix

        /**
         * (0,height) ----- (width,height)
         *  |                      |
         * (0,0) --------------(width,0)
         * 
         * Each world unit = 1 screen pixel
         */
        glOrtho(0.0, Wwidth, 0.0, Wheight,-1.0, 1.0);  

        glMatrixMode(GL_MODELVIEW); // Go back to model view to draw elements 
        
        glPushMatrix(); // Pushes the model view matrix to the stack to preserve it (this is other transforms like camera from scene etc)
            glLoadIdentity(); // Wipes clear the current matrix since hud elements need their own transformations

            for (auto &child : childrenList) {
                if (!child) {
                    continue;
                } 
                if (!child->visible || child->opacity == 0.0f)  {
                    continue; 
                }
                glColor4f(child->color.r,child->color.g,child->color.b,child->opacity);
                child->draw();
            }
        glPopMatrix();
        
        glMatrixMode(GL_PROJECTION); // Go to project matrix to pop it from the stack to not interfear with other ortho projections

    glPopMatrix(); 

    glMatrixMode(GL_MODELVIEW);  // Go back to model view for reset state 
    
}

// -- HUD ELEMENT -- //
// ----------------- //

_hudElement::_hudElement() {
    // ctor
}

_hudElement::~_hudElement() {
    // ctor
}

void _hudElement::draw() {
    // does nothing
}

// -- HUD TEXT -- //
// -------------- //

_hudText::_hudText() {
    // ctor
}

_hudText::~_hudText() {
    // dtor
}

void _hudText::setFont(void* _font) {
    font = _font;
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

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    
    glRasterPos2f(position.x, position.y);

    for (int i = 0; i < (int)text.size(); i++) {
        glutBitmapCharacter(font, text[i]);
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
}
