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
    owner.loopInButton.setColour(juce::TextButton::buttonColourId,
        placementMode == AppEnums::PlacementMode::LoopIn
            ? Config::Colors::buttonLoopPlacement
            : Config::Colors::buttonLoopActive);
    owner.loopOutButton.setColour(juce::TextButton::buttonColourId,
        placementMode == AppEnums::PlacementMode::LoopOut
            ? Config::Colors::buttonLoopPlacement
            : Config::Colors::buttonLoopActive);
    owner.updateLoopLabels();
}

