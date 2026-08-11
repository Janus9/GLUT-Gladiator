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

        // One-shot streams
        for (SDL_AudioStream *stream : activeStreams) {
            if (stream != nullptr) {
                SDL_DestroyAudioStream(stream);
            }
        }

        activeStreams.clear();

        // Background streams
        stopAllBackgroundSounds();

        // Registered audio data
        for (auto &entry : registery) {
            Registration &sound = entry.second;

            if (sound.data != nullptr) {
                SDL_free(sound.data);
                sound.data = nullptr;
            }
        }

        registery.clear();


        if (device != 0) {
            SDL_CloseAudioDevice(device);
            device = 0;
        }

        initialized = false;

        SDL_LogInfo(LOG_SOUND, "Successfully shut down the Sound Engine.");
    }

    void Engine::update() {
        // ----- One-shot sounds ----- //
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

        // ----- Background sounds ----- //
        for (auto &entry : backgroundStreams) {

            const std::string &id = entry.first;
            SDL_AudioStream *stream = entry.second;

            auto soundIt = registery.find(id);

            if (soundIt == registery.end()) {
                continue;
            }

            Registration &sound = soundIt->second;

            int queued = SDL_GetAudioStreamQueued(stream);

            if (queued <= static_cast<int>(sound.dataSize)) {

                if (!SDL_PutAudioStreamData(
                    stream,
                    sound.data,
                    static_cast<int>(sound.dataSize)
                )) {
                    SDL_LogError(
                        LOG_SOUND,
                        "Failed to loop background sound '%s': %s",
                        id.c_str(),
                        SDL_GetError()
                    );
                }
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

    void Engine::playSound(const std::string &id, const Vec2f &pos) {
        // Courtesy of mr.gpt
        if (!initialized) {
            return;
        }

        auto it = registery.find(id);

        if (it == registery.end()) {
            SDL_LogWarn(
                LOG_SOUND,
                "Sound ID not registered: %s",
                id.c_str()
            );

            return;
        }

        Registration &sound = it->second;


        // ---------------------------------
        // Calculate positional information
        // ---------------------------------

        const Vec2f offset = pos - listenerPosition;

        const float distance = offset.length();

        // -1.0 = left
        //  0.0 = center
        // +1.0 = right

        float pan = offset.x / spatialMaxDistance;

        pan = std::clamp(
            pan,
            -1.0f,
            1.0f
        );

        // Distance attenuation
        float distanceGain =
            1.0f - (distance / spatialMaxDistance);

        distanceGain = std::clamp(
            distanceGain,
            0.0f,
            1.0f
        );

        // Audio falloff
        distanceGain = powf(distanceGain, audioFalloff);

        // ---------------------------------
        // Calculate stereo gains
        // ---------------------------------

        float leftGain = 1.0f;
        float rightGain = 1.0f;

        if (pan < 0.0f) {
            rightGain = 1.0f + pan;
        }
        else {
            leftGain = 1.0f - pan;
        }

        leftGain *= distanceGain;
        rightGain *= distanceGain;


        // ---------------------------------
        // Convert source audio to stereo F32
        // ---------------------------------

        SDL_AudioSpec stereoSpec = {};

        stereoSpec.format = SDL_AUDIO_F32;
        stereoSpec.channels = 2;
        stereoSpec.freq = sound.spec.freq;


        Uint8 *convertedData = nullptr;
        int convertedSize = 0;

        if (!SDL_ConvertAudioSamples(
            &sound.spec,
            sound.data,
            static_cast<int>(sound.dataSize),
            &stereoSpec,
            &convertedData,
            &convertedSize
        )) {
            SDL_LogError(
                LOG_SOUND,
                "Unable to convert sound '%s': %s",
                id.c_str(),
                SDL_GetError()
            );

            return;
        }

        // ---------------------------------
        // Apply stereo panning
        // ---------------------------------

        float *samples =
            reinterpret_cast<float*>(convertedData);

        const int sampleCount =
            convertedSize / sizeof(float);


        for (int i = 0; i < sampleCount; i += 2) {
            samples[i] *= leftGain;
            samples[i + 1] *= rightGain;
        }


        // ---------------------------------
        // Create playback stream
        // ---------------------------------

        SDL_AudioStream *stream =
            SDL_CreateAudioStream(
                &stereoSpec,
                nullptr
            );

        if (stream == nullptr) {
            SDL_free(convertedData);
            return;
        }

        if (!SDL_BindAudioStream(
            device,
            stream
        )) {

            SDL_DestroyAudioStream(stream);
            SDL_free(convertedData);

            return;
        }

        if (!SDL_PutAudioStreamData(
            stream,
            convertedData,
            convertedSize
        )) {

            SDL_DestroyAudioStream(stream);
            SDL_free(convertedData);

            return;
        }

        SDL_FlushAudioStream(stream);

        SDL_free(convertedData);

        activeStreams.push_back(stream);

        return;
    }

    void Engine::playBackgroundSound(const std::string &id) {
        if (!initialized) {
            return;
        }

        // Already playing
        if (backgroundStreams.find(id) != backgroundStreams.end()) {
            return;
        }

        // Find registered sound
        auto soundIt = registery.find(id);
        if (soundIt == registery.end()) {
            SDL_LogWarn(
                LOG_SOUND,
                "Background sound ID not registered: %s",
                id.c_str()
            );

            return;
        }

        Registration &sound = soundIt->second;

        // Create stream
        SDL_AudioStream *stream = SDL_CreateAudioStream(
            &sound.spec,
            nullptr
        );

        if (!stream) {
            SDL_LogError(
                LOG_SOUND,
                "Failed to create background audio stream: %s",
                SDL_GetError()
            );
            return;
        }

        // Bind to playback device
        if (!SDL_BindAudioStream(device, stream)) {
            SDL_LogError(
                LOG_SOUND,
                "Failed to bind background audio stream: %s",
                SDL_GetError()
            );

            SDL_DestroyAudioStream(stream);

            return;
        }

        // Queue two copies to create a buffer ahead of playback.
        if (!SDL_PutAudioStreamData(
            stream,
            sound.data,
            static_cast<int>(sound.dataSize)
        )) {
            SDL_LogError(
                LOG_SOUND,
                "Failed to queue background audio: %s",
                SDL_GetError()
            );

            SDL_DestroyAudioStream(stream);
            return;
        }

        if (!SDL_PutAudioStreamData(
            stream,
            sound.data,
            static_cast<int>(sound.dataSize)
        )) {
            SDL_LogError(
                LOG_SOUND,
                "Failed to queue background audio: %s",
                SDL_GetError()
            );

            SDL_DestroyAudioStream(stream);
            return;
        }

        backgroundStreams[id] = stream;
    }

    void Engine::stopBackgroundSound(const std::string &id) {
        auto it = backgroundStreams.find(id);

        if (it == backgroundStreams.end()) {
            return;
        }

        SDL_DestroyAudioStream(it->second);

        backgroundStreams.erase(it);
    }

    void Engine::stopAllBackgroundSounds() {
        for (auto &entry : backgroundStreams) {
            SDL_AudioStream *stream = entry.second;

            if (stream != nullptr) {
                SDL_DestroyAudioStream(stream);
            }
        }

        backgroundStreams.clear();
    }

    void Engine::setListenerPosition(const Vec2f &pos) {
        listenerPosition = pos;
    }

    void Engine::setMasterVolume(float volume) {
        masterVolume = std::clamp(volume, 0.0f, 1.0f);
    }

    float Engine::getMasterVolume() const {
        return masterVolume;
    }

    // -- PRIVATE -- //
}