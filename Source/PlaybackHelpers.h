/**
 * @file PlaybackHelpers.h
 * @brief Defines the PlaybackHelpers class.
 * @ingroup Engine
 */

#ifndef AUDIOFILER_PLAYBACKHELPERS_H
#define AUDIOFILER_PLAYBACKHELPERS_H

#include <juce_core/juce_core.h>

/**
 * @class PlaybackHelpers
 * @brief Home: Engine.
 *
 */
class PlaybackHelpers
{
public:
    /**
     * @brief Undocumented method.
     * @param position [in] Description for position.
     * @param cutIn [in] Description for cutIn.
     * @param cutOut [in] Description for cutOut.
     * @return double
     */
    static double constrainPosition(double position, double cutIn, double cutOut);
};

#endif
