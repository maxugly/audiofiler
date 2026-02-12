#pragma once

#include <JuceHeader.h>
#include <functional>

/**
 * @class LoopButton
 * @brief A custom button class designed to differentiate between left and right mouse clicks.
 *
 * This button is specifically used for the "Loop In" and "Loop Out" functionalities,
 * allowing a left-click to directly set a point and a right-click to enter a
 * placement mode for more precise interaction. It exposes `onLeftClick` and
 * `onRightClick` function objects for flexible callback assignment.
 */
class LoopButton : public juce::TextButton {
public:
    std::function<void()> onLeftClick;  ///< Function to call when the left mouse button is released over the button.
    std::function<void()> onRightClick; ///< Function to call when the right mouse button is released over the button.

    /**
     * @brief Constructs a LoopButton.
     * @param name The text to display on the button.
     */
    LoopButton (const juce::String& name = {});

private:
    /**
     * @brief Overrides `mouseUp` to detect left vs. right clicks and trigger custom callbacks.
     * @param event The mouse event details.
     */
    void mouseUp (const juce::MouseEvent& event) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoopButton)
};
