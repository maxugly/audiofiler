#pragma once

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
    static void logLoopStartSet(ControlPanel& panel, int sampleIndex, double sampleRate);
    static void logLoopEndSet(ControlPanel& panel, int sampleIndex, double sampleRate);
    static void logNoSoundFound(ControlPanel& panel, const juce::String& boundaryDescription);
    static void logAudioTooLarge(ControlPanel& panel);
};
