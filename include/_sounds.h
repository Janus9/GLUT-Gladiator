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

        // Play a looped track, fading in from silence to avoid the startup spike
        void playSounds(const std::string& fileName);

        // Ramps currentMusic volume from 0 to targetVolume over fadeDuration seconds.
        // Call once per frame from _scene::updateScene with dt in seconds.
        void updateFadeIn(double dt);
    protected:
    private:
        ISoundEngine* engine = nullptr;
        ISound* currentMusic = nullptr;
        float targetVolume = 1.0f;
        float fadeDuration = 0.75f;
        float fadeElapsed = 0.0f;
        bool  fading = false;
};

#endif
