#ifndef AUDIOFILER_TIMEUTILS_H
#define AUDIOFILER_TIMEUTILS_H

#include <juce_core/juce_core.h>

/**
 * @ingroup Helpers
 * @class TimeUtils
 * @brief Static utility class for time formatting and parsing.
 */
class TimeUtils
{
public:

    /**
     * @brief Formats a time value in seconds into a string (HH:MM:SS.ms).
     * @param seconds The time in seconds.
     * @return The formatted string.
     */
    static juce::String formatTime(double seconds);

    /**
     * @brief Parses a time string (e.g., "1:30.5") into seconds.
     * @param timeString The string to parse.
     * @return The time in seconds, or 0.0 if parsing fails.
     */
    static double parseTime(const juce::String& timeString);
};

#endif
