#pragma once

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
    void initialiseLoopButton();
    void initialiseAutoplayButton();
    void initialiseAutoCutInButton();
    void initialiseAutoCutOutButton();
    void initialiseCutButton();
    void initialiseLoopButtons();
    void initialiseClearButtons();

    ControlPanel& owner;
};

