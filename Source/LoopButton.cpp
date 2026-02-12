#include "LoopButton.h"

LoopButton::LoopButton (const juce::String& name) : juce::TextButton (name)
{
}

void LoopButton::mouseUp (const juce::MouseEvent& event)
{
    if (isEnabled()) {
        if (event.mods.isRightButtonDown()) {
            if (onRightClick) onRightClick();
        } else if (event.mods.isLeftButtonDown()) {
            if (onLeftClick) onLeftClick();
        }
    }
    juce::TextButton::mouseUp(event); // Call base class method
}
