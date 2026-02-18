#include "AudioPlayer.h"
#include "PlaybackHelpers.h"
#include "SessionState.h"
#include "FileMetadata.h"
#include <algorithm>

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
AudioPlayer::AudioPlayer(SessionState& state)
    #if !defined(JUCE_HEADLESS)
    : waveformManager(formatManager),
    #else
    :
    #endif
      readAheadThread("Audio File Reader"),
      sessionState(state),
      silenceWorker(*this, state)
{
    formatManager.registerBasicFormats(); // Register standard audio file formats
    sessionState.addListener(this);
    readAheadThread.startThread(); // Start background thread for file reading
    transportSource.addChangeListener(this); // Listen to transportSource for changes (e.g., playback finished)

    lastAutoCutThresholdIn = sessionState.getCutPrefs().autoCut.thresholdIn;
    lastAutoCutThresholdOut = sessionState.getCutPrefs().autoCut.thresholdOut;
    lastAutoCutInActive = sessionState.getCutPrefs().autoCut.inActive;
    lastAutoCutOutActive = sessionState.getCutPrefs().autoCut.outActive;
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
    sessionState.removeListener(this);
    transportSource.setSource(nullptr); // Ensure source is detached before thread stops
    readAheadThread.stopThread(1000); // Stop background thread
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
    silenceWorker.signalThreadShouldExit();
    auto* reader = formatManager.createReaderFor(file);

    if (reader != nullptr)
    {
        const juce::String filePath = file.getFullPathName();
        sessionState.setCurrentFilePath(filePath);

        if (sessionState.hasMetadataForFile(filePath))
        {
            const FileMetadata cached = sessionState.getMetadataForFile(filePath);
            sessionState.setMetadataForFile(filePath, cached);
        }
        else
        {
            FileMetadata metadata;
            if (reader->sampleRate > 0.0)
                metadata.cutOut = reader->lengthInSamples / reader->sampleRate;
            // Initialize the Cut Workspace to the full track on first load.
            sessionState.setMetadataForFile(filePath, metadata);
        }

        lastAutoCutThresholdIn = sessionState.getCutPrefs().autoCut.thresholdIn;
        lastAutoCutThresholdOut = sessionState.getCutPrefs().autoCut.thresholdOut;
        lastAutoCutInActive = sessionState.getCutPrefs().autoCut.inActive;
        lastAutoCutOutActive = sessionState.getCutPrefs().autoCut.outActive;

        loadedFile = file; // Store the loaded file for later reference
        {
            std::lock_guard<std::mutex> lock(readerMutex);
            auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true); // True means reader is owned by source
            // Use background thread for file reading with configured buffer size
            transportSource.setSource(newSource.get(), Config::Audio::readAheadBufferSize, &readAheadThread, reader->sampleRate);
            #if !defined(JUCE_HEADLESS)
            waveformManager.loadFile(file);
            #endif
            readerSource.reset(newSource.release()); // Transfer ownership to unique_ptr
        }
        setPlayheadPosition(sessionState.getCutPrefs().cutIn);

        const FileMetadata activeMetadata = sessionState.getMetadataForFile(filePath);
        if (!activeMetadata.isAnalyzed)
        {
            if (sessionState.getCutPrefs().autoCut.inActive)
                startSilenceAnalysis(sessionState.getCutPrefs().autoCut.thresholdIn, true);
            if (sessionState.getCutPrefs().autoCut.outActive)
                startSilenceAnalysis(sessionState.getCutPrefs().autoCut.thresholdOut, false);
        }
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

void AudioPlayer::startSilenceAnalysis(float threshold, bool detectingIn)
{
    silenceWorker.startAnalysis(threshold, detectingIn);
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
 * @brief Checks if repeating is enabled.
 * @return True if the internal `repeating` flag is set, false otherwise.
 */
bool AudioPlayer::isRepeating() const
{
    return repeating;
}

/**
 * @brief Enables or disables repeating.
 * @param shouldRepeat True to enable repeating, false to disable.
 *
 * This method updates an internal flag. The actual repeating logic is handled
 * within `getNextAudioBlock` when the stream finishes.
 */
void AudioPlayer::setRepeating(bool shouldRepeat)
{
    repeating = shouldRepeat;
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
    return waveformManager.getThumbnail();
}

WaveformManager& AudioPlayer::getWaveformManager()
{
    return waveformManager;
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
 * the logic to handle repeating: if the stream finishes and `isRepeating()` is true,
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

    if (sessionState.getCutPrefs().active)
    {
        const double currentPos = transportSource.getCurrentPosition();
        if (currentPos >= sessionState.getCutPrefs().cutOut)
        {
            if (repeating)
                setPlayheadPosition(sessionState.getCutPrefs().cutIn);
            else
                transportSource.stop();
        }
    }
}

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

void AudioPlayer::cutPreferenceChanged(const MainDomain::CutPreferences& prefs)
{
    const auto& autoCut = prefs.autoCut;
    const bool inThresholdChanged = autoCut.thresholdIn != lastAutoCutThresholdIn;
    const bool outThresholdChanged = autoCut.thresholdOut != lastAutoCutThresholdOut;
    const bool inActiveChanged = autoCut.inActive != lastAutoCutInActive;
    const bool outActiveChanged = autoCut.outActive != lastAutoCutOutActive;

    const bool shouldAnalyzeIn = (inThresholdChanged || inActiveChanged) && autoCut.inActive;
    const bool shouldAnalyzeOut = (outThresholdChanged || outActiveChanged) && autoCut.outActive;

    if (shouldAnalyzeIn)
        startSilenceAnalysis(autoCut.thresholdIn, true);
    else if (shouldAnalyzeOut)
        startSilenceAnalysis(autoCut.thresholdOut, false);

    lastAutoCutThresholdIn = autoCut.thresholdIn;
    lastAutoCutThresholdOut = autoCut.thresholdOut;
    lastAutoCutInActive = autoCut.inActive;
    lastAutoCutOutActive = autoCut.outActive;
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

bool AudioPlayer::getReaderInfo(double& sampleRateOut, juce::int64& lengthInSamplesOut) const
{
    std::lock_guard<std::mutex> lock(readerMutex);
    if (readerSource == nullptr)
        return false;

    auto* reader = readerSource->getAudioFormatReader();
    if (reader == nullptr)
        return false;

    sampleRateOut = reader->sampleRate;
    lengthInSamplesOut = reader->lengthInSamples;
    return true;
}

void AudioPlayer::setPlayheadPosition(double seconds)
{
    if (readerSource == nullptr)
        return;

    double sampleRate = 0.0;
    juce::int64 lengthInSamples = 0;
    if (!getReaderInfo(sampleRate, lengthInSamples) || sampleRate <= 0.0)
        return;

    const double totalDuration = (double)lengthInSamples / sampleRate;

    double effectiveIn = 0.0;
    double effectiveOut = totalDuration;
    if (sessionState.getCutPrefs().active)
    {
        effectiveIn = juce::jmin(sessionState.getCutPrefs().cutIn, sessionState.getCutPrefs().cutOut);
        effectiveOut = juce::jmax(sessionState.getCutPrefs().cutIn, sessionState.getCutPrefs().cutOut);
    }

    transportSource.setPosition(juce::jlimit(effectiveIn, effectiveOut, seconds));
}
