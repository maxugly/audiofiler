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

/**
 * @file AudioPlayer.h
 * @brief Defines the AudioPlayer class for managing audio playback and resources.
 */

/**
 * @class AudioPlayer
 * @brief Manages audio file loading, playback, and transport.
 *
 * This class serves as the core audio engine for the application, handling all
 * aspects of audio playback. It is responsible for:
 * - Loading audio files from disk.
 * - Controlling playback (start, stop, pause, cutModeActive).
 * - Managing the audio transport (position, length).
 * - Providing audio data to the JUCE audio callback system (`juce::AudioSource`).
 * - Generating and managing an audio thumbnail for visual representation of the waveform.
 *
 * It acts as a `juce::ChangeListener` to respond to changes in its internal
 * `juce::AudioTransportSource` and as a `juce::ChangeBroadcaster` to notify
 * other components (e.g., UI elements) about changes in its state.
 */
class AudioPlayer : public juce::AudioSource,
                    public juce::ChangeListener,
                    public juce::ChangeBroadcaster
{
public:
    //==============================================================================
    /** @name Constructors and Destructors
     *  @{
     */

    /**
     * @brief Constructs the AudioPlayer.
     *
     * Initializes the JUCE audio format manager to support various audio file types.
     */
    AudioPlayer();

    /**
     * @brief Destructor.
     *
     * Cleans up resources, including ensuring the transport source is stopped
     * and any loaded audio format reader source is released.
     */
    ~AudioPlayer() override;

    /**
     * @brief Sets the playback position, constrained by provided loop points.
     * @param newPosition The desired new position in seconds.
     * @param cutIn The cut-in position in seconds.
     * @param cutOut The cut-out position in seconds.
     */
    void setPositionConstrained(double newPosition, double cutIn, double cutOut);

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name Core Playback Control
     *  @{
     */

    /**
     * @brief Loads an audio file for playback.
     *
     * This method attempts to load the specified audio file, prepare the transport
     * source, and generate a thumbnail. It will automatically stop any current
     * playback before loading a new file.
     * @param file The `juce::File` object representing the audio file to load.
     * @return A `juce::Result` indicating `juce::Result::ok()` on success, or
     *         an error if the file could not be loaded or processed.
     */
    juce::Result loadFile(const juce::File& file);

    /**
     * @brief Toggles the playback state between playing and stopped.
     *
     * If audio is currently playing, it will stop. If stopped (and a file is loaded),
     * it will start playing from the current transport position.
     */
    void togglePlayStop();

    /**
     * @brief Checks if audio is currently playing.
     * @return True if the transport source is currently playing, false otherwise.
     */
    bool isPlaying() const;

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name Looping Control
     *  @{
     */

    /**
     * @brief Checks if cutModeActive is enabled for the current playback.
     * @return True if playback will loop when it reaches the end of the current loop region, false otherwise.
     */
    bool isCutModeActive() const;

    /**
     * @brief Enables or disables cutModeActive for playback.
     * @param isCutModeActive True to enable cutModeActive, false to disable.
     */
    void setCutModeActive(bool isCutModeActive);

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name Component Accessors
     *  Getters for internal JUCE components managed by this class.
     *  @{
     */

    /**
     * @brief Gets a reference to the internal `juce::AudioThumbnail`.
     *
     * This is used by UI components (e.g., `MainComponent`) to draw the waveform.
     * @return A reference to the `juce::AudioThumbnail` object.
     */
    #if !defined(JUCE_HEADLESS)
    juce::AudioThumbnail& getThumbnail();
    #endif

    /**
     * @brief Gets a reference to the internal `juce::AudioTransportSource`.
     *
     * This provides direct access to control the audio playback transport,
     * including setting playback position, starting, and stopping.
     * @return A reference to the `juce::AudioTransportSource` object.
     */
    juce::AudioTransportSource& getTransportSource();

    /**
     * @brief Gets a reference to the internal `juce::AudioFormatManager`.
     *
     * This is used to register and manage audio file formats.
     * @return A reference to the `juce::AudioFormatManager` object.
     */
    juce::AudioFormatManager& getFormatManager();

    /**
     * @brief Gets the underlying `juce::AudioFormatReader` for the currently loaded file.
     *
     * This can be used for direct access to raw audio data, for example,
     * for silence detection algorithms.
     * @return A pointer to the `juce::AudioFormatReader`, or `nullptr` if no file is loaded.
     */
    juce::AudioFormatReader* getAudioFormatReader() const;

    /**
     * @brief Gets the `juce::File` object representing the currently loaded audio file.
     * @return The `juce::File` object.
     */
    juce::File getLoadedFile() const;

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name juce::AudioSource Overrides
     *  Methods inherited from `juce::AudioSource` for the audio processing chain.
     *  These are typically called by the audio device directly.
     *  @{
     */

    /**
     * @brief Prepares the audio source for playback.
     *
     * This method is called by the audio device to set up internal resources
     * (e.g., sample rate, buffer sizes) before playback begins.
     * @param samplesPerBlockExpected The expected number of samples in each audio block.
     * @param sampleRate The sample rate of the audio device.
     */
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;

    /**
     * @brief Fills an audio buffer with the next block of audio data.
     *
     * This is the main audio callback where audio processing and sample output occurs.
     * It pulls audio from the `transportSource`.
     * @param bufferToFill The buffer structure to be filled with audio data.
     */
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

    /**
     * @brief Releases any audio resources held by the source.
     *
     * Called when playback stops or when the audio device is shut down.
     */
    void releaseResources() override;

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name juce::ChangeListener Overrides
     *  Callback for receiving change notifications from observed JUCE objects.
     *  @{
     */

    /**
     * @brief Callback method triggered when an observed `juce::ChangeBroadcaster` changes state.
     *
     * This `AudioPlayer` listens to its `transportSource` to detect when it stops
     * (e.g., reaching the end of the file) and can then handle cutModeActive or re-broadcasting
     * the change.
     * @param source A pointer to the `juce::ChangeBroadcaster` that initiated the change.
     */
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    /** @} */
    //==============================================================================

private:
    //==============================================================================
    /** @name Private Member Variables
     *  Internal components and state of the AudioPlayer.
     *  @{
     */

    juce::AudioFormatManager formatManager;                 ///< Manages registered audio file formats (e.g., WAV, AIFF, OGG).
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource; ///< Handles reading audio data from a file.
    juce::TimeSliceThread readAheadThread;                  ///< Thread for background audio file reading.
    juce::AudioTransportSource transportSource;             ///< Controls playback, such as starting, stopping, and positioning.

    #if !defined(JUCE_HEADLESS)
    juce::AudioThumbnailCache thumbnailCache;               ///< Caches audio thumbnails to avoid re-generating them.
    juce::AudioThumbnail thumbnail;                         ///< Generates and stores the visual waveform data.
    #endif

    juce::File loadedFile;                                  ///< Stores the currently loaded audio file.

    bool cutModeActive = false;                                   ///< Flag indicating if playback should loop.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPlayer) ///< Macro to prevent copying and detect memory leaks.

    /** @} */
    //==============================================================================
};

#endif // AUDIOFILER_AUDIOPLAYER_H
