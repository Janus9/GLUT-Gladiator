#include <_vbo.h>

_vbo::_vbo()
{
    //ctor

    rot = {0.0f, 0.0f, 0.0f};
    pos = {2.0f, 0.0f, -8.0f};
    scale = {1.0f, 1.0f, 1.0f};
}

_vbo::~_vbo()
{
    //dtor
}

void _vbo::vboInit()
{
    // Verticies //
    glGenBuffers(1, &vboVertices);                                             // create the space
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);                                // bind the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // send the data, static (fixed size) draw means we wont be changing it
    // Normals //
    glGenBuffers(1, &vboNormals);                                              // create the space
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);                                 // bind the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);   // send the data, static (fixed size) draw means we wont be changing it
    // Faces //
    glGenBuffers(1, &eboId);                                                           // create the space
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId);                                      // bind the buffer. We use GL_ELEMENT_ARRAY_BUFFER because the faces INDEX into verticies and normals
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);   // send the data, static (fixed size) draw means we wont be changing it
}

void _vbo::drawVBO()
{
    // todo
    glPushMatrix();
    glPopMatrix();
}
