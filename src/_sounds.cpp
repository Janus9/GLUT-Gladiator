#include <_sounds.h>

_sounds::_sounds()
{
    engine = createIrrKlangDevice();
    if (!engine) {
        std::cerr << "Error occured starting the engine\n";
    }
    engine->setDefault3DSoundMinDistance(50.0f);    // Distance less than this plays at 100% volume
    engine->setDefault3DSoundMaxDistance(512.0f);   // Distance greater than this plays at 0% volume
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

void _sounds::playSfx3D(const std::string& name, const Vec2f& pos) {
    auto it = sfxRegistry.find(name);
    if (it == sfxRegistry.end()) {
        std::cerr << "playSfx: unknown SFX '" << name << "'\n";
        return;
    }
    engine->play3D(it->second.path.c_str(),vec3df(pos.x,pos.y,0.0f),false,false,false);
}

void _sounds::playSfx3DLooped(const std::string &name, int ID, const Vec2f& pos) {
    auto idIt = loopedSfxActive.find(ID);
    if (idIt != loopedSfxActive.end()) {
        // ID already registed, skip
        return;
    }

    auto it = sfxRegistry.find(name);
    if (it == sfxRegistry.end()) {
        std::cerr << "playSfx: unknown SFX '" << name << "'\n";
        return;
    }
    ISound* sound = engine->play3D(
        it->second.path.c_str(),
        vec3df(pos.x,pos.y,0.0f),
        true,   // looped
        false,  // start paused
        true    // track sound
    );
    if (!sound) {
        cout << "ERROR: Cannot play sfx: " << name << "\n";
        return;
    }
    loopedSfxActive[ID] = sound;
}

void _sounds::removeSfx3DLooped(int ID) {
    auto idIt = loopedSfxActive.find(ID);
    if (idIt == loopedSfxActive.end()) {
        // No sound active, skip
        return;
    }

    ISound* sound = idIt->second;
    if (sound) {
        sound->stop();
        sound->drop();
    }

    loopedSfxActive.erase(idIt);
}

void _sounds::setListenerPos(const Vec2f &pos) {
    listenerPos = pos;
    engine->setListenerPosition(vec3df(listenerPos.x,listenerPos.y,0.0f),
                                vec3df(0,0,1),  // Look direction (constant to forward)
                                vec3df(0,0,0),  // Velocity (not used so 0)
                                vec3df(0,1,0)); // Up vector (y is up)
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
