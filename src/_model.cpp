#include <_model.h>

_model::_model()
{
    //ctor
}

_model::~_model()
{
    //dtor
}

void _model::initModel() {
    rotation.x = rotation.y = rotation.z = 0.0f;
    scale.x = scale.y = scale.z = 1.0f;
    position.x = position.y = position.z = 0.0f;
    color.r = color.g = color.b = 1.0f;
}

void _model::drawModel() {
    glPushMatrix();
        // Setup model identity
        glColor3f(color.r, color.g, color.b);
        glTranslatef(position.x, position.y, position.z);
        glScalef(scale.x, scale.y, scale.z);

        glRotatef(rotation.x, 1.0f, 0.0f, 0.0f);
        glRotatef(rotation.y, 0.0f, 1.0f, 0.0f);
        glRotatef(rotation.z, 0.0f, 0.0f, 1.0f);

        glutSolidTeapot(1.0f); // ex

    glPopMatrix();
}