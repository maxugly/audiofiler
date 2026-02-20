

#include "Presenters/CutButtonPresenter.h"

#include "UI/ControlPanel.h"
#include "Utils/Config.h"
#include "UI/MouseHandler.h"

CutButtonPresenter::CutButtonPresenter(ControlPanel& ownerPanel)
    : owner(ownerPanel)
{
}

void CutButtonPresenter::updateColours()
{
    const auto placementMode = owner.getPlacementMode();
    
    if (owner.inStrip != nullptr)
        owner.inStrip->getMarkerButton().setColour(juce::TextButton::buttonColourId,
            placementMode == AppEnums::PlacementMode::CutIn
                ? Config::Colors::Button::cutPlacement
                : Config::Colors::Button::cutActive);
    
    if (owner.outStrip != nullptr)
        owner.outStrip->getMarkerButton().setColour(juce::TextButton::buttonColourId,
            placementMode == AppEnums::PlacementMode::CutOut
                ? Config::Colors::Button::cutPlacement
                : Config::Colors::Button::cutActive);
    
    owner.refreshLabels();
}
