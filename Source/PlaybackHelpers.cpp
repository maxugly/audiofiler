#include "PlaybackHelpers.h"

double PlaybackHelpers::constrainPosition(double position, double cutIn, double cutOut)
{
    const double effectiveLoopIn = juce::jmin(cutIn, cutOut);
    const double effectiveLoopOut = juce::jmax(cutIn, cutOut);
    return juce::jlimit(effectiveLoopIn, effectiveLoopOut, position);
}
