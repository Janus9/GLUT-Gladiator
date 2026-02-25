#include <_quad.h>

_quad::_quad() 
{
    pos = {0.0f, 0.0f, 0.0f};
    rot = {0.0f, 0.0f, 0.0f};
    scale = {1.0f, 1.0f, 1.0f};
    color = {0.0f, 0.0f, 0.0f};
}

_quad::~_quad() {
    delete quadTexture;
    quadTexture = nullptr;
}

void _quad::initQuad(std::string fileName) 
{
    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), verticies, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &vboTex);
    glBindBuffer(GL_ARRAY_BUFFER, vboTex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoord), texCoord, GL_DYNAMIC_DRAW);
    
    quadTexture->loadTexture(fileName);
}

void _quad::drawQuad() 
{
    glPushMatrix();
        glColor3f(color.r, color.b, color.g);

        quadTexture->bindTexture();

        glTranslatef(pos.x, pos.y, pos.z);

        glScalef(scale.x, scale.y, scale.z);

        glRotatef(rot.x, 1,0,0);
        glRotatef(rot.y, 0,1,0);
        glRotatef(rot.z, 0,0,1);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glBindBuffer(GL_ARRAY_BUFFER, vboTex);
        glTexCoordPointer(2, GL_FLOAT, 0,(void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, vboPos);
        glVertexPointer(3, GL_FLOAT, 0,(void*)0);

        glDrawArrays(GL_QUADS,0,4);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glPopMatrix();
}

void _quad::updateQuad() 
{
    float texCoord[8] = {
        xMax, yMax, //  
        xMax, yMin, //
        xMin, yMin, //
        xMin, yMax  //
    };
    glBindBuffer(GL_ARRAY_BUFFER, vboTex);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(texCoord), texCoord);
}