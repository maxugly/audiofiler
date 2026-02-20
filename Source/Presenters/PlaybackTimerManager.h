#ifndef AUDIOFILER_PLAYBACKTIMERMANAGER_H
#define AUDIOFILER_PLAYBACKTIMERMANAGER_H

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
    #include <juce_events/juce_events.h>
#else
    #include <JuceHeader.h>
#endif

#include "Core/AppEnums.h"
#include <functional>

class SessionState;
class AudioPlayer;
class PlaybackRepeatController;

/**
 * @class PlaybackTimerManager
 * @brief A domain-level utility that manages high-frequency (60Hz) UI heartbeats.
 * 
 * This manager evacuates high-frequency polling from the UI layer. It monitors 
 * playback progress and keyboard state, notifying registered listeners at 60Hz.
 * 
 * @ingroup Logic
 */
class PlaybackTimerManager final : public juce::Timer
{
public:
    /**
     * @class Listener
     * @brief Interface for components that need high-frequency updates.
     */
    class Listener
    {
    public:
        virtual ~Listener() = default;

        /** @brief Called at a 60Hz frequency to trigger UI updates. */
        virtual void playbackTimerTick() = 0;

        /** @brief Called at a 60Hz frequency to broadcast the master pulse. */
        virtual void animationUpdate (float breathingPulse) = 0;

        /** @brief Called when the active zoom point changes (e.g., via 'Z' key). */
        virtual void activeZoomPointChanged(AppEnums::ActiveZoomPoint newPoint) { juce::ignoreUnused(newPoint); }
    };

    /**
     * @brief Constructor.
     * @param sessionStateIn Reference to the central application state.
     * @param audioPlayerIn Reference to the audio engine.
     */
    PlaybackTimerManager(SessionState& sessionStateIn, AudioPlayer& audioPlayerIn);
    
    /** @brief Destructor. stops the timer. */
    ~PlaybackTimerManager() override;

    /** @brief Sets the repeat controller to be ticked by this manager. */
    void setRepeatController(PlaybackRepeatController* controller) { m_repeatController = controller; }

    /** @brief Sets the provider for the active zoom point. */
    void setZoomPointProvider(std::function<AppEnums::ActiveZoomPoint()> provider) { m_zoomPointProvider = std::move(provider); }

    /** @brief Sets a manual override for the zoom point (e.g. from mouse hover). */
    void setManualZoomPoint(AppEnums::ActiveZoomPoint point);

    /** @brief Returns the current active zoom point. */
    AppEnums::ActiveZoomPoint getActiveZoomPoint() const { return m_activeZoomPoint; }

    /** @brief Registers a listener for timer ticks. */
    void addListener(Listener* l);

    /** @brief Unregisters a listener. */
    void removeListener(Listener* l);

    /** @brief Returns true if the 'z' key is currently held down. */
    bool isZKeyDown() const { return m_isZKeyDown; }

    /** @brief Returns the master animation phase (0.0 to 1.0). */
    float getMasterPhase() const { return m_masterPhase; }

    /** @brief Returns the breathing pulse value (0.0 to 1.0). */
    float getBreathingPulse() const { return m_breathingPulse; }

    /** @brief Internal timer callback. */
    void timerCallback() override;

private:
    SessionState& sessionState;
    AudioPlayer& audioPlayer;
    PlaybackRepeatController* m_repeatController = nullptr;
    std::function<AppEnums::ActiveZoomPoint()> m_zoomPointProvider;
    
    juce::ListenerList<Listener> listeners;
    juce::CriticalSection listenerLock;
    
    bool m_isZKeyDown = false;
    bool m_wasZKeyDown = false;
    AppEnums::ActiveZoomPoint m_activeZoomPoint = AppEnums::ActiveZoomPoint::None;
    AppEnums::ActiveZoomPoint m_manualZoomPoint = AppEnums::ActiveZoomPoint::None;
    float m_masterPhase = 0.0f;
    float m_breathingPulse = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaybackTimerManager)
};

#endif 
