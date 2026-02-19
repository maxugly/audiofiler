#ifndef AUDIOFILER_AUDIOPLAYER_H
#define AUDIOFILER_AUDIOPLAYER_H

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
    #include <juce_audio_basics/juce_audio_basics.h>
    #include <juce_audio_formats/juce_audio_formats.h>
    #include <juce_audio_devices/juce_audio_devices.h>
    #include <juce_events/juce_events.h>
#else
    #include <JuceHeader.h>
#endif

#include "Config.h"
#include "SessionState.h"
#include "MainDomain.h"
#if !defined(JUCE_HEADLESS)
#include "WaveformManager.h"
#endif
#include <mutex>

class ControlPanel;

/**
 * @class AudioPlayer
 * @brief Manages audio file loading, playback, and transport.
 */
class AudioPlayer : public juce::AudioSource,
                    public juce::ChangeListener,
                    public juce::ChangeBroadcaster,
                    public SessionState::Listener
{
public:
    explicit AudioPlayer(SessionState& state);
    ~AudioPlayer() override;

    /** @brief Sets the playback position. */
    void setPlayheadPosition(double seconds);

    juce::Result loadFile(const juce::File& file);
    void togglePlayStop();
    bool isPlaying() const;

    /** @brief Sets the ControlPanel reference for silence detection delegation. */
    void setControlPanel(ControlPanel* panel) { controlPanel = panel; }

    /** @brief Starts a background silence analysis (delegated via ControlPanel). */
    void startSilenceAnalysis(float threshold, bool detectingIn);

    /** @brief Checks if repeating is enabled. */
    bool isRepeating() const;

    /** @brief Enables or disables repeating for playback. */
    void setRepeating(bool shouldRepeat);

    #if !defined(JUCE_HEADLESS)
    juce::AudioThumbnail& getThumbnail();
    WaveformManager& getWaveformManager();
    #endif

    juce::AudioTransportSource& getTransportSource();
    juce::AudioFormatManager& getFormatManager();
    juce::AudioFormatReader* getAudioFormatReader() const;
    juce::File getLoadedFile() const;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void cutPreferenceChanged(const MainDomain::CutPreferences& prefs) override;
    
    double getCutIn() const { return sessionState.getCutIn(); }
    double getCutOut() const { return sessionState.getCutOut(); }
    void setCutIn(double positionSeconds) { sessionState.setCutIn(positionSeconds); }
    void setCutOut(double positionSeconds) { sessionState.setCutOut(positionSeconds); }
    
    std::mutex& getReaderMutex() { return readerMutex; }
    bool getReaderInfo(double& sampleRateOut, juce::int64& lengthInSamplesOut) const;

private:
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::TimeSliceThread readAheadThread;
    juce::AudioTransportSource transportSource;

    #if !defined(JUCE_HEADLESS)
    WaveformManager waveformManager;
    #endif

    juce::File loadedFile;
    SessionState& sessionState;
    ControlPanel* controlPanel{nullptr};
    float lastAutoCutThresholdIn{-1.0f};
    float lastAutoCutThresholdOut{-1.0f};
    bool lastAutoCutInActive{false};
    bool lastAutoCutOutActive{false};
    mutable std::mutex readerMutex;

    bool repeating = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPlayer)
};

#endif // AUDIOFILER_AUDIOPLAYER_H
