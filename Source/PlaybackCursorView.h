

#ifndef AUDIOFILER_PLAYBACKCURSORVIEW_H
#define AUDIOFILER_PLAYBACKCURSORVIEW_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

#include "PlaybackTimerManager.h"

class ControlPanel;

class PlaybackCursorView : public juce::Component,
                           public PlaybackTimerManager::Listener
{
public:

    explicit PlaybackCursorView(ControlPanel& owner);
    ~PlaybackCursorView() override;

    void paint(juce::Graphics& g) override;

    void playbackTimerTick() override;
    void animationUpdate (float breathingPulse) override;

private:
    ControlPanel& owner;
    int lastCursorX = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaybackCursorView)
};

#endif 
