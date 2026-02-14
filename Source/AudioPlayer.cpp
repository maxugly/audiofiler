#include "AudioPlayer.h"
#include "PlaybackHelpers.h"

/**
 * @file AudioPlayer.cpp
 * @brief Implements the AudioPlayer class, managing audio file loading, playback, and transport.
 *
 * This file provides the concrete implementations for the methods declared in `AudioPlayer.h`.
 * It leverages JUCE's audio classes to handle sound file I/O, playback control,
 * and waveform visualization.
 */

/**
 * @brief Constructs the AudioPlayer.
 *
 * Initializes the `thumbnailCache` (which stores waveform data to avoid re-calculating),
 * and the `thumbnail` (which generates the visual waveform). It registers basic audio
 * formats (like WAV, AIFF) with the `formatManager` to enable loading common audio files.
 * Finally, it adds itself as a `ChangeListener` to `transportSource` so it can react
 * to playback state changes (e.g., reaching the end of the file).
 */
AudioPlayer::AudioPlayer()
    #if !defined(JUCE_HEADLESS)
    : thumbnailCache(Config::Audio::thumbnailCacheSize), // Initialize thumbnail cache with a configured size
      thumbnail(Config::Audio::thumbnailSizePixels, formatManager, thumbnailCache) // Initialize thumbnail with configured size
    #endif
{
    formatManager.registerBasicFormats(); // Register standard audio file formats
    transportSource.addChangeListener(this); // Listen to transportSource for changes (e.g., playback finished)
}

/**
 * @brief Destructor.
 *
 * Removes itself as a `ChangeListener` from `transportSource` to prevent dangling
 * pointers or crashes if the `transportSource` outlives this `AudioPlayer` object.
 * This is crucial for proper JUCE component lifecycle management.
 */
AudioPlayer::~AudioPlayer()
{
    transportSource.removeChangeListener(this);
}

/**
 * @brief Loads an audio file for playback.
 * @param file The audio file to load.
 * @return A `juce::Result` indicating success or failure.
 *
 * This method first attempts to create a `juce::AudioFormatReader` for the given file.
 * If successful, it stores the file, creates a new `juce::AudioFormatReaderSource`
 * (which wraps the reader for use with the `AudioTransportSource`), and sets this
 * as the new audio source for `transportSource`. It also updates the `thumbnail`
 * to reflect the newly loaded audio. The `readerSource` is managed by a `std::unique_ptr`
 * to ensure proper memory cleanup.
 */
juce::Result AudioPlayer::loadFile(const juce::File& file)
{
    auto* reader = formatManager.createReaderFor(file);

    if (reader != nullptr)
    {
        loadedFile = file; // Store the loaded file for later reference
        auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true); // True means reader is owned by source
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate); // Set new source for playback
        #if !defined(JUCE_HEADLESS)
        thumbnail.setSource(new juce::FileInputSource(file)); // Update thumbnail to reflect new file
        #endif
        readerSource.reset(newSource.release()); // Transfer ownership to unique_ptr
        return juce::Result::ok();
    }

    // Return a detailed error message if file loading fails
    return juce::Result::fail("Failed to read audio file: " + file.getFileName());
}

/**
 * @brief Gets the currently loaded audio file.
 * @return The `juce::File` object representing the loaded audio file.
 *
 * This accessor provides external components with information about the currently
 * active audio file.
 */
juce::File AudioPlayer::getLoadedFile() const
{
    return loadedFile;
}

/**
 * @brief Toggles the playback state between playing and stopped.
 *
 * This provides a simple interface to start or stop audio playback without
 * needing to directly interact with the `transportSource`.
 */
void AudioPlayer::togglePlayStop()
{
    if (transportSource.isPlaying())
        transportSource.stop(); // If playing, stop
    else
        transportSource.start(); // If stopped, start
}

/**
 * @brief Checks if audio is currently playing.
 * @return True if the `transportSource` is currently playing, false otherwise.
 */
bool AudioPlayer::isPlaying() const
{
    return transportSource.isPlaying();
}

/**
 * @brief Checks if looping is enabled.
 * @return True if the internal `looping` flag is set, false otherwise.
 */
bool AudioPlayer::isLooping() const
{
    return looping;
}

