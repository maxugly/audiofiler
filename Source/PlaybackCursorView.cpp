

#include "PlaybackCursorView.h"
#include "ControlPanel.h"
#include "PlaybackCursorGlow.h"
#include "Config.h"
#include "CoordinateMapper.h"

PlaybackCursorView::PlaybackCursorView(ControlPanel& ownerPanel)
    : owner(ownerPanel)
{

    setInterceptsMouseClicks(false, false);

    setOpaque(false);
}

void PlaybackCursorView::paint(juce::Graphics& g)
{
    auto& audioPlayer = owner.getAudioPlayer();
    const double audioLength = audioPlayer.getWaveformManager().getThumbnail().getTotalLength();
    if (audioLength <= 0.0)
        return;

    const auto waveformBounds = getLocalBounds();
    const double drawPosition = audioPlayer.getCurrentPosition();
    const float x = CoordinateMapper::secondsToPixels(drawPosition, (float)waveformBounds.getWidth(), audioLength);

    g.setColour(Config::Colors::playbackText);
    g.drawVerticalLine(juce::roundToInt(x), 0.0f, (float)getHeight());
}
