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

#include "Utils/Config.h"
#include "Core/SessionState.h"
#include "MainDomain.h"
#if !defined(JUCE_HEADLESS)
#include "Core/WaveformManager.h"
#endif
#include <mutex>

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

    /** @brief Seeks to the specified position in seconds, clamped by cut boundaries if active. */
    void setPlayheadPosition(double seconds);

    /** @brief Loads an audio file and synchronizes SessionState with its metadata. */
    juce::Result loadFile(const juce::File& file);

    /** @brief Toggles between playback and paused states. */
    void togglePlayStop();

    /** @brief Returns true if the transport is currently playing. */
    bool isPlaying() const;

    /** @brief Returns the current transport position in seconds. */
    double getCurrentPosition() const;

    /** @brief Returns true if the player is set to loop between cut points. */
    bool isRepeating() const;

    /** @brief Sets whether the player should loop between cut points. */
    void setRepeating(bool shouldRepeat);

    #if !defined(JUCE_HEADLESS)

    /** @brief Returns the audio thumbnail for waveform rendering. */
    juce::AudioThumbnail& getThumbnail();

    /** @brief Provides access to the WaveformManager for thumbnail updates. */
    WaveformManager& getWaveformManager();

    /** @brief Provides read-only access to the WaveformManager. */
    const WaveformManager& getWaveformManager() const;
    #endif

    /** @brief Starts audio playback. */
    void startPlayback();

    /** @brief Stops audio playback. */
    void stopPlayback();

    /** @brief Stops playback and seeks back to the cut-in position. */
    void stopPlaybackAndReset();

    /** @brief Provides access to the global audio format manager. */
    juce::AudioFormatManager& getFormatManager();

    /** @brief Returns the underlying audio format reader for the loaded file. */
    juce::AudioFormatReader* getAudioFormatReader() const;

    /** @brief Returns the juce::File handle for the currently loaded audio. */
    juce::File getLoadedFile() const;

    /** @brief Initializes audio processing parameters. */
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
    float lastAutoCutThresholdIn{-1.0f};
    float lastAutoCutThresholdOut{-1.0f};
    bool lastAutoCutInActive{false};
    bool lastAutoCutOutActive{false};
    mutable std::mutex readerMutex;

    bool repeating = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPlayer)
};

#endif 
