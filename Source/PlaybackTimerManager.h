#ifndef AUDIOFILER_PLAYBACKTIMERMANAGER_H
#define AUDIOFILER_PLAYBACKTIMERMANAGER_H

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
    #include <juce_events/juce_events.h>
#else
    #include <JuceHeader.h>
#endif

#include "AppEnums.h"

class AudioPlayer;
struct ControlPanelLayoutCache;
class PlaybackCursorView;
class ZoomView;
class ControlPanel;

/**
 * @class PlaybackTimerManager
 * @brief Manages high-frequency (60Hz) updates for playback-related UI elements.
 * 
 * This manager handles the logic for updating the playback cursor position,
 * managing the zoom state based on keyboard input, and notifying listeners
 * of timer ticks.
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
        virtual void playbackTimerTick() = 0;
    };

    PlaybackTimerManager(ControlPanel& ownerIn, 
                         AudioPlayer& audioPlayerIn, 
                         const ControlPanelLayoutCache& layoutCacheIn);
    
    ~PlaybackTimerManager() override;

    void addListener(Listener* l);
    void removeListener(Listener* l);

    void setViews(PlaybackCursorView* cursorViewIn, ZoomView* zoomViewIn);

    void timerCallback() override;

    bool isZKeyDown() const { return m_isZKeyDown; }

    void updateCursorPosition();
    void updateZoomState();

private:
    ControlPanel& owner;
    AudioPlayer& audioPlayer;
    const ControlPanelLayoutCache& layoutCache;
    
    PlaybackCursorView* playbackCursorView = nullptr;
    ZoomView* zoomView = nullptr;
    
    juce::ListenerList<Listener> listeners;
    
    bool m_isZKeyDown = false;
    int lastCursorX = -1;
    int lastMouseX = -1;
    int lastMouseY = -1;
    juce::Rectangle<int> lastPopupBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaybackTimerManager)
};

#endif 
