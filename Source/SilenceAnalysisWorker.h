#ifndef AUDIOFILER_SILENCEANALYSISWORKER_H
#define AUDIOFILER_SILENCEANALYSISWORKER_H

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
    #include <juce_events/juce_events.h>
#else
    #include <JuceHeader.h>
#endif

#include <atomic>
#include <memory>
#include "SilenceWorkerClient.h"

class SessionState;

/**
 * @ingroup Threading
 * @class SilenceAnalysisWorker
 * @brief Background thread for detecting silence in audio files.
 * @details This worker offloads the heavy processing of scanning large audio files
 *          to a separate thread to prevent UI freezing. It uses `SilenceAnalysisAlgorithms`
 *          to perform the actual sample analysis.
 *
 *          When analysis is complete, it updates `SessionState` (via `SilenceWorkerClient` or
 *          direct callback) with the detected silence boundaries.
 *
 * @see SilenceAnalysisAlgorithms
 */
class SilenceAnalysisWorker : public juce::Thread
{
public:

    explicit SilenceAnalysisWorker(SilenceWorkerClient& client, SessionState& sessionState);

    ~SilenceAnalysisWorker() override;

    void startAnalysis(float threshold, bool detectingIn);

    bool isBusy() const;

    bool isDetectingIn() const { return detectingIn.load(); }

private:

    void run() override;

    SilenceWorkerClient& client;
    SessionState& sessionState;
    std::atomic<float> threshold { 0.0f };
    std::atomic<bool> detectingIn { true };
    std::atomic<bool> busy { false };
    bool wasPlayingBeforeScan = false;
    juce::String assignedFilePath;

    std::shared_ptr<bool> lifeToken;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SilenceAnalysisWorker)
};

#endif 
