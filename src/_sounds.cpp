#include <_sounds.h>

_sounds::_sounds() 
{
    // ctor
    engine = createIrrKlangDevice();
    if (!engine) {
        std::cerr << "Error occured starting the engine\n";
    }
}

_sounds::~_sounds() 
{
    // dtor
    engine->drop();
}

void _sounds::playSounds(const std::string& fileName) {
    if(!engine->play2D(fileName.c_str(),true)) {
        std::cerr << "Error playing " << fileName << "\n";
    }
}