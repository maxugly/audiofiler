

#ifndef AUDIOFILER_REPEATBUTTON_H
#define AUDIOFILER_REPEATBUTTON_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

#include <functional>

class RepeatButton final : public juce::TextButton {
public:
    std::function<void()> onLeftClick;  
    std::function<void()> onRightClick; 

    RepeatButton (const juce::String& name = {});

private:

    void mouseUp (const juce::MouseEvent& event) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RepeatButton)
};

#endif 
