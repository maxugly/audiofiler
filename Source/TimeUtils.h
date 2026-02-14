#ifndef AUDIOFILER_TIMEUTILS_H
#define AUDIOFILER_TIMEUTILS_H

#include <juce_core/juce_core.h>

class TimeUtils
{
public:
    static juce::String formatTime(double seconds);
    static double parseTime(const juce::String& timeString);
    static double getStepSize(int charIndex, bool shift, bool ctrl, bool alt, double sampleRate = 0.0);
};

#endif
