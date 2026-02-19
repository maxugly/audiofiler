

#ifndef AUDIOFILER_SILENCEDETECTIONPRESENTER_H
#define AUDIOFILER_SILENCEDETECTIONPRESENTER_H

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
#else
    #include <JuceHeader.h>
#endif

#include "SilenceWorkerClient.h"
#include "SilenceAnalysisWorker.h"

class ControlPanel;

class SessionState;

class AudioPlayer;

class SilenceDetectionPresenter final : public SilenceWorkerClient
{
public:

    SilenceDetectionPresenter(ControlPanel& ownerPanel, SessionState& sessionState, AudioPlayer& audioPlayer);

    void handleAutoCutInToggle(bool isActive);

    void handleAutoCutOutToggle(bool isActive);

    void startSilenceAnalysis(float threshold, bool detectingIn);

    bool isAnalyzing() const { return silenceWorker.isBusy(); }

    AudioPlayer& getAudioPlayer() override;

    void setCutStart(int sampleIndex) override;

    void setCutEnd(int sampleIndex) override;

    void logStatusMessage(const juce::String& message, bool isError = false) override;
    bool isCutModeActive() const override;

private:

    bool hasLoadedAudio() const;

    ControlPanel& owner;
    SessionState& sessionState;
    AudioPlayer& audioPlayer;
    SilenceAnalysisWorker silenceWorker;
};

#endif 
