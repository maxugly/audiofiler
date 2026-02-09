#pragma once

#include <JuceHeader.h>

class ControlPanel;

/**
 * @class ControlStatePresenter
 * @brief Centralises the enable/disable and visibility rules for every ControlPanel widget.
 *
 * ControlPanel delegates to this helper so the button state logic lives outside the main
 * component class, making it easier to tweak availability rules without bloating the panel.
 */
class ControlStatePresenter final
{
public:
    /**
     * @brief Constructs the presenter bound to a specific ControlPanel.
     * @param ownerPanel Reference to the panel whose controls will be updated.
     */
    explicit ControlStatePresenter(ControlPanel& ownerPanel);

    /**
     * @brief Applies the latest enable/visibility rules to every relevant control.
     */
    void refreshStates();

private:
    void updateGeneralButtonStates(bool enabled);
    void updateCutModeControlStates(bool isCutModeActive, bool enabled);

    ControlPanel& owner;
};
