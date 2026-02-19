

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
}

void SilenceDetectionPresenter::handleAutoCutInToggle(bool isActive)
{
    owner.setAutoCutInActive(isActive);
}

void SilenceDetectionPresenter::handleAutoCutOutToggle(bool isActive)
{
    owner.setAutoCutOutActive(isActive);
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
        owner.updateCutLabels();
        owner.repaint();
    }
}

void SilenceDetectionPresenter::setCutEnd(int sampleIndex)
{
    double sampleRate = 0.0;
    juce::int64 length = 0;
    if (audioPlayer.getReaderInfo(sampleRate, length) && sampleRate > 0.0)
    {
        sessionState.setCutOut((double)sampleIndex / sampleRate);
        owner.updateCutLabels();
        owner.repaint();
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
