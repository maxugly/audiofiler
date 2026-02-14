#ifndef AUDIOFILER_TIMEUTILS_H
#define AUDIOFILER_TIMEUTILS_H

#include <juce_core/juce_core.h>

class TimeUtils
{
public:
    static juce::String formatTime(double seconds);
    static double parseTime(const juce::String& timeString);
};

#endif
