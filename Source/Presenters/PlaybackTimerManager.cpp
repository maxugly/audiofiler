#include "Presenters/PlaybackTimerManager.h"
#include "Core/SessionState.h"
#include "Core/AudioPlayer.h"
#include "Utils/UIAnimationHelper.h"
#include "Utils/Config.h"
#include "Presenters/PlaybackRepeatController.h"

PlaybackTimerManager::PlaybackTimerManager(SessionState& sessionStateIn, AudioPlayer& audioPlayerIn)
    : sessionState(sessionStateIn), audioPlayer(audioPlayerIn)
{
    startTimerHz(60);
}
PlaybackTimerManager::~PlaybackTimerManager()
{
    stopTimer();
}

void PlaybackTimerManager::addListener(Listener* l)
{
    const juce::ScopedLock lock(listenerLock);
    listeners.add(l);
}

void PlaybackTimerManager::removeListener(Listener* l)
{
    const juce::ScopedLock lock(listenerLock);
    listeners.remove(l);
}

void PlaybackTimerManager::setManualZoomPoint(AppEnums::ActiveZoomPoint point)
{
    if (m_manualZoomPoint != point)
    {
        m_manualZoomPoint = point;
        // If we are not overriding with Z, update the active point immediately
        if (!m_isZKeyDown)
        {
            if (m_activeZoomPoint != m_manualZoomPoint)
            {
                m_activeZoomPoint = m_manualZoomPoint;
                listeners.call(&Listener::activeZoomPointChanged, m_activeZoomPoint);
            }
        }
    }
}

void PlaybackTimerManager::timerCallback()
{
    const bool isZDown = juce::KeyPress::isKeyCurrentlyDown('z') || juce::KeyPress::isKeyCurrentlyDown('Z');
    
    const auto lastActivePoint = m_activeZoomPoint;

    if (isZDown != m_isZKeyDown)
    {
        m_isZKeyDown = isZDown;
        if (m_isZKeyDown)
        {
            if (m_zoomPointProvider)
                m_activeZoomPoint = m_zoomPointProvider();
        }
        else
        {
            m_activeZoomPoint = m_manualZoomPoint;
        }
    }

    if (m_activeZoomPoint != lastActivePoint)
        listeners.call(&Listener::activeZoomPointChanged, m_activeZoomPoint);

    if (m_repeatController != nullptr)
        m_repeatController->tick();

    // Update master animation clock - 4 second cycle
    m_masterPhase += (1.0f / (60.0f * 4.0f));
    if (m_masterPhase >= 1.0f)
        m_masterPhase = 0.0f;

    // Calculate breathing pulse at 1Hz (multiplier = 4.0f since cycle is 4s)
    m_breathingPulse = UIAnimationHelper::getSinePulse(m_masterPhase, 4.0f);

    // Notify all high-frequency listeners
    const juce::ScopedLock lock(listenerLock);
    listeners.call(&Listener::playbackTimerTick);
    listeners.call(&Listener::animationUpdate, m_breathingPulse);
}
