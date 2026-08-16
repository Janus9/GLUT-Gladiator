#include <SoundEngine.h>

namespace sound {
    // -- PUBLIC -- //
    Engine::Engine() {
        activeSoundTrack.first = "";
        activeSoundTrack.second = nullptr;

        nextSoundTrack.first = "";
        nextSoundTrack.second = nullptr;
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

        reload();

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

        // Sound Tracks //
        if (activeSoundTrack.second) SDL_DestroyAudioStream(activeSoundTrack.second);
        if (nextSoundTrack.second) SDL_DestroyAudioStream(nextSoundTrack.second);

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

    void Engine::reload() {
        auto start = std::chrono::steady_clock::now();

        SDL_LogInfo(LOG_SOUND, "Reloading sound engine");

        // -- Load Config -- //

        toml::table config;
        try {
            config = toml::parse_file("configs/sounds.toml");
        } catch (const toml::parse_error &err) {
            SDL_LogError(LOG_SOUND, "ERROR: Failed to parse the sounds: %s", err.what());
            return;
        }

        toml::array* sounds = config["sounds"].as_array();
        if (!sounds) {
            SDL_LogError(LOG_SOUND,"ERROR: Cannot parse sounds as sounds.toml is missing"); 
            return;
        }

        SDL_LogDebug(LOG_SOUND,"Read: %llu sounds from sounds.toml",sounds->size());

        // -- Clear Memory -- //

        // One-shot streams
        for (SDL_AudioStream *stream : activeStreams) {
            if (stream) SDL_DestroyAudioStream(stream);
        }
        activeStreams.clear();

        // Background streams
        stopAllBackgroundSounds();

        // Sound Tracks //
        if (activeSoundTrack.second) SDL_DestroyAudioStream(activeSoundTrack.second);
        if (nextSoundTrack.second) SDL_DestroyAudioStream(nextSoundTrack.second);
        activeSoundTrack.first = ""; activeSoundTrack.second = nullptr;
        nextSoundTrack.first = ""; nextSoundTrack.second = nullptr;
        
        // Registered audio data
        for (auto &entry : registery) {
            Registration &sound = entry.second;

            if (sound.data) {
                SDL_free(sound.data);
                sound.data = nullptr;
            }
        }
        registery.clear();

        // -- Load Audio -- //

        for (toml::node &node : *sounds) {
            toml::table* soundTable = node.as_table();

            if (!soundTable) {
                SDL_LogWarn(LOG_SOUND, "Skipping invalid sound entry");
                continue;
            }

            Config sound;

            sound.id = soundTable->get("id")->value_or("");
            sound.filePath = soundTable->get("file_path")->value_or("");
            sound.gain = std::clamp(soundTable->get("gain")->value_or(1.0f), 0.0f, 1.0f);

            if (sound.id.empty()) {
                SDL_LogWarn(LOG_SOUND, "Skipping sound with missing ID");
                continue;
            }

            if (sound.filePath.empty()) {
                SDL_LogWarn(
                    LOG_SOUND,
                    "Skipping sound '%s': missing file_path",
                    sound.id.c_str()
                );
                continue;
            }

            if (!registerSound(sound)) {
                SDL_LogError(LOG_SOUND, "ERROR: Unable to register sound '%s'", sound.id.c_str());
                continue;
            }
        }

        SDL_LogInfo(LOG_SOUND, "Finished reloading sound engine");

        auto stop = std::chrono::steady_clock::now();
        double dt = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count(); 
        SDL_LogDebug(LOG_SOUND, "Sound engine reload took [%ams]",dt);
    }

    void Engine::update(double dt) {
        // ----- One-shot sounds ----- //
        for (auto it = activeStreams.begin(); it != activeStreams.end();) {

            SDL_AudioStream *stream = *it;

            int queued = SDL_GetAudioStreamQueued(stream);
            int available = SDL_GetAudioStreamAvailable(stream);
            // Automatically destroyed as they are non-looping
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
                if (!SDL_PutAudioStreamData(stream, sound.data, static_cast<int>(sound.dataSize))) {
                    SDL_LogError(LOG_SOUND, "Failed to loop background sound '%s': %s", id.c_str(), SDL_GetError());
                }
            }
        }

        // ------- Soundtrack ------- //

        // -- Loop Sound Track -- //
        if(!activeSoundTrack.first.empty() && activeSoundTrack.second) {
            const std::string &id = activeSoundTrack.first;
            SDL_AudioStream *stream = activeSoundTrack.second;

            auto soundIt = registery.find(id);
            if (soundIt != registery.end()) {
                Registration &sound = soundIt->second;
    
                int queued = SDL_GetAudioStreamQueued(stream);
                if (queued <= static_cast<int>(sound.dataSize)) {
                    if (!SDL_PutAudioStreamData(stream, sound.data, static_cast<int>(sound.dataSize))) {
                        SDL_LogError(LOG_SOUND, "Failed to loop soundtrack '%s': %s", id.c_str(), SDL_GetError());
                    }
                }
            }
        }

        // -- Fade Into Next Track -- //
        if (!nextSoundTrack.first.empty() && nextSoundTrack.second) {
            // Has next soundtrack
            if (fadeTime != 0.0f) {
                // Defaults to 1.0 if the track does is not found
                float activeGainMul = 1.0f;
                float nextGainMul = 1.0f;

                // Active soundtrack may be empty
                if (!activeSoundTrack.first.empty() && activeSoundTrack.second) {
                    auto activeIt = registery.find(activeSoundTrack.first);
                    if (activeIt != registery.end()) {
                        activeGainMul = activeIt->second.gain;
                    }
                }

                // Set gain for the next soundtrack
                auto nextIt = registery.find(nextSoundTrack.first);
                if (nextIt != registery.end()) {
                    nextGainMul = nextIt->second.gain;
                }

                SDL_AudioStream* nextStream = nextSoundTrack.second;
                const float nextGain = std::clamp(
                    std::lerp(0.0f, 1.0f, fadeTimeElapsed / fadeTime),
                    0.0f,
                    1.0f
                ) * nextGainMul;
                SDL_AudioStream* activeStream = activeSoundTrack.second;
                const float activeGain = std::clamp(
                    std::lerp(0.0f, 1.0f, 1.0f - fadeTimeElapsed / fadeTime),
                    0.0f,
                    1.0f
                ) * activeGainMul;
                // Set fades between track
                SDL_SetAudioStreamGain(nextStream,nextGain);
                if(activeStream) SDL_SetAudioStreamGain(activeStream,activeGain);
            }
            
            if (fadeTimeElapsed > fadeTime) {
                if (activeSoundTrack.second) {
                    SDL_DestroyAudioStream(activeSoundTrack.second);
                }
                
                // Assign next track to be the active track
                activeSoundTrack.first = nextSoundTrack.first;
                activeSoundTrack.second = nextSoundTrack.second;
                
                // Clear next sound track as its been swapped to the first
                nextSoundTrack.first = "";
                nextSoundTrack.second = nullptr;
                
                // Get the gain attached to the sound
                float activeGainMul = 1.0f;

                auto activeIt = registery.find(activeSoundTrack.first);
                if (activeIt != registery.end()) {
                    activeGainMul = activeIt->second.gain;
                }

                SDL_SetAudioStreamGain(activeSoundTrack.second, activeGainMul);  // Set to full volume (gain value)

                SDL_LogInfo(LOG_SOUND, "Soundtrack '%s' set to active",activeSoundTrack.first.c_str());
            }
        }

        fadeTimeElapsed += static_cast<float>(dt);
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

        SDL_SetAudioStreamGain(stream,sound.gain);

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

        leftGain *= distanceGain * sound.gain;
        rightGain *= distanceGain * sound.gain;


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

        SDL_SetAudioStreamGain(stream, sound.gain);

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

    void Engine::setSoundTrack(const std::string &id, float fadeTime) {
        if (!initialized) {
            SDL_LogError(LOG_SOUND, "ERROR: Unable to set soundtrack for sound ID: %s as the engine is not initialized.", id.c_str());
            return;
        }

        auto it = registery.find(id);
        if (it == registery.end()) {
            SDL_LogError(LOG_SOUND, "Unable to set soundtrack for sound ID: %s as it is not in the registery", id.c_str());
            return;
        }

        if (id == activeSoundTrack.first) {
            SDL_LogDebug(LOG_SOUND, "'%s' already activley playing, swapping tracks", id.c_str());

            // Tracks are swapped so that we now have our incoming track fade BACK in from where it left off fading OUT
            std::swap(activeSoundTrack, nextSoundTrack); 

            const float activeGain = std::clamp(
                std::lerp(0.0f, 1.0f, 1.0f - fadeTimeElapsed / this->fadeTime),
                0.0f,
                1.0f
            );
            // Set fade time to be % of where we were before to make it seamless even with fade time changes
            fadeTimeElapsed = fadeTime * activeGain; 
            return;   
        }

        if (!nextSoundTrack.first.empty() || nextSoundTrack.second) {
            // Next soundtrack already set -- destroy it prior to replacing it
            SDL_LogDebug(
                LOG_SOUND, 
                "Replacing next track '%s' with '%s'", 
                nextSoundTrack.first.c_str(),
                id.c_str()
            );
            SDL_DestroyAudioStream(nextSoundTrack.second);
            nextSoundTrack.first = "";
            nextSoundTrack.second = nullptr;
        }
        
        fadeTimeElapsed = 0.0f;
        this->fadeTime = fadeTime;
        nextSoundTrack.first = id;

        Registration &sound = it->second;

        // Create stream
        SDL_AudioStream* stream = SDL_CreateAudioStream(&sound.spec, nullptr);
        nextSoundTrack.second = stream;
        if (!stream) {
            SDL_LogError(LOG_SOUND, "Failed to create background audio stream: %s", SDL_GetError());
            return;
        }

        // Bind to playback device
        if (!SDL_BindAudioStream(device, stream)) {
            SDL_LogError(LOG_SOUND, "Failed to bind background audio stream: %s", SDL_GetError());
            SDL_DestroyAudioStream(stream);
            return;
        }

        // Set gain to 0.0 so there is no audio (for fade in)
        if (!SDL_SetAudioStreamGain(stream, 0.0f)) {
            SDL_LogError(LOG_SOUND, "Failed to set background audio gain to 0.0: %s", SDL_GetError());
            SDL_DestroyAudioStream(stream);
            return;
        }

        // Fill stream with data for playback. Done twice to buffer for the looping
        if (!SDL_PutAudioStreamData(stream, sound.data, static_cast<int>(sound.dataSize))) {
            SDL_LogError(LOG_SOUND, "Failed to queue background audio: %s", SDL_GetError());
            SDL_DestroyAudioStream(stream);
            return;
        }

        if (!SDL_PutAudioStreamData(stream, sound.data, static_cast<int>(sound.dataSize))) {
            SDL_LogError(LOG_SOUND, "Failed to queue background audio: %s", SDL_GetError());
            SDL_DestroyAudioStream(stream);
            return;
        }

        SDL_LogInfo(LOG_SOUND, "Set next sound track to '%s' with fade time '%fs'", id.c_str(), fadeTime);
    }

    void Engine::stopSoundTrack([[maybe_unused]] float fadeTime) {
        // TODO
        SDL_LogWarn(LOG_SOUND, "stopSoundTrack Function unfinished -- does nothing");
    }

    bool Engine::isPlayingSoundTrack(const std::string &id) const {
        return (id == activeSoundTrack.first);
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

    void Engine::playSpatialLooped(const std::string &id, int instanceId, const Vec2f &pos) {
        // -- Find Registration -- //
        auto regIt = registery.find(id);
        if (regIt == registery.end()) {
            SDL_LogError(
                LOG_SOUND,
                "Sound '%s' not registered",
                id.c_str()
            );
            return;
        }

        SpatialLoopKey key = {id, instanceId};

        // -- Find In Spatial Map -- //
        auto it = spatialLoopMap.find(key);
        if (it != spatialLoopMap.end()) {

            SDL_LogDebug(
                LOG_SOUND, 
                "Audio (%s, %i) already playing, resuming playback", 
                key.soundId.c_str(),
                key.instanceId
            );
            
            SpatialLoop &audio = spatialLoopList[it->second]; 
            audio.playing = true;
            return;
        }

        // -- Create Stream -- //
        Registration &sound = regIt->second;

        SDL_AudioStream *stream = SDL_CreateAudioStream(
            &sound.spec,
            nullptr
        );

        if (!stream) {
            SDL_LogError(
                LOG_SOUND,
                "Failed to create looped spatial audio stream: %s",
                SDL_GetError()
            );
            return;
        }

        // Bind to playback device
        if (!SDL_BindAudioStream(device, stream)) {
            SDL_LogError(
                LOG_SOUND,
                "Failed to bind looped spatial audio stream: %s",
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
                "Failed to queue looped spatial audio: %s",
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
                "Failed to queue looped spatial audio: %s",
                SDL_GetError()
            );

            SDL_DestroyAudioStream(stream);
            return;
        }

        // -- Create New Spatial Loop -- //
        SpatialLoop audio {
            .index = spatialLoopList.size(),
            .stream = stream,
            .position = pos,
            .playing = true,
            .leftGain = 1.0f,
            .rightGain = 1.0f,
            .distanceGain = 1.0f
        };
        spatialLoopList.push_back(audio);

        spatialLoopMap[key] = audio.index;

        SDL_LogDebug(
            LOG_SOUND, 
            "Added new loop spatial audio (%s, %i)",
            key.soundId.c_str(),
            key.instanceId
        );
    }

    // -- PRIVATE -- //

    bool Engine::registerSound(const Config &config) {
        if (registery.contains(config.id)) {
            SDL_LogWarn(LOG_SOUND, "Sound ID: %s is already registered!", config.id.c_str());
            return false;
        }

        Registration sound;
        sound.gain = config.gain;
        if (!SDL_LoadWAV(config.filePath.c_str(), &sound.spec, &sound.data, &sound.dataSize)) {
            SDL_LogError(
                LOG_SOUND, 
                "ERROR: Unable to load sound: %s, reason: %s", 
                config.filePath.c_str(), 
                SDL_GetError()
            );
            return false;
        }

        registery[config.id] = sound;

        SDL_LogDebug(
            LOG_SOUND, 
            "Registered Sound"
            "\n - ID: %s"
            "\n - File Path: %s"
            "\n - Frequency: %i fps"
            "\n - Channels: %i channels"
            "\n - Sample Rate: %i Hz",
            config.id.c_str(),
            config.filePath.c_str(),
            sound.spec.freq,
            sound.spec.channels,
            sound.spec.freq * sound.spec.channels
        );

        return true;
    }

}