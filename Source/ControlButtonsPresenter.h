#ifndef AUDIOFILER_CONTROLBUTTONSPRESENTER_H
#define AUDIOFILER_CONTROLBUTTONSPRESENTER_H

class ControlPanel;

/**
 * @class ControlButtonsPresenter
 * @brief Encapsulates all ControlPanel button creation and wiring.
 *
 * Extracting button setup keeps ControlPanel focused on orchestration while this helper
 * handles addAndMakeVisible, text, and onClick lambdas for every button.
 */
class ControlButtonsPresenter final
{
public:
    explicit ControlButtonsPresenter(ControlPanel& ownerPanel);

    /**
     * @brief Initializes every button owned by ControlPanel.
     */
    void initialiseAllButtons();

private:
    void initialiseOpenButton();
    void initialisePlayStopButton();
    void initialiseModeButton();
    void initialiseChannelViewButton();
    void initialiseQualityButton();
    void initialiseExitButton();
    void initialiseStatsButton();
    void initialiseCutModeButton();
    void initialiseAutoplayButton();
    void initialiseAutoCutInButton();
    void initialiseAutoCutOutButton();
    void initialiseCutButton();
    void initialiseCutModeButtons();
    void initialiseClearButtons();

    ControlPanel& owner;
};


#endif // AUDIOFILER_CONTROLBUTTONSPRESENTER_H
