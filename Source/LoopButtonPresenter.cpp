#include "LoopButtonPresenter.h"

#include "ControlPanel.h"
#include "Config.h"
#include "MouseHandler.h"

LoopButtonPresenter::LoopButtonPresenter(ControlPanel& ownerPanel)
    : owner(ownerPanel)
{
}

void LoopButtonPresenter::updateColours()
{
    const auto placementMode = owner.getMouseHandler().getCurrentPlacementMode();
    owner.cutInSetButton.setColour(juce::TextButton::buttonColourId,
        placementMode == AppEnums::PlacementMode::LoopIn
            ? Config::Colors::Button::loopPlacement
            : Config::Colors::Button::loopActive);
    owner.cutOutSetButton.setColour(juce::TextButton::buttonColourId,
        placementMode == AppEnums::PlacementMode::LoopOut
            ? Config::Colors::Button::loopPlacement
            : Config::Colors::Button::loopActive);
    owner.updateCutLabels();
}

