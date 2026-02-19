/**
 * @file ControlStatePresenter.h
 * @brief Defines the ControlStatePresenter class.
 * @ingroup Presenters
 */

#ifndef AUDIOFILER_CONTROLSTATEPRESENTER_H
#define AUDIOFILER_CONTROLSTATEPRESENTER_H

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
#else
    #include <JuceHeader.h>
#endif

/**
 * @class ControlPanel
 * @brief Home: View.
 *
 */
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
    /**
     * @brief Undocumented method.
     * @param enabled [in] Description for enabled.
     */
    void updateGeneralButtonStates(bool enabled);
    /**
     * @brief Undocumented method.
     * @param isCutModeActive [in] Description for isCutModeActive.
     * @param enabled [in] Description for enabled.
     */
    void updateCutModeControlStates(bool isCutModeActive, bool enabled);

    ControlPanel& owner;
};

#endif // AUDIOFILER_CONTROLSTATEPRESENTER_H
