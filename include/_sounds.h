#ifndef _SOUNDS_H
#define _SOUNDS_H

#include <_common.h>
#include <SNDS/irrKlang.h>
#include <unordered_map>

using namespace irrklang;

class _sounds
{
    public:
        _sounds();
        virtual ~_sounds();

        // Plays a looped background track, fading in from silence. Replaces any prior music.
        void playBackgroundMusic(const std::string& fileName);

        // Preloads a sound effect and stores it under `name`. Later looked up by playSfx.
        // `volume` is the per-SFX gain (0.0 - 1.0) applied when played.
        void registerSfx(const std::string& name, const std::string& fileName, float volume = 1.0f);

        // Fires a one-shot SFX previously registered under `name`. Does not affect background music.
        // Safe to call rapidly; overlapping plays are mixed by irrKlang.
        void playSfx(const std::string& name);

        // Master SFX volume multiplier (0.0 - 1.0). Applied to registered SFX at register-time.
        void setSfxMasterVolume(float v);

        // Ramps currentMusic volume from 0 to targetVolume over fadeDuration seconds.
        // Call once per frame with dt in seconds.
        void updateFadeIn(double dt);
    protected:
    private:
        struct SfxEntry {
            std::string path;
            float volume = 1.0f;
        };

        ISoundEngine* engine = nullptr;

        // Music state
        ISound* currentMusic = nullptr;
        float targetVolume = 1.0f;
        float fadeDuration = 0.75f;
        float fadeElapsed = 0.0f;
        bool  fading = false;

        // SFX registry
        std::unordered_map<std::string, SfxEntry> sfxRegistry;
        float sfxMasterVolume = 1.0f;
};

#endif
