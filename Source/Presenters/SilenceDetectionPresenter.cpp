

#include "Presenters/SilenceDetectionPresenter.h"

#include "UI/ControlPanel.h"
#include "Workers/SilenceDetector.h"
#include "Core/AudioPlayer.h"
#include "Core/SessionState.h"
#include "Core/SilenceAnalysisWorker.h"

SilenceDetectionPresenter::SilenceDetectionPresenter(ControlPanel& ownerPanel, SessionState& sessionStateIn, AudioPlayer& audioPlayerIn)
    : owner(ownerPanel),
      sessionState(sessionStateIn),
      audioPlayer(audioPlayerIn),
      silenceWorker(*this, sessionStateIn)
{
    sessionState.addListener(this);
    owner.getPlaybackTimerManager().addListener(this);

    auto prefs = sessionState.getCutPrefs();
    lastAutoCutThresholdIn = prefs.autoCut.thresholdIn;
    lastAutoCutThresholdOut = prefs.autoCut.thresholdOut;
    lastAutoCutInActive = prefs.autoCut.inActive;
    lastAutoCutOutActive = prefs.autoCut.outActive;
}

SilenceDetectionPresenter::~SilenceDetectionPresenter()
{
    owner.getPlaybackTimerManager().removeListener(this);
    sessionState.removeListener(this);
}

void SilenceDetectionPresenter::playbackTimerTick()
{
}

void SilenceDetectionPresenter::animationUpdate(float breathingPulse)
{
    if (silenceWorker.isBusy())
    {
        auto& button = silenceWorker.isDetectingIn() ? owner.getAutoCutInButton() : owner.getAutoCutOutButton();
        button.getProperties().set("isProcessing", true);
        button.getProperties().set("pulseAlpha", breathingPulse);
        button.repaint();
    }
    else
    {
        // Ensure both buttons have the property cleared when analysis is not running
        if (owner.getAutoCutInButton().getProperties().getWithDefault("isProcessing", false))
        {
            owner.getAutoCutInButton().getProperties().set("isProcessing", false);
            owner.getAutoCutInButton().repaint();
        }
        if (owner.getAutoCutOutButton().getProperties().getWithDefault("isProcessing", false))
        {
            owner.getAutoCutOutButton().getProperties().set("isProcessing", false);
            owner.getAutoCutOutButton().repaint();
        }
    }
}

void SilenceDetectionPresenter::fileChanged(const juce::String& filePath)
{
    if (filePath.isEmpty())
        return;

    const FileMetadata activeMetadata = sessionState.getMetadataForFile(filePath);
    if (!activeMetadata.isAnalyzed)
    {
        auto prefs = sessionState.getCutPrefs();
        if (prefs.autoCut.inActive)
            startSilenceAnalysis(prefs.autoCut.thresholdIn, true);
        else if (prefs.autoCut.outActive)
            startSilenceAnalysis(prefs.autoCut.thresholdOut, false);
    }
}

void SilenceDetectionPresenter::cutPreferenceChanged(const MainDomain::CutPreferences& prefs)
{
    const auto& autoCut = prefs.autoCut;
    const bool inThresholdChanged = !juce::exactlyEqual(autoCut.thresholdIn, lastAutoCutThresholdIn);
    const bool outThresholdChanged = !juce::exactlyEqual(autoCut.thresholdOut, lastAutoCutThresholdOut);
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

void SilenceDetectionPresenter::handleAutoCutInToggle(bool isActive)
{
    sessionState.setAutoCutInActive(isActive);
}

void SilenceDetectionPresenter::handleAutoCutOutToggle(bool isActive)
{
    sessionState.setAutoCutOutActive(isActive);
}

void SilenceDetectionPresenter::startSilenceAnalysis(float threshold, bool detectingIn)
{
    silenceWorker.startAnalysis(threshold, detectingIn);
}

AudioPlayer& SilenceDetectionPresenter::getAudioPlayer()
{
    return audioPlayer;
}

void SilenceDetectionPresenter::setCutStart(int sampleIndex)
{
    double sampleRate = 0.0;
    juce::int64 length = 0;
    if (audioPlayer.getReaderInfo(sampleRate, length) && sampleRate > 0.0)
    {
        sessionState.setCutIn((double)sampleIndex / sampleRate);
    }
}

void SilenceDetectionPresenter::setCutEnd(int sampleIndex)
{
    double sampleRate = 0.0;
    juce::int64 length = 0;
    if (audioPlayer.getReaderInfo(sampleRate, length) && sampleRate > 0.0)
    {
        sessionState.setCutOut((double)sampleIndex / sampleRate);
    }
}

void SilenceDetectionPresenter::logStatusMessage(const juce::String& message, bool isError)
{
    owner.logStatusMessage(message, isError);
}

bool SilenceDetectionPresenter::isCutModeActive() const
{
    return owner.isCutModeActive();
}

bool SilenceDetectionPresenter::hasLoadedAudio() const
{
    return audioPlayer.getThumbnail().getTotalLength() > 0.0;
}
