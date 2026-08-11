#include <SoundEngine.h>

namespace sound {
    // -- PUBLIC -- //
    Engine::Engine() {
        
    }

    Engine::~Engine() {
        shutdown();
    }

    bool Engine::init() {
        SDL_LogInfo(LOG_SOUND, "Initializing the Sound Engine.");
        if (initialized) {
            SDL_LogInfo(LOG_SOUND, "Sound Engine already initialized, skipping.");
            return false;
        }

        device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
        if (device == 0) {
            SDL_LogError(LOG_SOUND, "ERROR: Unable to open audio device: %s", SDL_GetError());
            return false;
        }

        SDL_LogInfo(LOG_SOUND, "Successfully initialized the Sound Engine.");
        
        initialized = true;

        return true;
    }

    void Engine::shutdown() {
        SDL_LogInfo(LOG_SOUND, "Shutting down the Sound Engine");

        if (!initialized) {
            SDL_LogWarn(LOG_SOUND, "Sound engine never initialized, skipping shut down.");
            return;
        }

        // Destroy all currently playing sounds.
        for (SDL_AudioStream *stream : activeStreams) {

            if (stream != nullptr) {
                SDL_DestroyAudioStream(stream);
            }
        }

        activeStreams.clear();


        // Free registered sound data.
        for (auto &entry : registery) {

            Registration &sound = entry.second;

            if (sound.data != nullptr) {
                SDL_free(sound.data);
                sound.data = nullptr;
            }
        }

        registery.clear();

        // Close playback device.
        if (device != 0) {
            SDL_CloseAudioDevice(device);
            device = 0;
        }

        initialized = false;

        SDL_LogInfo(LOG_SOUND, "Successfully shut down the Sound Engine.");
    }

    void Engine::update() {
        for (auto it = activeStreams.begin(); it != activeStreams.end();) {

            SDL_AudioStream *stream = *it;

            int queued = SDL_GetAudioStreamQueued(stream);
            int available = SDL_GetAudioStreamAvailable(stream);

            if (queued == 0 && available == 0) {
                SDL_DestroyAudioStream(stream);
                it = activeStreams.erase(it);
            } else {
                ++it;
            }
        }
    }

    bool Engine::registerSound(const std::string &id, const std::string &filePath) {
        if (registery.contains(id)) {
            SDL_LogWarn(LOG_SOUND, "Sound ID: %s is already registered!", id.c_str());
            return false;
        }

        Registration sound;
        if (!SDL_LoadWAV(filePath.c_str(), &sound.spec, &sound.data, &sound.dataSize)) {
            SDL_LogError(LOG_SOUND, "ERROR: Unable to load sound: %s, reason: %s", filePath.c_str(), SDL_GetError());
            return false;
        }

        registery[id] = sound;

        SDL_LogDebug(
            LOG_SOUND, 
            "Registered Sound"
            "\n - ID: %s"
            "\n - File Path: %s",
            id.c_str(),
            filePath.c_str()
        );

        return true;
    }

    bool Engine::unRegisterSound(const std::string &id) {
        auto it = registery.find(id);
        if (it == registery.end()) {
            SDL_LogError(LOG_SOUND, "ERROR: Unable to remove sound ID: %s as it is not in the registery.", id.c_str());
            return false;
        }

        if (it->second.data) {
            SDL_free(it->second.data);
        }

        registery.erase(it);
        SDL_LogInfo(LOG_PICKUPS, "Removed registered sound of ID: %s", id.c_str());
        return true;
    }

    void Engine::playSound(const std::string &id) {
        if (!initialized) {
            SDL_LogError(LOG_SOUND, "ERROR: Unable to play sound ID: %s as the engine is not initialized.", id.c_str());
            return;
        }

        auto it = registery.find(id);
        if (it == registery.end()) {
            SDL_LogWarn(LOG_SOUND, "WARNING: Unable to play sound ID: %s as it was not in the registery.", id.c_str());
            return;
        }

        Registration &sound = it->second;

        SDL_AudioStream *stream = SDL_CreateAudioStream(&sound.spec, nullptr);
        if (!stream) {
            SDL_LogError(LOG_SOUND, "ERROR: Failed to create audio stream for sound ID: %s, reason: %s", id.c_str(), SDL_GetError());
            return;    
        }
        
        if (!SDL_BindAudioStream(device, stream)) {
            SDL_LogError(
                LOG_SOUND,
                "Failed to bind audio stream for sound ID: %s, reason: %s",
                id.c_str(),
                SDL_GetError()
            );

            SDL_DestroyAudioStream(stream);
            return;
        }

        if (!SDL_PutAudioStreamData(stream, sound.data, static_cast<int>(sound.dataSize))) {
            SDL_LogError(
                LOG_SOUND,
                "Failed to queue audio data for sound ID: %s, reason %s",
                id.c_str(),
                SDL_GetError()
            );

            SDL_DestroyAudioStream(stream);
            return;
        }


        SDL_FlushAudioStream(stream);

        activeStreams.push_back(stream);

        return;
    }

    void Engine::setMasterVolume(float volume) {
        masterVolume = std::clamp(volume, 0.0f, 1.0f);
    }

    float Engine::getMasterVolume() const {
        return masterVolume;
    }

    // -- PRIVATE -- //
}