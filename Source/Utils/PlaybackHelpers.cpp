

#include "Utils/PlaybackHelpers.h"

double PlaybackHelpers::constrainPosition(double position, double cutIn, double cutOut)
{
    const double effectiveCutIn = juce::jmin(cutIn, cutOut);
    const double effectiveCutOut = juce::jmax(cutIn, cutOut);

    return juce::jlimit(effectiveCutIn, effectiveCutOut, position);
}
