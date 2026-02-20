

#pragma once

#if defined(JUCE_HEADLESS)
    #include <juce_graphics/juce_graphics.h>
#else
    #include <JuceHeader.h>
#endif

#include "Utils/Config.h"

class PlaybackCursorGlow
{
public:

    static void renderGlow(juce::Graphics& g, int x, int topY, int bottomY, juce::Colour baseColor);
};
