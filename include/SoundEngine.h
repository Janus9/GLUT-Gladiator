#ifndef SOUND_ENGINE_H
#define SOUND_ENGINE_H

#include <_common.h>
#include <SDL3/SDL.h>

#define TOML_HEADER_ONLY 1
#include <toml.hpp>

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
             * Sets up sound registrations. Safe to call while program is active for runtime changes to sounds.
             * 
             * Automatically ran by init to setup initial sounds.   
             * */
            void reload();

            /** 
             * Update loop for the sound engine. 
             * */
            void update(double dt);

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
             * Plays a non-looped spatial sound from a given ID. Audio can stack.
             * The spatial sound cannot move. This should only be used for rapid or short audio clips. 
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
             * 
             * If the soundtrack is already set, the next soundtrack is set and fades in over the given fade time. 
             * The active soundtrack fades out over the given fade time.
             * If the next soundtrack is already set, function overwrites the existing next soundtrack.
             * If the provided sound ID is the same as the active soundtracks, it makes the active soundtrack fade back in.
             * 
             * This function is not protected from repeated calls, to prevent odd clipping, use "isPlayingSoundTrack" to ensure you are
             * not calling function repeatedly on the same soundtrack.
             * 
             * @param id Unique ID of the sound.
             * @param fadeTime Time it takes to fade to next track (Default: 0.0s)
             */
            void setSoundTrack(const std::string &id, float fadeTime = 0.0f);

            /**
             * Stops playing and removes the current soundtrack over the given fade time.
             * 
             * UNFINISHED
             * 
             * @param fadeTime Time it takes to fade out of the soundtrack (Default 0.0s)
             */
            void stopSoundTrack(float fadeTime = 0.0f);

            /**
             * @param id Unique ID of the sound.
             * @return True if track is currently active (not next); False if track is not currently active.
             */
            bool isPlayingSoundTrack(const std::string &id) const;

            /**
             * Unloads all background sounds currently active.
             */
            void stopAllBackgroundSounds();

            /**
             * Plays a looped spatial sound from a given ID. Audio can stack.
             * The looped spatial sound can move. This should be used for repeated audio effects (walking, gun reving, etc).
             * The looped spatial sound can be pause via `pauseSpatialLooped`.
             * 
             * The instanceID must be unique only for a given sound id. Meaning there can be repeats if the sound id is different.
             * 
             * The looped sound must be manually cleaned via `stopSoundLooped`.
             * 
             * @param id Unique ID of the sound.
             * @param instanceId Unique ID for this specific sound instance. Can be a unit/enemy ID, effect ID, etc.
             * @param pos Position of the spatial sound.
             */
            void createSpatialLooped(const std::string &id, int instanceId, const Vec2f &pos);

            /**
             * Resumes playback of a looped spatial sound from a given ID. 
             * 
             * If audio is already playing, then command is skipped. 
             */
            void resumeSpatialLooped(const std::string &id, int instanceId);

            /**
             * Updates a given looped spatial sound to modify it's position. This allows for moving spatial audio.
             * 
             * @param id Unique ID of the sound.
             * @param instanceId Unique ID for this specific sound instance.
             * @param pos Position of the spatial sound.
             */
            void updateSpatialLooped(const std::string &id, int instanceId, const Vec2f &pos);

            /**
             * Pauses a given looped spatial sound, but does not remove it from the playback list. 
             * 
             * The sound will be resumed by `playSpatialLooped`. `updateSpatialLooped` will not resume playback.
             * 
             * This does not clean up a sound, use `stopSpatialLooped` upon cleanup (enemy dead, etc).
             * 
             * @param id Unique ID of the sound.
             * @param instanceId Unique ID for this specific sound instance.
             */
            void pauseSpatialLooped(const std::string &id, int instanceId);

            /**
             * Pauses all the looped spatial sounds. 
             * This does not clean them like `stopAllSpatialLooped` does, but it mean they do not have to be re-created. 
             */
            void pauseAllSpatialLooped();

            /**
             * Stops playing a looped spatial sound. 
             * 
             * @param id Unique ID of the sound.
             * @param instanceId Unique ID for this specific sound instance.
             */
            void stopSpatialLooped(const std::string &id, int instanceId);

            /**
             * Stops all of the looped spatial sounds and cleans up.
             */
            void stopAllSpatialLooped();

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
            struct Config {
                std::string id;
                std::string filePath;
                float gain;
            };

            struct Registration {
                float gain;

                // -- Regular -- //
                SDL_AudioSpec spec = {};
                Uint8* data = nullptr;
                Uint32 dataSize = 0;

                // -- Spatial Audio -- //
                SDL_AudioSpec spatialSpec = {};
                Uint8* spatialData = nullptr;
                Uint32 spatialDataSize = 0;
            };

            // -- Spatial Audio -- //
            struct SpatialLoop {
                std::string soundId;
                int instanceId = 0;

                const Registration* registration = nullptr;
                
                SDL_AudioStream* stream = nullptr;

                size_t sampleOffset = 0;

                Vec2f position = {0.0f, 0.0f};

                bool playing = true;

                float leftGain = 1.0f;
                float rightGain = 1.0f;
                float distanceGain = 1.0f;
            };

            struct SpatialLoopKey {
                std::string soundId;
                int instanceId;

                bool operator==(const SpatialLoopKey &other) const {
                    return soundId == other.soundId && instanceId == other.instanceId;
                }
            };

            struct SpatialLoopHash {
                size_t operator()(const SpatialLoopKey &key) const {
                    size_t h1 = std::hash<std::string>{}(key.soundId);
                    size_t h2 = std::hash<int>{}(key.instanceId);

                    return h1 ^ (h2 << 1);
                }
            };

            std::unordered_map<SpatialLoopKey, size_t, SpatialLoopHash> spatialLoopMap; // Holds indicies to the Spatial Loops in memory
            std::vector<SpatialLoop> spatialLoopList; // List of Spatial Loops in memory 

            // Device for playback
            SDL_AudioDeviceID device = 0;
            // Audio registration data
            std::unordered_map<std::string, Registration> registery;
            // Audio streams with automatic looping. Must be manually cleared.
            std::unordered_map<std::string, SDL_AudioStream*> backgroundStreams;
            // Audio streams with no looping and autmatic cleanup.
            std::vector<SDL_AudioStream*> activeStreams;
            // The current active soundtrack
            std::pair<std::string, SDL_AudioStream*> activeSoundTrack;
            // The next sound track that fades in over time
            std::pair<std::string, SDL_AudioStream*> nextSoundTrack;

            float fadeTimeElapsed = 0.0f;
            float fadeTime = 0.0f;

            Vec2f listenerPosition = {0.0f, 0.0f};   // Where listener (player) is located
            float spatialMaxDistance = 1000.0f;      // Max distance player can hear audio from
            float audioFalloff = 2.5f;               // What power is applied to audio falloff (ex 2.0f is squared falloff)

            float masterVolume = 1.0f;
            bool initialized = false;

            bool registerSound(const Config &config);
            void updateSpatialGains(SpatialLoop &audio);   // Sets panning & attenuation based on distance
            void queueSpatialChunk(SpatialLoop &audio);    // Assigns chunk of playback to the stream each update
    };
}

#endif // SOUND_ENGINE_H