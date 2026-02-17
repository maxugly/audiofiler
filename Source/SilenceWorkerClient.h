#ifndef AUDIOFILER_SILENCEWORKERCLIENT_H
#define AUDIOFILER_SILENCEWORKERCLIENT_H

#include <JuceHeader.h>
class AudioPlayer;

/**
 * @class SilenceWorkerClient
 * @brief Interface for any component that wishes to use the SilenceAnalysisWorker.
 *
 * This abstraction allows the worker to be unit tested without instantiating the full ControlPanel,
 * and decouples the worker from the UI implementation.
 */
class SilenceWorkerClient
{
public:
    virtual ~SilenceWorkerClient() = default;

    /** @brief Provides access to the audio player for scanning. */
    virtual AudioPlayer& getAudioPlayer() = 0;

    /** @brief Updates the loop start position. */
    virtual void setLoopInPosition(double seconds) = 0;

    /** @brief Updates the loop end position. */
    virtual void setLoopOutPosition(double seconds) = 0;

    /** @brief Gets the current loop start position (needed for safe checks). */
    virtual double getLoopInPosition() const = 0;

    /** @brief Checks if cut mode is active. */
    virtual bool isCutModeActive() const = 0;

    /** @brief Logs a status message (e.g., to the stats display). */
    virtual void logStatusMessage(const juce::String& message, bool isError = false) = 0;
};

#endif // AUDIOFILER_SILENCEWORKERCLIENT_H
