#ifndef AUDIOFILER_TIMEUTILS_H
#define AUDIOFILER_TIMEUTILS_H

#include <juce_core/juce_core.h>

class TimeUtils
{
public:
    enum class ValidationResult
    {
        Valid,      ///< The time is valid and within the total length.
        Invalid,    ///< The time string could not be parsed (e.g. invalid format).
        OutOfRange  ///< The time is valid but outside the allowed range (e.g. > totalLength).
    };

    static juce::String formatTime(double seconds);
    static double parseTime(const juce::String& timeString);
    static ValidationResult validateTime(const juce::String& text, double totalLength);
};

#endif
