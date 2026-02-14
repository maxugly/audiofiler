#ifndef AUDIOFILER_SILENCEDETECTIONLOGGER_H
#define AUDIOFILER_SILENCEDETECTIONLOGGER_H

#include <JuceHeader.h>

class ControlPanel;

/**
 * @class SilenceDetectionLogger
 * @brief Provides consistent logging for silence detection events.
 */
class SilenceDetectionLogger final
{
public:
    static void logNoAudioLoaded(ControlPanel& panel);
    static void logReadingSamples(ControlPanel& panel, const juce::String& direction, juce::int64 length);
    static void logZeroLength(ControlPanel& panel);
    static void logLoopStartSet(ControlPanel& panel, juce::int64 sampleIndex, double sampleRate);
    static void logLoopEndSet(ControlPanel& panel, juce::int64 sampleIndex, double sampleRate);
    static void logNoSoundFound(ControlPanel& panel, const juce::String& boundaryDescription);
    static void logAudioTooLarge(ControlPanel& panel);
};

#endif // AUDIOFILER_SILENCEDETECTIONLOGGER_H
