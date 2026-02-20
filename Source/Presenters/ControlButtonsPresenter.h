

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

    void initialiseStopButton();

    void initialiseModeButton();

    void initialiseChannelViewButton();

    void initialiseExitButton();
void initialiseStatsButton();
void initialiseEyeCandyButton();

private:
    ControlPanel& owner;
};

#endif 
