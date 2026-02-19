/**
 * @file PlaybackHelpers.cpp
 * @brief Defines the PlaybackHelpers class.
 * @ingroup Engine
 */

#include "PlaybackHelpers.h"

double PlaybackHelpers::constrainPosition(double position, double cutIn, double cutOut)
{
    const double effectiveCutIn = juce::jmin(cutIn, cutOut);
    const double effectiveCutOut = juce::jmax(cutIn, cutOut);
    /**
     * @brief Undocumented method.
     * @param effectiveCutIn [in] Description for effectiveCutIn.
     * @param effectiveCutOut [in] Description for effectiveCutOut.
     * @param position [in] Description for position.
     * @return return
     */
    return juce::jlimit(effectiveCutIn, effectiveCutOut, position);
}
