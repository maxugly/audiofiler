#ifndef AUDIOFILER_CUTRESETPRESENTER_H
#define AUDIOFILER_CUTRESETPRESENTER_H

class ControlPanel;

/**
 * @class CutResetPresenter
 * @brief Handles clear-loop button behavior, keeping ControlPanel lean.
 */
class CutResetPresenter
{
public:
    explicit CutResetPresenter(ControlPanel& ownerPanel);

    void clearLoopIn();
    void clearLoopOut();

private:
    ControlPanel& owner;
};


#endif // AUDIOFILER_CUTRESETPRESENTER_H
