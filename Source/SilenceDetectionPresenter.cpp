

#include "SilenceDetectionPresenter.h"

#include "ControlPanel.h"
#include "SilenceDetector.h"
#include "AudioPlayer.h"
#include "SessionState.h"
#include "SilenceAnalysisWorker.h"

SilenceDetectionPresenter::SilenceDetectionPresenter(ControlPanel& ownerPanel, SessionState& sessionStateIn, AudioPlayer& audioPlayerIn)
    : owner(ownerPanel),
      sessionState(sessionStateIn),
      audioPlayer(audioPlayerIn),
      silenceWorker(*this, sessionStateIn)
{
    sessionState.addListener(this);
    owner.getPlaybackTimerManager().addListener(this);
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
