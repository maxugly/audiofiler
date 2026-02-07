#pragma once

#include <JuceHeader.h>

#include "Config.h"



/**
 * @brief Manages audio file loading, playback, and transport.
 *
 * This class handles all core audio operations, including reading audio files,
 * managing the transport state (play, stop, looping), and providing the audio

 * stream to the host. It also manages an audio thumbnail for waveform display.
 */
class AudioPlayer : public juce::AudioSource,
                    public juce::ChangeListener,
                    public juce::ChangeBroadcaster
{
public:
    /**
     * @brief Constructs the AudioPlayer.
     */
    AudioPlayer();

    /**
     * @brief Destructor.
     */
    ~AudioPlayer() override;

    /**
     * @brief Loads an audio file for playback.
     * @param file The audio file to load.
     * @return A juce::Result indicating success or failure.
     */
    juce::Result loadFile(const juce::File& file);

    /**
     * @brief Toggles the playback state between playing and stopped.
     */
    void togglePlayStop();

    /**
     * @brief Checks if audio is currently playing.
     * @return True if playing, false otherwise.
     */
    bool isPlaying() const;

    /**
     * @brief Checks if looping is enabled.
     * @return True if looping is enabled.
     */
    bool isLooping() const;

    /**
     * @brief Enables or disables looping.
     * @param shouldLoop True to enable looping.
     */
    void setLooping(bool shouldLoop);

    /**
     * @brief Gets a reference to the audio thumbnail.
     * @return A reference to the juce::AudioThumbnail.
     */
    juce::AudioThumbnail& getThumbnail();

    /**
     * @brief Gets a reference to the audio transport source.
     * @return A reference to the juce::AudioTransportSource.
     */
    juce::AudioTransportSource& getTransportSource();

    /**
     * @brief Gets a reference to the audio format manager.
     * @return A reference to the juce::AudioFormatManager.
     */
    juce::AudioFormatManager& getFormatManager();

    //==============================================================================
    // juce::AudioSource overrides
    //==============================================================================

    /** @internal */
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    /** @internal */
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    /** @internal */
    void releaseResources() override;

    //==============================================================================
    // juce::ChangeListener override
    //==============================================================================

    /** @internal */
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

public:
    /**
     * @brief Gets the underlying AudioFormatReader.
     * @return A pointer to the juce::AudioFormatReader, or nullptr if not loaded.
     */
    juce::AudioFormatReader* getAudioFormatReader() const;

private:
    //==============================================================================
    // Private Methods
    //==============================================================================


    //==============================================================================
    // Member Variables
    //==============================================================================
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;


    bool looping = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPlayer)
};