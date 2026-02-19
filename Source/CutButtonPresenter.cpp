

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
    owner.cutInButton.setColour(juce::TextButton::buttonColourId,
        placementMode == AppEnums::PlacementMode::CutIn
            ? Config::Colors::Button::cutPlacement
            : Config::Colors::Button::cutActive);
    owner.cutOutButton.setColour(juce::TextButton::buttonColourId,
        placementMode == AppEnums::PlacementMode::CutOut
            ? Config::Colors::Button::cutPlacement
            : Config::Colors::Button::cutActive);
    owner.updateCutLabels();
}
