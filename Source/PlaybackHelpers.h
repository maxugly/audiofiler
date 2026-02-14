#ifndef AUDIOFILER_PLAYBACKHELPERS_H
#define AUDIOFILER_PLAYBACKHELPERS_H

#include <juce_core/juce_core.h>

namespace PlaybackHelpers {

    /**
     * @brief Constrains a position within the given loop bounds.
     * @param newPosition The desired position.
     * @param loopIn The loop start point.
     * @param loopOut The loop end point.
     * @return The constrained position.
     */
    static inline double constrainPosition(double newPosition, double loopIn, double loopOut) {
        // Ensure loopIn is not greater than loopOut to prevent jlimit issues
        const double effectiveLoopIn = juce::jmin(loopIn, loopOut);
        const double effectiveLoopOut = juce::jmax(loopIn, loopOut);

        // Constrain the new position within the effective loop bounds
        return juce::jlimit(effectiveLoopIn, effectiveLoopOut, newPosition);
    }

}

#endif // AUDIOFILER_PLAYBACKHELPERS_H
