#include "audioEngine.hpp"
#include "miniaudio.h"
#include <iostream>

namespace CustomEngine
{
    Sound::~Sound()
    {
        if (m_Sound)
        {
            ma_sound_uninit(static_cast<::ma_sound*>(m_Sound));
            delete static_cast<::ma_sound*>(m_Sound);
        }
    }

    bool Sound::load(void* engine, const std::string& filePath)
    {
        ::ma_sound* sound = new ::ma_sound();
        ma_result result = ma_sound_init_from_file(static_cast<::ma_engine*>(engine), filePath.c_str(), 0, nullptr, nullptr, sound);
        if (result != MA_SUCCESS)
        {
            std::cerr << "[AudioEngine] Failed to load sound: " << filePath << std::endl;
            delete sound;
            return false;
        }
        m_Sound = sound;
        m_Loaded = true;
        return true;
    }

    void Sound::play()
    {
        if (m_Loaded)
        {
            ma_sound_seek_to_pcm_frame(static_cast<::ma_sound*>(m_Sound), 0);
            ma_sound_start(static_cast<::ma_sound*>(m_Sound));
        }
    }

    void Sound::stop()
    {
        if (m_Loaded)
            ma_sound_stop(static_cast<::ma_sound*>(m_Sound));
    }

    void Sound::setVolume(float volume)
    {
        if (m_Loaded)
            ma_sound_set_volume(static_cast<::ma_sound*>(m_Sound), volume);
    }

    void Sound::setLooping(bool loop)
    {
        if (m_Loaded)
            ma_sound_set_looping(static_cast<::ma_sound*>(m_Sound), loop ? MA_TRUE : MA_FALSE);
    }

    bool Sound::isPlaying() const
    {
        if (!m_Loaded) return false;
        return ma_sound_is_playing(static_cast<::ma_sound*>(m_Sound)) == MA_TRUE;
    }

    AudioEngine::AudioEngine()
    {
        ::ma_engine* engine = new ::ma_engine();
        ma_result result = ma_engine_init(nullptr, engine);
        if (result != MA_SUCCESS)
        {
            std::cerr << "[AudioEngine] Failed to initialise audio engine." << std::endl;
            delete engine;
            return;
        }
        m_Engine = engine;
    }

    AudioEngine::~AudioEngine()
    {
        m_Sounds.clear();
        if (m_Engine)
        {
            ma_engine_uninit(static_cast<::ma_engine*>(m_Engine));
            delete static_cast<::ma_engine*>(m_Engine);
        }
    }

    bool AudioEngine::loadSound(const std::string& name, const std::string& filePath, float volume)
    {
        if (!m_Engine) return false;
        auto sound = std::make_unique<Sound>();
        if (!sound->load(m_Engine, filePath))
            return false;
        if (volume >= 0.0f)
            sound->setVolume(volume);
        m_Sounds[name] = std::move(sound);
        return true;
    }

    void AudioEngine::playSound(const std::string& name)
    {
        auto it = m_Sounds.find(name);
        if (it != m_Sounds.end())
            it->second->play();
    }

    void AudioEngine::stopSound(const std::string& name)
    {
        auto it = m_Sounds.find(name);
        if (it != m_Sounds.end())
            it->second->stop();
    }

    Sound* AudioEngine::getSound(const std::string& name)
    {
        auto it = m_Sounds.find(name);
        if (it != m_Sounds.end())
            return it->second.get();
        return nullptr;
    }

    void AudioEngine::setMasterVolume(float volume)
    {
        if (m_Engine)
            ma_engine_set_volume(static_cast<::ma_engine*>(m_Engine), volume);
    }
}

