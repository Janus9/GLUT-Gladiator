#include <_hudText.h>

_hudText::_hudText() {
    // ctor
}

_hudText::~_hudText() {
    // dtor
}

void _hudText::initText(const string &_text, const Vec2f &pos, const Vec2f &size) {
    text = _text;
    pos_pixels = pos;
    size_pixels = size;
}

void _hudText::drawText() {
    if (text != "") {
        glPushMatrix();
            
        glPopMatrix();
    }
}
