/**
 * @file TimeUtils.h
 * @brief Defines the TimeUtils class.
 * @ingroup Engine
 */

#ifndef AUDIOFILER_TIMEUTILS_H
#define AUDIOFILER_TIMEUTILS_H

#include <juce_core/juce_core.h>

/**
 * @class TimeUtils
 * @brief Home: Engine.
 *
 */
class TimeUtils
{
public:
    /**
     * @brief Undocumented method.
     * @param seconds [in] Description for seconds.
     * @return juce::String
     */
    static juce::String formatTime(double seconds);
    /**
     * @brief Undocumented method.
     * @param timeString [in] Description for timeString.
     * @return double
     */
    static double parseTime(const juce::String& timeString);
};

#endif
