/**
 * @file CutResetPresenter.h
 * @brief Defines the CutResetPresenter class.
 * @ingroup Presenters
 */

#ifndef AUDIOFILER_CUTRESETPRESENTER_H
#define AUDIOFILER_CUTRESETPRESENTER_H

/**
 * @class ControlPanel
 * @brief Home: View.
 *
 */
class ControlPanel;

/**
 * @class CutResetPresenter
 * @brief Handles cut boundary reset behavior, keeping ControlPanel lean.
 */
class CutResetPresenter
{
public:
    /**
     * @brief Undocumented method.
     * @param ownerPanel [in] Description for ownerPanel.
     */
    explicit CutResetPresenter(ControlPanel& ownerPanel);

    /**
     * @brief Undocumented method.
     */
    void resetIn();
    /**
     * @brief Undocumented method.
     */
    void resetOut();

private:
    ControlPanel& owner;
};


#endif 
