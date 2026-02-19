/**
 * @file CutButtonPresenter.h
 * @brief Defines the CutButtonPresenter class.
 * @ingroup Presenters
 */

#ifndef AUDIOFILER_CUTBUTTONPRESENTER_H
#define AUDIOFILER_CUTBUTTONPRESENTER_H

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
 * @class CutButtonPresenter
 * @brief Handles the color state of the cut in/out buttons.
 */
class CutButtonPresenter
{
public:
    /**
     * @brief Undocumented method.
     * @param ownerPanel [in] Description for ownerPanel.
     */
    explicit CutButtonPresenter(ControlPanel& ownerPanel);

    /**
     * @brief Undocumented method.
     */
    void updateColours();

private:
    ControlPanel& owner;
};


#endif // AUDIOFILER_CUTBUTTONPRESENTER_H
