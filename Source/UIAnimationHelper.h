#ifndef AUDIOFILER_UIANIMATIONHELPER_H
#define AUDIOFILER_UIANIMATIONHELPER_H

#include <cmath>

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
#else
    #include <JuceHeader.h>
#endif

/**
 * @class UIAnimationHelper
 * @brief Header-only utility for normalized animation curves.
 * 
 * Provides static methods to process a linear master phase into 
 * periodic curves with zero dependency on UI components.
 */
class UIAnimationHelper {
public:
    /**
     * @brief Returns a 0.0 to 1.0 value mapped to a sine curve.
     * @param masterPhase A value from 0.0 to 1.0 representing the master clock.
     * @param multiplier Frequency multiplier for the pulse.
     * @return A float between 0.0 and 1.0.
     */
    static float getSinePulse(float masterPhase, float multiplier) {
        float rawSine = std::sin(masterPhase * multiplier * juce::MathConstants<float>::twoPi);
        // Map [-1.0, 1.0] to [0.0, 1.0]
        return (rawSine + 1.0f) * 0.5f;
    }
};

#endif
