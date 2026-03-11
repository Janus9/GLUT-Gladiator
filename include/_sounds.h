#ifndef _SOUNDS_H
#define _SOUNDS_H

#include <_common.h>
#include <SNDS/irrKlang.h>

using namespace irrklang;

class _sounds 
{
    public:
        _sounds();
        virtual ~_sounds();

        // There is no init function, default constructor handles this
        
        // Test function to play a file looped
        void playSounds(const std::string& fileName);
    protected:
    private:
        ISoundEngine* engine = nullptr;
};

#endif