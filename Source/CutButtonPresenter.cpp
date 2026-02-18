#include "CutButtonPresenter.h"

#include "ControlPanel.h"
#include "Config.h"
#include "MouseHandler.h"

CutButtonPresenter::CutButtonPresenter(ControlPanel& ownerPanel)
    : owner(ownerPanel)
{
}

void CutButtonPresenter::updateColours()
{
    const auto placementMode = owner.getMouseHandler().getCurrentPlacementMode();
    owner.loopInButton.setColour(juce::TextButton::buttonColourId,
        placementMode == AppEnums::PlacementMode::LoopIn
            ? Config::Colors::Button::loopPlacement
            : Config::Colors::Button::loopActive);
    owner.loopOutButton.setColour(juce::TextButton::buttonColourId,
        placementMode == AppEnums::PlacementMode::LoopOut
            ? Config::Colors::Button::loopPlacement
            : Config::Colors::Button::loopActive);
    owner.updateCutLabels();
}

