/**
 * @file RepeatButton.h
 * @brief Defines the RepeatButton class.
 * @ingroup Views
 */

#ifndef AUDIOFILER_REPEATBUTTON_H
#define AUDIOFILER_REPEATBUTTON_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

#include <functional>

/**
 * @class RepeatButton
 * @brief A custom button class designed to differentiate between left and right mouse clicks.
 */
class RepeatButton final : public juce::TextButton {
public:
    std::function<void()> onLeftClick;  
    std::function<void()> onRightClick; 

    /**
     * @brief Constructs a RepeatButton.
     * @param name The text to display on the button.
     */
    RepeatButton (const juce::String& name = {});

private:
    /**
     * @brief Overrides `mouseUp` to detect left vs. right clicks and trigger custom callbacks.
     * @param event The mouse event details.
     */
    void mouseUp (const juce::MouseEvent& event) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RepeatButton)
};

#endif 
