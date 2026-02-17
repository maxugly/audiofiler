#ifndef AUDIOFILER_SILENCEDETECTIONLOGGER_H
#define AUDIOFILER_SILENCEDETECTIONLOGGER_H

#include <JuceHeader.h>
#include "SilenceWorkerClient.h"

/**
 * @class SilenceDetectionLogger
 * @brief Provides consistent logging for silence detection events.
 */
class SilenceDetectionLogger final
{
public:
    static void logNoAudioLoaded(SilenceWorkerClient& client);
    static void logReadingSamples(SilenceWorkerClient& client, const juce::String& direction, juce::int64 length);
    static void logZeroLength(SilenceWorkerClient& client);
    static void logLoopStartSet(SilenceWorkerClient& client, juce::int64 sampleIndex, double sampleRate);
    static void logLoopEndSet(SilenceWorkerClient& client, juce::int64 sampleIndex, double sampleRate);
    static void logNoSoundFound(SilenceWorkerClient& client, const juce::String& boundaryDescription);
    static void logAudioTooLarge(SilenceWorkerClient& client);
};

#endif // AUDIOFILER_SILENCEDETECTIONLOGGER_H
