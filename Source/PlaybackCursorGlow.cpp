#include "PlaybackCursorGlow.h"
#include "ControlPanel.h"
#include "AudioPlayer.h"
#include "Config.h"

void PlaybackCursorGlow::renderGlow(juce::Graphics& g, const ControlPanel& controlPanel, const juce::Rectangle<int>& waveformBounds)
{
    auto& audioPlayer = controlPanel.getAudioPlayer();
    const float audioLength = (float)audioPlayer.getThumbnail().getTotalLength();
    if (audioLength <= 0.0f)
        return;

    const float drawPosition = (float)audioPlayer.getTransportSource().getCurrentPosition();
    const float x = (drawPosition / audioLength) * (float)waveformBounds.getWidth() + (float)waveformBounds.getX();

    const float glowWidth = Config::Layout::Glow::thickness;
    const auto baseColor = Config::Colors::playbackText;

    // Create a horizontal gradient for the glow: Transparent -> Color -> Transparent
    juce::ColourGradient gradient(baseColor.withAlpha(0.0f), (float)x - glowWidth, 0.0f,
                                  baseColor.withAlpha(0.0f), (float)x + glowWidth, 0.0f, false);

    // Use a slightly higher alpha for the center to ensure visibility
    gradient.addColour(0.5, baseColor.withAlpha(0.6f));

    g.setGradientFill(gradient);
    // Draw the glow - using fillRect with gradient instead of drawLine
    g.fillRect((float)x - glowWidth, (float)waveformBounds.getY(), glowWidth * 2.0f, (float)waveformBounds.getHeight());

    // Draw the core 1-pixel vertical line
    g.setColour(baseColor);
    g.drawVerticalLine((int)x, (float)waveformBounds.getY(), (float)waveformBounds.getBottom());
}
