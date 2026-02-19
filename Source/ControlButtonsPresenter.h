/**
 * @file ControlButtonsPresenter.h
 * @brief Defines the ControlButtonsPresenter class.
 * @ingroup Presenters
 */

#ifndef AUDIOFILER_CONTROLBUTTONSPRESENTER_H
#define AUDIOFILER_CONTROLBUTTONSPRESENTER_H

/**
 * @class ControlPanel
 * @brief Home: View.
 *
 */
class ControlPanel;

/**
 * @class ControlButtonsPresenter
 * @brief Encapsulates all ControlPanel button creation and wiring.
 */
class ControlButtonsPresenter final
{
public:
    /**
     * @brief Undocumented method.
     * @param ownerPanel [in] Description for ownerPanel.
     */
    explicit ControlButtonsPresenter(ControlPanel& ownerPanel);

    /**
     * @brief Undocumented method.
     */
    void initialiseAllButtons();

private:
    /**
     * @brief Undocumented method.
     */
    void initialiseOpenButton();
    /**
     * @brief Undocumented method.
     */
    void initialisePlayStopButton();
    /**
     * @brief Undocumented method.
     */
    void initialiseModeButton();
    /**
     * @brief Undocumented method.
     */
    void initialiseChannelViewButton();
    /**
     * @brief Undocumented method.
     */
    void initialiseQualityButton();
    /**
     * @brief Undocumented method.
     */
    void initialiseExitButton();
    /**
     * @brief Undocumented method.
     */
    void initialiseStatsButton();
    /**
     * @brief Undocumented method.
     */
    void initialiseRepeatButton();
    /**
     * @brief Undocumented method.
     */
    void initialiseAutoplayButton();
    /**
     * @brief Undocumented method.
     */
    void initialiseAutoCutInButton();
    /**
     * @brief Undocumented method.
     */
    void initialiseAutoCutOutButton();
    /**
     * @brief Undocumented method.
     */
    void initialiseCutButton();
    /**
     * @brief Undocumented method.
     */
    void initialiseCutBoundaryButtons();
    /**
     * @brief Undocumented method.
     */
    void initialiseClearButtons();

    ControlPanel& owner;
};


#endif // AUDIOFILER_CONTROLBUTTONSPRESENTER_H
