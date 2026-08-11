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
            void update();

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
             * TODO -- Plays a sound but non-looped and can stack.
             */
            void playSound(const std::string &id);

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
                Uint8 *data = nullptr;
                Uint32 dataSize = 0;
            };

            SDL_AudioDeviceID device = 0;

            std::unordered_map<std::string, Registration> registery;
            std::vector<SDL_AudioStream*> activeStreams;

            float masterVolume = 1.0f;
            bool initialized = false;
    };
}

#endif // SOUND_ENGINE_H