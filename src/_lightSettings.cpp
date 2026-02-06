#include "_lightSettings.h"

/*
- Two colors can be mixed together through the RGB structure
- Red and Green make yellow
- Red and Blue makes Magenta
- Green and Blue make Cyan
- White is a result of all colors placed together
- Printers don't have additive colors, rather, they are made by subtracting colors from others
- There is no "white" in a printer necessarily, and nothing is printed to match this color scheme
- Values are stored in a register to process specific handling of information

- 32 bits pair to into 4 8-bit segments to match to specific color scheme
- The amount of bits depend on resolution of how many 32 bit pixels appear
- 1082x1920 is the amount of bits to define the entire screen for a given laptop
- Values stored into 32-bit values is the equivalent of a lookup-table for specific color values

- Base color: color of standard object
- Ambient color: color of environment around
- Shading: 3D color to object (base + ambient) then added with diffuse to register proper values
- Specular: Light intended to reflect glare adding more light to the given object

- Base color is the specific color of the object which factors into the
- Orientation of polygon dictates the specific shading information required
- More reflection angles provides more realism to the object

-


*/

_lightSettings::_lightSettings()
{
    //ctor
}

_lightSettings::~_lightSettings()
{
    //dtor
}

void _lightSettings::setLight(GLenum light)
{
    glLightfv(light, GL_AMBIENT, light_ambient); //establishes ambient light as part of GL_AMBIENT call for particular setting
    glLightfv(light, GL_DIFFUSE, light_diffuse); //establishes diffuse light as part of GL_DIFFUSE call for particular setting
    glLightfv(light, GL_SPECULAR, light_specular); //establishes specular light as part of GL_SPECULAR call for particular setting
    glLightfv(light, GL_POSITION, light_position); //establishes position of the light as part of GL_POSITION call for particular setting

    //Sets-up the material light textures
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);


}

