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
    if (currentMusic) {
        currentMusic->drop();
        currentMusic = nullptr;
    }
    engine->drop();
}

void _sounds::playSounds(const std::string& fileName) {
    // Preload so first-play decode doesn't stall the audio thread
    engine->addSoundSourceFromFile(fileName.c_str(), ESM_AUTO_DETECT, true);

    // Release any prior music handle before replacing it
    if (currentMusic) {
        currentMusic->stop();
        currentMusic->drop();
        currentMusic = nullptr;
    }

    // Start looped, paused, and tracked so we get an ISound* back to fade in
    ISound* snd = engine->play2D(fileName.c_str(),
                                 true  /*looped*/,
                                 true  /*startPaused*/,
                                 true  /*track*/);
    if (!snd) {
        std::cerr << "Error playing " << fileName << "\n";
        return;
    }

    snd->setVolume(0.0f);
    snd->setIsPaused(false);

    currentMusic = snd;
    fadeElapsed = 0.0f;
    fading = true;
}

void _sounds::updateFadeIn(double dt) {
    if (!fading || !currentMusic) return;

    fadeElapsed += static_cast<float>(dt);
    float t = fadeElapsed / fadeDuration;
    if (t >= 1.0f) {
        t = 1.0f;
        fading = false;
    }
    currentMusic->setVolume(targetVolume * t);
}
