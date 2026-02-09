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
    glowGradient.addColour(0.0, Config::playbackCursorGlowColorStart);
    glowGradient.addColour(0.5, Config::playbackCursorGlowColorEnd);
    glowGradient.addColour(1.0, Config::playbackCursorColor.withAlpha(0.0f));
    glowGradient.point1 = { x - 5.0f, (float) waveformBounds.getCentreY() };
    glowGradient.point2 = { x + 5.0f, (float) waveformBounds.getCentreY() };
    g.setGradientFill(glowGradient);

    g.drawLine((float) x, (float) waveformBounds.getY(), (float) x, (float) waveformBounds.getBottom(), 2.0f);
}

