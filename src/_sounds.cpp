#include <_sounds.h>

_sounds::_sounds()
{
    engine = createIrrKlangDevice();
    if (!engine) {
        std::cerr << "Error occured starting the engine\n";
    }
}

_sounds::~_sounds()
{
    if (currentMusic) {
        currentMusic->drop();
        currentMusic = nullptr;
    }
    if (engine) engine->drop();
}

void _sounds::playBackgroundMusic(const std::string& fileName) {
    // Preload so first-play decode doesn't stall the audio thread
    engine->addSoundSourceFromFile(fileName.c_str(), ESM_AUTO_DETECT, true);

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

void _sounds::registerSfx(const std::string& name, const std::string& fileName, float volume) {
    ISoundSource* src = engine->addSoundSourceFromFile(fileName.c_str(), ESM_AUTO_DETECT, true);
    if (!src) {
        std::cerr << "registerSfx: failed to load " << fileName << " for '" << name << "'\n";
        return;
    }
    src->setDefaultVolume(volume * sfxMasterVolume);
    sfxRegistry[name] = SfxEntry{ fileName, volume };
}

void _sounds::playSfx(const std::string& name) {
    auto it = sfxRegistry.find(name);
    if (it == sfxRegistry.end()) {
        std::cerr << "playSfx: unknown SFX '" << name << "'\n";
        return;
    }
    // Fire-and-forget: not looped, not paused, not tracked. Uses the source's default volume.
    engine->play2D(it->second.path.c_str(),
                   false /*looped*/,
                   false /*startPaused*/,
                   false /*track*/);
}

void _sounds::setSfxMasterVolume(float v) {
    sfxMasterVolume = v;
    // Re-apply to all registered sources so the change takes effect on future plays.
    for (auto& kv : sfxRegistry) {
        ISoundSource* src = engine->getSoundSource(kv.second.path.c_str(), false);
        if (src) src->setDefaultVolume(kv.second.volume * sfxMasterVolume);
    }
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
