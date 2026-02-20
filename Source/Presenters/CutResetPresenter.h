

#ifndef AUDIOFILER_CUTRESETPRESENTER_H
#define AUDIOFILER_CUTRESETPRESENTER_H

class ControlPanel;

class CutResetPresenter
{
public:

    explicit CutResetPresenter(ControlPanel& ownerPanel);

    void resetIn();

    void resetOut();

private:
    ControlPanel& owner;
};

#endif 
