

#include "PlaybackCursorView.h"
#include "ControlPanel.h"
#include "PlaybackCursorGlow.h"
#include "Config.h"
#include "CoordinateMapper.h"
#include "AudioPlayer.h"
#include "WaveformManager.h"

PlaybackCursorView::PlaybackCursorView(ControlPanel& ownerPanel)
    : owner(ownerPanel)
{

    setInterceptsMouseClicks(false, false);

    setOpaque(false);
}

PlaybackCursorView::~PlaybackCursorView()
{
    owner.getPlaybackTimerManager().removeListener(this);
}

void PlaybackCursorView::playbackTimerTick()
{
    const auto& audioPlayer = owner.getAudioPlayer();
    const double audioLength = audioPlayer.getWaveformManager().getThumbnail().getTotalLength();
    if (audioLength > 0.0)
    {
        const auto& layout = owner.getWaveformBounds();
        const float x = CoordinateMapper::secondsToPixels(audioPlayer.getCurrentPosition(), 
                                                          (float)layout.getWidth(), 
                                                          audioLength);
        const int currentX = juce::roundToInt(x);

        if (currentX != lastCursorX)
        {
            if (lastCursorX >= 0)
                repaint(lastCursorX - 1, 0, 3, getHeight());

            repaint(currentX - 1, 0, 3, getHeight());

            lastCursorX = currentX;
        }

        const auto& timerManager = owner.getPlaybackTimerManager();
        const bool zDown = timerManager.isZKeyDown();
        const auto activePoint = owner.getActiveZoomPoint();
        const bool isZooming = zDown || activePoint != AppEnums::ActiveZoomPoint::None;

        if (isZooming && owner.getZoomPopupBounds().translated(-layout.getX(), -layout.getY()).contains(currentX, 10))
            setVisible(false);
        else
            setVisible(true);
    }
}

void PlaybackCursorView::animationUpdate(float breathingPulse)
{
    juce::ignoreUnused(breathingPulse);
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
