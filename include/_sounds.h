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

        // Plays a looped background track, fading in from silence to `volume` (0.0 - 1.0). Replaces any prior music.
        void playBackgroundMusic(const std::string& fileName, float volume = 1.0f);

        // Preloads a sound effect and stores it under `name`. Later looked up by playSfx.
        // `volume` is the per-SFX gain (0.0 - 1.0) applied when played.
        void registerSfx(const std::string& name, const std::string& fileName, float volume = 1.0f);

        // Fires a one-shot SFX previously registered under `name`. Does not affect background music.
        // Safe to call rapidly; overlapping plays are mixed by irrKlang.
        void playSfx(const std::string& name);

        /**
         * Fires a one-shot 3D SFX previously registered under `name`. Does not affect background music.
         * 3D Audio allows for panning, attenuation, and directionality (z axis is ignored since game is 2D)
         * Safe to call rapidly; overlapping plays are mixed by irrKlang.
         * 
         * @param name Name of sound to load
         * @param pos Position to play sound at
         */
        void playSfx3D(const std::string& name, const Vec2f& pos);

        /**
         * Adds a SFX to the "looped" registery, these looped sounds play looped until "removeSfx3DLooped" is called.
         * 
         * Once an ID is registered, it cannot be changed or edited, only removed. This means its safe to call this in a loop
         * as after the 1st call, it does nothing
         * 
         * @param name Name of the sound to play
         * @param ID Unique ID tied to the sound (ex/ unit ID)
         * @param pos Position to play the sound at
         */
        void playSfx3DLooped(const std::string &name, int ID, const Vec2f& pos);

        /**
         * Removes a registered loop SFX to stop playing
         * 
         * @param ID Unique ID tied to the sound
         */
        void removeSfx3DLooped(int ID);

        // Sets the listener position for playSfx3D
        void setListenerPos(const Vec2f& pos);

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

        std::unordered_map<int, ISound*> loopedSfxActive;

        // 3D Audio Listener Position
        Vec2f listenerPos = {0.0f, 0.0f};
};

#endif
