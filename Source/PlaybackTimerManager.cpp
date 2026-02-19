#include "PlaybackTimerManager.h"
#include "ControlPanel.h"
#include "AudioPlayer.h"
#include "WaveformManager.h"
#include "CoordinateMapper.h"
#include "PlaybackCursorView.h"
#include "ZoomView.h"
#include "MouseHandler.h"
#include "Config.h"
#include "FocusManager.h"

PlaybackTimerManager::PlaybackTimerManager(ControlPanel& ownerIn, 
                                           AudioPlayer& audioPlayerIn, 
                                           const ControlPanelLayoutCache& layoutCacheIn)
    : owner(ownerIn), audioPlayer(audioPlayerIn), layoutCache(layoutCacheIn)
{
    startTimerHz(60);
}

PlaybackTimerManager::~PlaybackTimerManager()
{
    stopTimer();
}

void PlaybackTimerManager::addListener(Listener* l)
{
    listeners.add(l);
}

void PlaybackTimerManager::removeListener(Listener* l)
{
    listeners.remove(l);
}

void PlaybackTimerManager::setViews(PlaybackCursorView* cursorViewIn, ZoomView* zoomViewIn)
{
    playbackCursorView = cursorViewIn;
    zoomView = zoomViewIn;
}

void PlaybackTimerManager::timerCallback()
{
    const bool isZDown = juce::KeyPress::isKeyCurrentlyDown('z') || juce::KeyPress::isKeyCurrentlyDown('Z');
    
    if (m_isZKeyDown != isZDown)
    {
        m_isZKeyDown = isZDown;
        owner.setZKeyDown(m_isZKeyDown);
    }

    owner.updateCutLabels();

    updateCursorPosition();
    
    if (zoomView != nullptr)
        updateZoomState();

    listeners.call(&Listener::playbackTimerTick);
}

void PlaybackTimerManager::updateCursorPosition()
{
    if (playbackCursorView != nullptr)
    {
        const double audioLength = audioPlayer.getWaveformManager().getThumbnail().getTotalLength();
        if (audioLength > 0.0)
        {
            const float x = CoordinateMapper::secondsToPixels(audioPlayer.getCurrentPosition(), 
                                                              (float)layoutCache.waveformBounds.getWidth(), 
                                                              audioLength);
            const int currentX = juce::roundToInt(x);

            if (currentX != lastCursorX)
            {
                if (lastCursorX >= 0)
                    playbackCursorView->repaint(lastCursorX - 1, 0, 3, playbackCursorView->getHeight());

                playbackCursorView->repaint(currentX - 1, 0, 3, playbackCursorView->getHeight());

                lastCursorX = currentX;
            }

            const bool zDown = isZKeyDown();
            const auto activePoint = owner.getActiveZoomPoint();
            const bool isZooming = zDown || activePoint != AppEnums::ActiveZoomPoint::None;

            if (isZooming && owner.getZoomPopupBounds().translated(-layoutCache.waveformBounds.getX(), -layoutCache.waveformBounds.getY()).contains(currentX, 10))
                playbackCursorView->setVisible(false);
            else
                playbackCursorView->setVisible(true);
        }
    }
}

void PlaybackTimerManager::updateZoomState()
{
    if (zoomView == nullptr) return;

    const auto& mouse = owner.getMouseHandler();
    const int currentMouseX = mouse.getMouseCursorX();
    const int currentMouseY = mouse.getMouseCursorY();

    const bool zDown = isZKeyDown();
    const auto activePoint = owner.getActiveZoomPoint();
    const bool isZooming = zDown || activePoint != AppEnums::ActiveZoomPoint::None;

    if (currentMouseX != lastMouseX || currentMouseY != lastMouseY)
    {
        if (lastMouseX != -1)
        {
            zoomView->repaint(lastMouseX - 1, 0, 3, zoomView->getHeight());
            zoomView->repaint(0, lastMouseY - 1, zoomView->getWidth(), 3);
        }

        if (currentMouseX != -1)
        {
            zoomView->repaint(currentMouseX - 1, 0, 3, zoomView->getHeight());
            zoomView->repaint(0, currentMouseY - 1, zoomView->getWidth(), 3);
        }

        lastMouseX = currentMouseX;
        lastMouseY = currentMouseY;
    }

    if (isZooming)
    {
        const auto waveformBounds = zoomView->getLocalBounds();
        const int popupWidth = juce::roundToInt((float)waveformBounds.getWidth() * Config::Layout::Zoom::popupScale);
        const int popupHeight = juce::roundToInt((float)waveformBounds.getHeight() * Config::Layout::Zoom::popupScale);
        const juce::Rectangle<int> currentPopupBounds(
            waveformBounds.getCentreX() - popupWidth / 2,
            waveformBounds.getCentreY() - popupHeight / 2,
            popupWidth,
            popupHeight
        );

        if (currentPopupBounds != lastPopupBounds)
        {
            zoomView->repaint(lastPopupBounds.expanded(5));
            zoomView->repaint(currentPopupBounds.expanded(5));
            lastPopupBounds = currentPopupBounds;
        }
        else
        {
            zoomView->repaint(currentPopupBounds.expanded(5));
        }
    }
    else if (!lastPopupBounds.isEmpty())
    {
        zoomView->repaint(lastPopupBounds.expanded(5));
        lastPopupBounds = juce::Rectangle<int>();
    }
}
