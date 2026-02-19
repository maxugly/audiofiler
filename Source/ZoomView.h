

#ifndef AUDIOFILER_ZOOMVIEW_H
#define AUDIOFILER_ZOOMVIEW_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

#include "AppEnums.h"

class ControlPanel;

class ZoomView : public juce::Component
{
public:

    explicit ZoomView(ControlPanel& owner);
    ~ZoomView() override = default;

    void paint(juce::Graphics& g) override;

private:
    ControlPanel& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZoomView)
};

#endif 
