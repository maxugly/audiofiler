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
 * @ingroup AudioEngine
 * @class AudioPlayer
 * @brief High-level audio playback and file handling class.
 * @details This class wraps `juce::AudioTransportSource` and handles loading audio files,
 *          managing playback position, and enforcing cut regions defined in `SessionState`.
 *
 *          It runs a background `juce::TimeSliceThread` for read-ahead buffering to ensure
 *          smooth playback.
 *
 * @see SessionState
 * @see MainComponent
 */
class AudioPlayer : public juce::AudioSource,
                    public juce::ChangeListener,
                    public juce::ChangeBroadcaster,
                    public SessionState::Listener
{
public:

    explicit AudioPlayer(SessionState& state);

    ~AudioPlayer() override;

    void setPlayheadPosition(double seconds);

    juce::Result loadFile(const juce::File& file);

    void togglePlayStop();

    bool isPlaying() const;

    double getCurrentPosition() const;

    void setControlPanel(ControlPanel* panel) { controlPanel = panel; }

    void startSilenceAnalysis(float threshold, bool detectingIn);

    bool isRepeating() const;

    void setRepeating(bool shouldRepeat);

    #if !defined(JUCE_HEADLESS)

    juce::AudioThumbnail& getThumbnail();

    WaveformManager& getWaveformManager();

    const WaveformManager& getWaveformManager() const;
    #endif

    void startPlayback();

    void stopPlayback();

    void stopPlaybackAndReset();

    juce::AudioFormatManager& getFormatManager();

    juce::AudioFormatReader* getAudioFormatReader() const;

    juce::File getLoadedFile() const;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;

    /**
     * @brief Processes the next block of audio samples.
     * @details This is the core audio processing callback. The logic sequence is:
     *          1. Check if a valid reader source exists. If not, clear the buffer.
     *          2. Retrieve the current cut preferences from `SessionState`.
     *          3. If cut mode is inactive, simply delegate to `transportSource`.
     *          4. If active, check the current playback position against `cutIn` and `cutOut`.
     *          5. If the position exceeds `cutOut`:
     *             - If looping is enabled, seek back to `cutIn`.
     *             - If not, stop playback.
     *          6. If the current block crosses the `cutOut` boundary, fade out or truncate
     *             the buffer to ensure no audio is played past the cut point.
     *
     * @param bufferToFill The buffer to populate with audio data.
     */
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

#if JUCE_UNIT_TESTS

    void setSourceForTesting(juce::PositionableAudioSource* source, double sampleRate);
#endif

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

#endif 
