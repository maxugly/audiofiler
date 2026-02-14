#include "PlaybackCursorGlow.h"

#include "ControlPanel.h"
#include "Config.h"
#include "ControlPanelLayoutCache.h"

void PlaybackCursorGlow::renderGlow(juce::Graphics& g, const ControlPanel& controlPanel, const juce::Rectangle<int>& waveformBounds)
{
    const int x = static_cast<int>(controlPanel.getAudioPlayer().getTransportSource().getCurrentPosition()
        / controlPanel.getAudioPlayer().getThumbnail().getTotalLength()
        * waveformBounds.getWidth()) + waveformBounds.getX();

    juce::ColourGradient glowGradient;
    glowGradient.addColour(0.0, Config::Colors::playbackCursorGlowStart);
    glowGradient.addColour(0.5, Config::Colors::playbackCursorGlowEnd);
    glowGradient.addColour(1.0, Config::Colors::playbackCursor.withAlpha(0.0f));
    glowGradient.point1 = { x - Config::Layout::Glow::cursorGlowRadius, (float) waveformBounds.getCentreY() };
    glowGradient.point2 = { x + Config::Layout::Glow::cursorGlowRadius, (float) waveformBounds.getCentreY() };
    g.setGradientFill(glowGradient);

    g.drawLine((float) x, (float) waveformBounds.getY(), (float) x, (float) waveformBounds.getBottom(), Config::Layout::Glow::cursorGlowLineThickness);
}

