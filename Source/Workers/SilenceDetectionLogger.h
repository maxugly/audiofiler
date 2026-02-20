

#ifndef AUDIOFILER_SILENCEDETECTIONLOGGER_H
#define AUDIOFILER_SILENCEDETECTIONLOGGER_H

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
#else
    #include <JuceHeader.h>
#endif

#include "Workers/SilenceWorkerClient.h"

class SilenceDetectionLogger final
{
public:

    static void logNoAudioLoaded(SilenceWorkerClient& client);

    static void logReadingSamples(SilenceWorkerClient& client, const juce::String& direction, juce::int64 length);

    static void logZeroLength(SilenceWorkerClient& client);

    static void logCutInSet(SilenceWorkerClient& client, juce::int64 sampleIndex, double sampleRate);

    static void logCutOutSet(SilenceWorkerClient& client, juce::int64 sampleIndex, double sampleRate);

    static void logNoSoundFound(SilenceWorkerClient& client, const juce::String& boundaryDescription);

    static void logAudioTooLarge(SilenceWorkerClient& client);
};

#endif 
