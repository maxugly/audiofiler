

#include "PlaybackCursorGlow.h"

void PlaybackCursorGlow::renderGlow(juce::Graphics& g, int x, int topY, int bottomY, juce::Colour baseColor)
{
    const float glowWidth = Config::Layout::Glow::thickness;

    juce::ColourGradient gradient(baseColor.withAlpha(0.0f), (float)x - glowWidth, 0.0f,
                                  baseColor.withAlpha(0.0f), (float)x + glowWidth, 0.0f, false);

    gradient.addColour(0.5, baseColor.withAlpha(0.6f));
    g.setGradientFill(gradient);

    g.fillRect((float)x - glowWidth, (float)topY, glowWidth * 2.0f, (float)(bottomY - topY));

    g.setColour(baseColor);
    g.drawVerticalLine(x, (float)topY, (float)bottomY);
}
