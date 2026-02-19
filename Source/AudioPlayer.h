/**
 * @file AudioPlayer.h
 * @brief Manages Gated VLAN playback logic and private transport.
 * @ingroup Engine
 */

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

/**
 * @class ControlPanel
 * @brief Home: View.
 *
 */
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
    /**
     * @brief Constructs an AudioPlayer.
     * @param state The session state manager to use for playback information.
     */
    explicit AudioPlayer(SessionState& state);
    /**
     * @brief Undocumented method.
     */
    ~AudioPlayer() override;

    
    void setPlayheadPosition(double seconds);

    /**
     * @brief Undocumented method.
     * @param file [in] Description for file.
     * @return juce::Result
     */
    juce::Result loadFile(const juce::File& file);
    /**
     * @brief Undocumented method.
     */
    void togglePlayStop();
    /**
     * @brief Checks if Playing.
     * @return bool
     */
    bool isPlaying() const;
    /**
     * @brief Gets the CurrentPosition.
     * @return double
     */
    double getCurrentPosition() const;

    
    void setControlPanel(ControlPanel* panel) { controlPanel = panel; }

    
    void startSilenceAnalysis(float threshold, bool detectingIn);

    
    bool isRepeating() const;

    
    void setRepeating(bool shouldRepeat);

    #if !defined(JUCE_HEADLESS)
    /**
     * @brief Gets the Thumbnail.
     * @return juce::AudioThumbnail&
     */
    juce::AudioThumbnail& getThumbnail();
    /**
     * @brief Gets the WaveformManager.
     * @return WaveformManager&
     */
    WaveformManager& getWaveformManager();
    /**
     * @brief Gets the WaveformManager.
     * @return const WaveformManager&
     */
    const WaveformManager& getWaveformManager() const;
    #endif

    /**
     * @brief Undocumented method.
     */
    void startPlayback();
    /**
     * @brief Undocumented method.
     */
    void stopPlayback();
    /**
     * @brief Gets the FormatManager.
     * @return juce::AudioFormatManager&
     */
    juce::AudioFormatManager& getFormatManager();
    /**
     * @brief Gets the AudioFormatReader.
     * @return juce::AudioFormatReader*
     */
    juce::AudioFormatReader* getAudioFormatReader() const;
    /**
     * @brief Gets the LoadedFile.
     * @return juce::File
     */
    juce::File getLoadedFile() const;

    /**
     * @brief Undocumented method.
     * @param samplesPerBlockExpected [in] Description for samplesPerBlockExpected.
     * @param sampleRate [in] Description for sampleRate.
     */
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    /**
     * @brief Gets the NextAudioBlock.
     * @param bufferToFill [in] Description for bufferToFill.
     */
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    /**
     * @brief Undocumented method.
     */
    void releaseResources() override;

    /**
     * @brief Undocumented method.
     * @param source [in] Description for source.
     */
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    /**
     * @brief Undocumented method.
     * @param prefs [in] Description for prefs.
     */
    void cutPreferenceChanged(const MainDomain::CutPreferences& prefs) override;
    
    /**
     * @brief Gets the CutIn.
     * @return double
     */
    double getCutIn() const { return sessionState.getCutIn(); }
    /**
     * @brief Gets the CutOut.
     * @return double
     */
    double getCutOut() const { return sessionState.getCutOut(); }
    /**
     * @brief Sets the CutIn.
     * @param positionSeconds [in] Description for positionSeconds.
     */
    void setCutIn(double positionSeconds) { sessionState.setCutIn(positionSeconds); }
    /**
     * @brief Sets the CutOut.
     * @param positionSeconds [in] Description for positionSeconds.
     */
    void setCutOut(double positionSeconds) { sessionState.setCutOut(positionSeconds); }
    
    /**
     * @brief Gets the ReaderMutex.
     * @return std::mutex&
     */
    std::mutex& getReaderMutex() { return readerMutex; }
    /**
     * @brief Gets the ReaderInfo.
     * @param sampleRateOut [in] Description for sampleRateOut.
     * @param lengthInSamplesOut [in] Description for lengthInSamplesOut.
     * @return bool
     */
    bool getReaderInfo(double& sampleRateOut, juce::int64& lengthInSamplesOut) const;

#if JUCE_UNIT_TESTS
    /**
     * @brief Sets the SourceForTesting.
     * @param source [in] Description for source.
     * @param sampleRate [in] Description for sampleRate.
     */
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
