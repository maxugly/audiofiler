#include "AudioPlayer.h"
#include "PlaybackHelpers.h"
#include "SessionState.h"
#include "FileMetadata.h"
#if !defined(JUCE_HEADLESS)
#include "ControlPanel.h"
#include "SilenceDetectionPresenter.h"
#endif
#include <algorithm>

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

        if (sessionState.hasMetadataForFile(filePath))
        {
            const FileMetadata cached = sessionState.getMetadataForFile(filePath);
            sessionState.setMetadataForFile(filePath, cached);
        }
        else
        {
            FileMetadata metadata;
            if (reader->sampleRate > 0.0)
                metadata.cutOut = (double)reader->lengthInSamples / reader->sampleRate;
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
#endif

juce::AudioTransportSource& AudioPlayer::getTransportSource()
{
    return transportSource;
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


