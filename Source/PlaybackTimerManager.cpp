#include "PlaybackTimerManager.h"
#include "SessionState.h"
#include "AudioPlayer.h"
#include "UIAnimationHelper.h"
#include "Config.h"

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

void PlaybackTimerManager::timerCallback()
{
    const bool isZDown = juce::KeyPress::isKeyCurrentlyDown('z') || juce::KeyPress::isKeyCurrentlyDown('Z');
    m_isZKeyDown = isZDown;

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
