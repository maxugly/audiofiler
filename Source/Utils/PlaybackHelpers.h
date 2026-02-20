

#ifndef AUDIOFILER_PLAYBACKHELPERS_H
#define AUDIOFILER_PLAYBACKHELPERS_H

#include <juce_core/juce_core.h>

class PlaybackHelpers
{
public:

    static double constrainPosition(double position, double cutIn, double cutOut);
};

#endif
