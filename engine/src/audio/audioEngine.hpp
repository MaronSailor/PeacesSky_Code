#pragma once

#include "../engineCore/core.h"
#include <string>
#include <unordered_map>
#include <memory>

namespace CustomEngine
{
    // single loaded sound, can be played multiple times
    class CUSTOMENGINE_API Sound
    {
    public:
        Sound() = default;
        ~Sound();

        Sound(const Sound&) = delete;
        Sound& operator=(const Sound&) = delete;
        Sound(Sound&&) = default;
        Sound& operator=(Sound&&) = default;

        bool load(void* engine, const std::string& filePath);
        void play();
        void stop();
        void setVolume(float volume); // 0.0 to 1.0
        void setLooping(bool loop);
        bool isPlaying() const;

    private:
        void* m_Sound = nullptr;
        bool m_Loaded = false;
    };

    // central audio manager, one per application
    class CUSTOMENGINE_API AudioEngine
    {
    public:
        AudioEngine();
        ~AudioEngine();

        AudioEngine(const AudioEngine&) = delete;
        AudioEngine& operator=(const AudioEngine&) = delete;
        AudioEngine(AudioEngine&&) = default;
        AudioEngine& operator=(AudioEngine&&) = default;

        // load sound from file by key; volume -1.0f keeps file default
        bool loadSound(const std::string& name, const std::string& filePath, float volume = -1.0f);

        // play a previously loaded sound
        void playSound(const std::string& name);
        void stopSound(const std::string& name);

        // fine-grained control over a sound
        Sound* getSound(const std::string& name);

        // global volume (0.0 - 1.0)
        void setMasterVolume(float volume);

    private:
        void* m_Engine = nullptr;
        std::unordered_map<std::string, std::unique_ptr<Sound>> m_Sounds;
    };
}
