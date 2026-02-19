

#ifndef AUDIOFILER_PLAYBACKCURSORVIEW_H
#define AUDIOFILER_PLAYBACKCURSORVIEW_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

class ControlPanel;

class PlaybackCursorView : public juce::Component
{
public:

    explicit PlaybackCursorView(ControlPanel& owner);
    ~PlaybackCursorView() override = default;

    void paint(juce::Graphics& g) override;

private:
    ControlPanel& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaybackCursorView)
};

#endif 
