#include "PlaybackOverlay.h"
#include "ControlPanel.h"
#include "PlaybackCursorGlow.h"
#include "Config.h"

PlaybackOverlay::PlaybackOverlay(ControlPanel& ownerPanel)
    : owner(ownerPanel)
{
}

void PlaybackOverlay::paint(juce::Graphics& g)
{
    // Translate the coordinate system so that (0,0) in logic (ControlPanel coordinates)
    // maps to (0,0) in this component, relative to its position.
    // Since PlaybackOverlay is positioned at (waveformBounds.x, waveformBounds.y),
    // and WaveformRenderer draws at (waveformBounds.x, waveformBounds.y),
    // we need to translate by -x, -y.

    g.setOrigin(-getX(), -getY());

    owner.renderOverlays(g);

    auto& audioPlayer = owner.getAudioPlayer();
    const double audioLength = audioPlayer.getWaveformManager().getThumbnail().getTotalLength();
    if (audioLength <= 0.0)
        return;

    const auto waveformBounds = owner.getWaveformBounds();
    const float drawPosition = (float)audioPlayer.getTransportSource().getCurrentPosition();
    const float x = (drawPosition / (float)audioLength) * (float)waveformBounds.getWidth() + (float)waveformBounds.getX();

    PlaybackCursorGlow::renderGlow(g, (int)x, waveformBounds.getY(), waveformBounds.getBottom(), Config::Colors::playbackText);
}
