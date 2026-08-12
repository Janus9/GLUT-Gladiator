#ifndef SOUND_ENGINE_H
#define SOUND_ENGINE_H

#include <_common.h>
#include <SDL3/SDL.h>

namespace sound {
    class Engine {
        public:
            Engine();
            virtual ~Engine();
            /** 
             * Initializes the Sound Engine. Must be ran prior to engine usage. 
             * 
             * @return True if Sound Engine was initialized successfully. 
             * */
            bool init();

            /** Shuts down the engine. */
            void shutdown();

            /** 
             * Update loop for the sound engine. 
             * */
            void update(double dt);

            /** 
             * Registers a sound by ID for a given path. ID collisions will not apply and will return False. 
             * 
             * Only works for WAV files. 
             * 
             * @param id Unique ID for the sound.
             * @param filePath Location of the sound file.
             * @return True if sound was successfully registered.
             */
            bool registerSound(const std::string &id, const std::string &filePath);

            /**
             * Removes a sound from the registry by unique ID. Will fail on unregistered IDs.
             * 
             * @param id Unique ID of the sound.
             * @return True if sound was successfully removed. 
             */
            bool unRegisterSound(const std::string &id);

            /**
             * Plays a non-looped mono sound from a given ID. Audio can stack.
             */
            void playSound(const std::string &id);
            
            /**
             * Plays a non-looped sound spatial sound from a given ID. Audio can stack.
             * 
             * @param id Unique ID of the sound.
             * @param pos Position of the sound.
             */
            void playSound(const std::string &id, const Vec2f &pos);

            /**
             * Plays a looped sound from a given ID. Intended for music. Must be unloaded to stop playing (stopBackgroundSound). 
             * Different sound IDs can be stacked. If sound is already playing, then function does nothing.
             * 
             * @param id Unique ID of the sound.
             */
            void playBackgroundSound(const std::string &id);

            /**
             * Unloads a given background sound (looped) to stop it from playing from a given ID.
             * 
             * @param id Unique ID of the sound.
             */
            void stopBackgroundSound(const std::string &id);

            /**
             * Sets the soundtrack to the given ID.
             * The soundtrack cannot be stacked, or repeated.  
             * If the soundtrack is already set, this overwrites the soundtrack over the given fade time. 
             * 
             * @param id Unique ID of the sound.
             * @param fadeTime Time it takes to fade to next track (Default: 0.0s)
             */
            void setSoundTrack(const std::string id, float fadeTime = 0.0f);

            /**
             * Stops playing and removes the current soundtrack over the given fade time.
             * 
             * @param fadeTime Time it takes to fade out of the soundtrack (Default 0.0s)
             */
            void stopSoundTrack(float fadeTime = 0.0f);

            /**
             * Unloads all background sounds currently active.
             */
            void stopAllBackgroundSounds();

            /**
             * Sets the listener position for spatial audio.
             * 
             * @param pos Position of the listener.
             */
            void setListenerPosition(const Vec2f &pos);

            /**
             * Sets master volume for all sound outputs. 
             * 
             * @param volume [0.0 - 1.0]
             */
            void setMasterVolume(float volume);
            
            /** Returns the current master volume level [0.0-1.0] */
            float getMasterVolume() const;
        protected:
        private:
            struct Registration {
                SDL_AudioSpec spec = {};
                Uint8* data = nullptr;
                Uint32 dataSize = 0;
            };

            SDL_AudioDeviceID device = 0;

            std::unordered_map<std::string, Registration> registery;
            std::unordered_map<std::string, SDL_AudioStream*> backgroundStreams;
            std::vector<SDL_AudioStream*> activeStreams;

            std::pair<std::string, SDL_AudioStream*> activeSoundTrack;
            std::pair<std::string, SDL_AudioStream*> nextSoundTrack;
            float fadeTimeElapsed = 0.0f;
            float fadeTime = 0.0f;

            Vec2f listenerPosition = {0.0f, 0.0f};
            float spatialMaxDistance = 1000.0f;      // Max distance player can hear audio from
            float audioFalloff = 2.5f;               // What power is applied to audio falloff (ex 2.0f is squared falloff)

            float masterVolume = 1.0f;
            bool initialized = false;
    };
}

#endif // SOUND_ENGINE_H