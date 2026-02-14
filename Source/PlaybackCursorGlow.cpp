/*
  ==============================================================================

    PlaybackCursorGlow.cpp
    Created: 14 Feb 2026
    Author:  RenderAgent

  ==============================================================================
*/

#include "PlaybackCursorGlow.h"

void PlaybackCursorGlow::renderGlow(juce::Graphics& g, int x, int topY, int bottomY, juce::Colour baseColor)
{
    const float glowWidth = Config::Layout::Glow::thickness;
    // Create a horizontal gradient for the glow: Transparent -> Color -> Transparent
    juce::ColourGradient gradient(baseColor.withAlpha(0.0f), (float)x - glowWidth, 0.0f,
                                  baseColor.withAlpha(0.0f), (float)x + glowWidth, 0.0f, false);
    // Use a slightly higher alpha for the center to ensure visibility, as requested
    gradient.addColour(0.5, baseColor.withAlpha(0.6f));
    g.setGradientFill(gradient);
    // Draw the glow
    g.fillRect((float)x - glowWidth, (float)topY, glowWidth * 2.0f, (float)(bottomY - topY));
    // Draw the core 1-pixel vertical line
    g.setColour(baseColor);
    g.drawVerticalLine(x, (float)topY, (float)bottomY);
}
