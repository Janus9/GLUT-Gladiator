#include <_parallax.h>

_parallax::_parallax()
{
    //ctor
}

_parallax::~_parallax()
{
    //dtor
    delete parallaxLoader;
    parallaxLoader = nullptr;
}

void _parallax::initParallax(const std::string &fileName)
{
    //todo
    xMax = 1.0f;
    yMax = 1.0f;

    xMin = 0.0f;
    yMin = 0.0f;

    if (!parallaxLoader->loadTexture(fileName)) {
        std::cerr << "Failed to load parallax texture: " << fileName << std::endl;

        // this would be better to throw an exception -- look into later
    }
}

void _parallax::scrollParallax(bool isHorizontal, int direction, float amount)
{
    //todo
}

void _parallax::drawParallax(float width, float height)
{
    // width / height are size of screen (in pixels)

    glColor3f(1.0f, 1.0f, 1.0f); 
    
    parallaxLoader->bindTexture();

    glDisable(GL_LIGHTING); // Disable lighting for parallax background
    
    glBegin(GL_POLYGON);
        glTexCoord2f(xMin ,yMax); glVertex3f(-width / height,-1,-30);
        glTexCoord2f(xMax ,yMax); glVertex3f(width / height,-1,-30);
        glTexCoord2f(xMax ,yMin); glVertex3f(width / height,-1,-30);
        glTexCoord2f(xMin ,yMin); glVertex3f(-width / height,-1,-30);
    glEnd();

    glEnable(GL_LIGHTING);
}