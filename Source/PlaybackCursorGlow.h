/*
  ==============================================================================

    PlaybackCursorGlow.h
    Created: 14 Feb 2026
    Author:  RenderAgent

  ==============================================================================
*/

#pragma once

#if defined(JUCE_HEADLESS)
    #include <juce_graphics/juce_graphics.h>
#else
    #include <JuceHeader.h>
#endif

#include "Config.h"

class PlaybackCursorGlow
{
public:
    /**
     * @brief Renders the playback cursor glow and the cursor line itself.
     *
     * @param g The Graphics context.
     * @param x The x-coordinate of the cursor.
     * @param topY The top y-coordinate of the line.
     * @param bottomY The bottom y-coordinate of the line.
     * @param baseColor The base color of the cursor.
     */
    static void renderGlow(juce::Graphics& g, int x, int topY, int bottomY, juce::Colour baseColor);
};
