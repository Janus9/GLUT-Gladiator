#include <_vbo.h>

_vbo::_vbo()
{
    //ctor

    rot = {0.0f, 0.0f, 0.0f};
    pos = {2.0f, 0.0f, -8.0f};
    scale = {1.0f, 1.0f, 1.0f};
    color = {1.0f, 1.0f, 1.0f};
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
    glPushMatrix();
        glColor3f(color.r, color.g, color.b);

        glTranslatef(pos.x, pos.y, pos.z);

        glScalef(scale.x, scale.y, scale.z);
        
        glRotatef(rot.x, 1.0f, 0.0f, 0.0f);
        glRotatef(rot.y, 0.0f, 1.0f, 0.0f);
        glRotatef(rot.z, 0.0f, 0.0f, 1.0f);

        glEnableClientState(GL_VERTEX_ARRAY); // Enable buffer handling for verticies
        glEnableClientState(GL_NORMAL_ARRAY); // Enable buffer handling for normals

        glBindBuffer(GL_ARRAY_BUFFER, vboVertices); // Bind the vertex buffer
        glVertexPointer(3, GL_FLOAT, 0, 0);  // 3 for triangles, not quads
        glBindBuffer(GL_ARRAY_BUFFER, vboNormals); // Bind the normal buffer
        glNormalPointer(GL_FLOAT, 0, 0); // normals are always in sets of 3

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId); // Bind the face buffer
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0); // Draw the triangles using the faces as indexes into the verticies and normals

        glDisableClientState(GL_VERTEX_ARRAY); // Disable buffer handling for verticies
        glDisableClientState(GL_NORMAL_ARRAY); // Disable buffer handling for normals
    glPopMatrix();
}