/**
 * @brief Enables or disables looping.
 * @param shouldLoop True to enable looping, false to disable.
 *
 * This method updates an internal flag. The actual looping logic is handled
 * within `getNextAudioBlock` when the stream finishes.
 */
void AudioPlayer::setLooping(bool shouldLoop)
{
    looping = shouldLoop;
}

/**
 * @brief Gets a reference to the internal `juce::AudioThumbnail`.
 * @return A reference to the `juce::AudioThumbnail` object.
 *
 * This accessor allows other components, particularly UI elements, to
 * retrieve and draw the waveform visualization.
 */
#if !defined(JUCE_HEADLESS)
juce::AudioThumbnail& AudioPlayer::getThumbnail()
{
    return thumbnail;
}
#endif

/**
 * @brief Gets a reference to the internal `juce::AudioTransportSource`.
 * @return A reference to the `juce::AudioTransportSource` object.
 *
 * Provides direct access to the transport source for more granular control
 * over playback, such as setting playback position, without exposing its
 * internal details.
 */
juce::AudioTransportSource& AudioPlayer::getTransportSource()
{
    return transportSource;
}

/**
 * @brief Gets a reference to the internal `juce::AudioFormatManager`.
 * @return A reference to the `juce::AudioFormatManager` object.
 *
 * Allows external components to register additional audio formats if needed.
 */
juce::AudioFormatManager& AudioPlayer::getFormatManager()
{
    return formatManager;
}

/**
 * @brief Prepares the audio source for playback.
 * @param samplesPerBlockExpected The expected number of samples in each audio block.
 * @param sampleRate The sample rate of the audio device.
 *
 * This method is called by the audio device to set up internal resources
 * for the `transportSource` before playback begins.
 */
void AudioPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

/**
 * @brief Fills an audio buffer with the next block of audio data.
 * @param bufferToFill The buffer structure to be filled with audio data.
 *
 * This is the main audio callback method where audio data is requested from
 * the `transportSource` and sent to the audio output device. It also contains
 * the logic to handle looping: if the stream finishes and `isLooping()` is true,
 * the playback position is reset to the beginning.
 */
void AudioPlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // If no audio file is loaded, clear the buffer to prevent silence or clicks
    if (readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    // Request the next block of audio from the transport source
    transportSource.getNextAudioBlock(bufferToFill);
}

/**
 * @brief Releases any audio resources held by the source.
 *
 * This method is called by the audio device when playback stops or when the
 * audio device is shut down, ensuring that resources held by `transportSource`
 * are properly freed.
 */
void AudioPlayer::releaseResources()
{
    transportSource.releaseResources();
}

/**
 * @brief Callback method triggered when an observed `juce::ChangeBroadcaster` changes state.
 * @param source A pointer to the `juce::ChangeBroadcaster` that initiated the change.
 *
 * This `AudioPlayer` listens to its `transportSource`. When the `transportSource`
 * changes state (e.g., finishes playing), this callback is triggered. This `AudioPlayer`
 * then re-broadcasts this change using `sendChangeMessage()` so that other
 * components (like `MainComponent`) can react to the playback state change.
 */
void AudioPlayer::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &transportSource)
    {
        sendChangeMessage(); // Forward the change to listeners (e.g., MainComponent)
    }
}

/**
 * @brief Gets the underlying `juce::AudioFormatReader` for the currently loaded file.
 * @return A pointer to the `juce::AudioFormatReader`, or `nullptr` if no file is loaded.
 *
 * This provides direct access to the raw audio reader, which is particularly useful
 * for advanced audio processing tasks like silence detection that need to read
 * raw sample data.
 */
juce::AudioFormatReader* AudioPlayer::getAudioFormatReader() const
{
    if (readerSource != nullptr)
        return readerSource->getAudioFormatReader();
    return nullptr;
}

/**
 * @brief Sets the playback position, constrained by provided loop points.
 * @param newPosition The desired new position in seconds.
 * @param loopIn The loop-in position in seconds.
 * @param loopOut The loop-out position in seconds.
 *
 * This method ensures that the playback position is always within the
 * specified loop-in and loop-out bounds.
 */
void AudioPlayer::setPositionConstrained(double newPosition, double loopIn, double loopOut)
{
    transportSource.setPosition(PlaybackHelpers::constrainPosition(newPosition, loopIn, loopOut));
}
