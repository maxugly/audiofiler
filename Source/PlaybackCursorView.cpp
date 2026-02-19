/**
 * @file PlaybackCursorView.cpp
 * @brief Defines the PlaybackCursorView class.
 * @ingroup Views
 */

#include "PlaybackCursorView.h"
#include "ControlPanel.h"
#include "PlaybackCursorGlow.h"
#include "Config.h"
#include "CoordinateMapper.h"

PlaybackCursorView::PlaybackCursorView(ControlPanel& ownerPanel)
    : owner(ownerPanel)
{
    /**
     * @brief Sets the InterceptsMouseClicks.
     * @param false [in] Description for false.
     * @param false [in] Description for false.
     */
    setInterceptsMouseClicks(false, false);
    /**
     * @brief Sets the Opaque.
     * @param false [in] Description for false.
     */
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
