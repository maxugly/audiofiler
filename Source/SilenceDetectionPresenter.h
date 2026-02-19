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

/**
 * @class SilenceDetectionPresenter
 * @brief Handles auto-cut toggle behaviour and detection triggers for the SilenceDetector.
 *
 * It also manages the background SilenceAnalysisWorker and implements the
 * SilenceWorkerClient interface to receive analysis results.
 */
class SilenceDetectionPresenter final : public SilenceWorkerClient
{
public:
    /**
     * @brief Binds the presenter to a specific ControlPanel instance.
     * @param ownerPanel ControlPanel whose SilenceDetector is coordinated.
     * @param sessionState The global session state.
     * @param audioPlayer The audio player.
     */
    SilenceDetectionPresenter(ControlPanel& ownerPanel, SessionState& sessionState, AudioPlayer& audioPlayer);

    /**
     * @brief Handles toggling the auto-cut-in mode.
     * @param isActive True when auto-cut-in should be active.
     */
    void handleAutoCutInToggle(bool isActive);

    /**
     * @brief Handles toggling the auto-cut-out mode.
     * @param isActive True when auto-cut-out should be active.
     */
    void handleAutoCutOutToggle(bool isActive);

    /**
     * @brief Starts an asynchronous silence analysis scan.
     * @param threshold The normalized amplitude threshold (0-1).
     * @param detectingIn True to find the start of sound (In), false for end (Out).
     */
    void startSilenceAnalysis(float threshold, bool detectingIn);

    /**
     * @brief Checks if an analysis is currently in progress.
     */
    bool isAnalyzing() const { return silenceWorker.isBusy(); }

    // SilenceWorkerClient implementation
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

#endif // AUDIOFILER_SILENCEDETECTIONPRESENTER_H
