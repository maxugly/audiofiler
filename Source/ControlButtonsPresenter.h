

#ifndef AUDIOFILER_CONTROLBUTTONSPRESENTER_H
#define AUDIOFILER_CONTROLBUTTONSPRESENTER_H

class ControlPanel;

class ControlButtonsPresenter final
{
public:

    explicit ControlButtonsPresenter(ControlPanel& ownerPanel);

    void initialiseAllButtons();

private:

    void initialiseOpenButton();

    void initialisePlayStopButton();

    void initialiseModeButton();

    void initialiseChannelViewButton();

    void initialiseQualityButton();

    void initialiseExitButton();

    void initialiseStatsButton();

    void initialiseRepeatButton();

    void initialiseAutoplayButton();

    void initialiseAutoCutInButton();

    void initialiseAutoCutOutButton();

    void initialiseCutButton();

    void initialiseCutBoundaryButtons();

    void initialiseClearButtons();

    ControlPanel& owner;
};

#endif 
