#ifndef AUDIOFILER_SILENCEANALYSISWORKER_H
#define AUDIOFILER_SILENCEANALYSISWORKER_H

#include <JuceHeader.h>
#include <atomic>
#include <memory>
#include "SilenceWorkerClient.h"

/**
 * @class SilenceAnalysisWorker
 * @brief Handles background processing for silence detection.
 *
 * Runs the heavy mathematical scanning (delegated to SilenceAnalysisAlgorithms)
 * on a background thread to prevent UI freezing. It manages the thread lifecycle
 * and communicates results back to the SilenceWorkerClient (ControlPanel) on the message thread.
 */
class SilenceAnalysisWorker : public juce::Thread
{
public:
    /**
     * @brief Constructs the worker thread.
     * @param client The component that owns this worker and receives updates (e.g., ControlPanel).
     */
    explicit SilenceAnalysisWorker(SilenceWorkerClient& client);

    /**
     * @brief Destructor.
     * Ensures the thread is stopped before destruction.
     */
    ~SilenceAnalysisWorker() override;

    /**
     * @brief Starts an asynchronous silence analysis scan.
     *
     * If a scan is already in progress, this method returns immediately without
     * starting a new one. It handles pausing playback on the main thread before
     * launching the background task.
     *
     * @param threshold The normalized amplitude threshold (0-1).
     * @param detectingIn True to find the start of sound (In), false for end (Out).
     */
    void startAnalysis(float threshold, bool detectingIn);

    /**
     * @brief Checks if the worker is currently running a scan.
     * @return True if busy, false otherwise.
     */
    bool isBusy() const;

private:
    /**
     * @brief The thread body that performs the scanning.
     */
    void run() override;

    SilenceWorkerClient& client;
    std::atomic<float> threshold { 0.0f };
    std::atomic<bool> detectingIn { true };
    std::atomic<bool> busy { false };
    bool wasPlayingBeforeScan = false;

    // Token to ensure safe async callbacks
    std::shared_ptr<bool> lifeToken;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SilenceAnalysisWorker)
};

#endif // AUDIOFILER_SILENCEANALYSISWORKER_H
