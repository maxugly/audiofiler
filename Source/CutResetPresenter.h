#ifndef AUDIOFILER_CUTRESETPRESENTER_H
#define AUDIOFILER_CUTRESETPRESENTER_H

class ControlPanel;

/**
 * @class CutResetPresenter
 * @brief Handles cut boundary reset behavior, keeping ControlPanel lean.
 */
class CutResetPresenter
{
public:
    explicit CutResetPresenter(ControlPanel& ownerPanel);

    void resetIn();
    void resetOut();

private:
    ControlPanel& owner;
};


#endif // AUDIOFILER_CUTRESETPRESENTER_H
