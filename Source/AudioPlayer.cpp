

#include "AudioPlayer.h"
#include "PlaybackHelpers.h"
#include "SessionState.h"
#include "FileMetadata.h"
#if !defined(JUCE_HEADLESS)
#include "ControlPanel.h"
#include "SilenceDetectionPresenter.h"
#endif
#include <algorithm>
#include <cmath>

AudioPlayer::AudioPlayer(SessionState& state)
    #if !defined(JUCE_HEADLESS)
    : waveformManager(formatManager),
    #else
    :
    #endif
      readAheadThread("Audio File Reader"),
      sessionState(state)
{
    formatManager.registerBasicFormats();
    sessionState.addListener(this);
    readAheadThread.startThread();
    transportSource.addChangeListener(this);

    lastAutoCutThresholdIn = sessionState.getCutPrefs().autoCut.thresholdIn;
    lastAutoCutThresholdOut = sessionState.getCutPrefs().autoCut.thresholdOut;
    lastAutoCutInActive = sessionState.getCutPrefs().autoCut.inActive;
    lastAutoCutOutActive = sessionState.getCutPrefs().autoCut.outActive;
}

AudioPlayer::~AudioPlayer()
{
    sessionState.removeListener(this);
    transportSource.setSource(nullptr);
    readAheadThread.stopThread(1000);
    transportSource.removeChangeListener(this);
}

juce::Result AudioPlayer::loadFile(const juce::File& file)
{
    auto* reader = formatManager.createReaderFor(file);

    if (reader != nullptr)
    {
        const juce::String filePath = file.getFullPathName();
        sessionState.setCurrentFilePath(filePath);

        const double totalDuration = (double)reader->lengthInSamples / reader->sampleRate;
        sessionState.setTotalDuration(totalDuration);

        if (sessionState.hasMetadataForFile(filePath))
        {
            const FileMetadata cached = sessionState.getMetadataForFile(filePath);
            sessionState.setMetadataForFile(filePath, cached);
        }
        else
        {
            FileMetadata metadata;
            if (reader->sampleRate > 0.0)
                metadata.cutOut = totalDuration;
            sessionState.setMetadataForFile(filePath, metadata);
        }

        lastAutoCutThresholdIn = sessionState.getCutPrefs().autoCut.thresholdIn;
        lastAutoCutThresholdOut = sessionState.getCutPrefs().autoCut.thresholdOut;
        lastAutoCutInActive = sessionState.getCutPrefs().autoCut.inActive;
        lastAutoCutOutActive = sessionState.getCutPrefs().autoCut.outActive;

        loadedFile = file;
        {

            std::lock_guard<std::mutex> lock(readerMutex);
            auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
            transportSource.setSource(newSource.get(), Config::Audio::readAheadBufferSize, &readAheadThread, reader->sampleRate);
            #if !defined(JUCE_HEADLESS)
            waveformManager.loadFile(file);
            #endif
            readerSource.reset(newSource.release());
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

    return juce::Result::fail("Failed to read audio file: " + file.getFileName());
}

juce::File AudioPlayer::getLoadedFile() const
{
    return loadedFile;
}

void AudioPlayer::startSilenceAnalysis(float threshold, bool detectingIn)
{
#if !defined(JUCE_HEADLESS)
    if (controlPanel != nullptr)
    {
        if (auto* presenter = controlPanel->getSilenceDetectionPresenter())
            presenter->startSilenceAnalysis(threshold, detectingIn);
    }
#else

    juce::ignoreUnused(threshold, detectingIn);
#endif
}

void AudioPlayer::togglePlayStop()
{
    if (transportSource.isPlaying())
        transportSource.stop();
    else
        transportSource.start();
}

bool AudioPlayer::isPlaying() const
{
    return transportSource.isPlaying();
}

double AudioPlayer::getCurrentPosition() const
{
    return transportSource.getCurrentPosition();
}

bool AudioPlayer::isRepeating() const
{
    return repeating;
}

void AudioPlayer::setRepeating(bool shouldRepeat)
{
    repeating = shouldRepeat;
}

#if !defined(JUCE_HEADLESS)
juce::AudioThumbnail& AudioPlayer::getThumbnail()
{
    return waveformManager.getThumbnail();
}

WaveformManager& AudioPlayer::getWaveformManager()
{
    return waveformManager;
}

const WaveformManager& AudioPlayer::getWaveformManager() const
{
    return waveformManager;
}
#endif

void AudioPlayer::startPlayback()
{
    transportSource.start();
}

void AudioPlayer::stopPlayback()
{
    transportSource.stop();
}

void AudioPlayer::stopPlaybackAndReset()
{
    transportSource.stop();
    setPlayheadPosition(sessionState.getCutIn());
}

juce::AudioFormatManager& AudioPlayer::getFormatManager()
{
    return formatManager;
}

void AudioPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void AudioPlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    const auto prefs = sessionState.getCutPrefs();
    if (!prefs.active)
    {
        transportSource.getNextAudioBlock(bufferToFill);
        return;
    }

    double sampleRate = 0.0;
    juce::int64 lengthInSamples = 0;
    if (!getReaderInfo(sampleRate, lengthInSamples) || sampleRate <= 0.0)
    {
        transportSource.getNextAudioBlock(bufferToFill);
        return;
    }

    const double cutIn = prefs.cutIn;
    const double cutOut = prefs.cutOut;
    const double startPos = transportSource.getCurrentPosition();

    if (startPos >= cutOut)
    {
        if (repeating)
        {
            transportSource.setPosition(cutIn);
            transportSource.start();
            transportSource.getNextAudioBlock(bufferToFill);
        }
        else
        {
            transportSource.stop();
            transportSource.setPosition(cutOut);
            bufferToFill.clearActiveBufferRegion();
        }
        return;
    }

    transportSource.getNextAudioBlock(bufferToFill);

    const double endPos = startPos + ((double)bufferToFill.numSamples / sampleRate);
    if (endPos >= cutOut)
    {
        const int samplesToKeep = juce::jlimit(
            0,
            bufferToFill.numSamples,
            (int)std::floor((cutOut - startPos) * sampleRate));

        if (samplesToKeep < bufferToFill.numSamples)
        {
            bufferToFill.buffer->clear(bufferToFill.startSample + samplesToKeep,
                                       bufferToFill.numSamples - samplesToKeep);
        }

        if (repeating)
        {
            transportSource.setPosition(cutIn);
            transportSource.start();
        }
        else
        {
            transportSource.stop();
            transportSource.setPosition(cutOut);
        }
    }
}

void AudioPlayer::releaseResources()
{
    transportSource.releaseResources();
}

void AudioPlayer::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &transportSource)
    {

        sendChangeMessage();
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

#if JUCE_UNIT_TESTS
void AudioPlayer::setSourceForTesting(juce::PositionableAudioSource* source, double sampleRate)
{
    transportSource.setSource(source, 0, nullptr, sampleRate);
}
#endif

void AudioPlayer::setPlayheadPosition(double seconds)
{
    double sampleRate = 0.0;
    juce::int64 lengthInSamples = 0;
    if (!getReaderInfo(sampleRate, lengthInSamples) || sampleRate <= 0.0)
        return;

    const double totalDuration = (double)lengthInSamples / sampleRate;

    double cutIn = 0.0;
    double cutOut = totalDuration;

    const auto prefs = sessionState.getCutPrefs();
    if (prefs.active)
    {
        cutIn = prefs.cutIn;
        cutOut = prefs.cutOut;
    }

    double clampedPos = juce::jlimit(cutIn, cutOut, seconds);
    transportSource.setPosition(clampedPos);
}
