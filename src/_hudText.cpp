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

void _hudText::setText(const string &_text) {
    text = _text;
}

void _hudText::drawText() {
    if (text.empty()) return;   // Nothing to draw if no text

    // Projection affects our "camera" mainly things like perspection and orthographic
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); // We push a NEW matrix onto the stack, if we dont do this were gonna mess up glOrtho in the _scene
    glLoadIdentity(); // Resets the model view removing the existing GL_PROJECTION modifications (mainly from _scene)

    // glOrtho does NOT replace the matrix, it multiples onto what already exists. Since glLoadIdentity is called above, were starting with a fresh GL_PROJECTION
    glOrtho(0.0, (double)screenWidth, 0.0, (double)screenHeight, -1.0, 1.0);

    // Model view is world transformations like rotate/translate etc
    glMatrixMode(GL_MODELVIEW);
    // We follow the same process as above, make a new matrix so our model translations etc dont bleed
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_LIGHTING);     // Lighting will hard text
    glDisable(GL_TEXTURE_2D);   // Prevent loading textures onto text
    glDisable(GL_DEPTH_TEST);   // Text ignores depth_test to ensure it draws on TOP and is never hidden

    glColor3f(color.r, color.g, color.b);

    glRasterPos2f(pos_pixels.x, pos_pixels.y);

    int i = 0;
    for (i = 0; i < (int)text.size(); i++) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, text[i]);
    }

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
}

void _hudText::setScreenDimensions(double width, double height) {
    screenWidth = width;
    screenHeight = height;
}