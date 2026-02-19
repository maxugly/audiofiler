/**
 * @file CutPresenter.h
 * @brief Defines the CutPresenter class.
 * @ingroup Presenters
 */

#ifndef AUDIOFILER_CUTPRESENTER_H
#define AUDIOFILER_CUTPRESENTER_H

#include "SessionState.h"
#include "MouseHandler.h"

/**
 * @class CutLayerView
 * @brief Home: View.
 *
 */
class CutLayerView;
/**
 * @class ControlPanel
 * @brief Home: View.
 *
 */
class ControlPanel;

/**
 * @class CutPresenter
 * @brief Home: Presenter.
 *
 * @see CutLayerView
 */
class CutPresenter : public SessionState::Listener
{
public:
    /**
     * @brief Undocumented method.
     * @param controlPanel [in] Description for controlPanel.
     * @param sessionState [in] Description for sessionState.
     * @param cutLayerView [in] Description for cutLayerView.
     */
    CutPresenter(ControlPanel& controlPanel, SessionState& sessionState, CutLayerView& cutLayerView);
    /**
     * @brief Undocumented method.
     */
    ~CutPresenter() override;

    /**
     * @brief Gets the MouseHandler.
     * @return MouseHandler&
     */
    MouseHandler& getMouseHandler() { return mouseHandler; }
    /**
     * @brief Gets the MouseHandler.
     * @return const MouseHandler&
     */
    const MouseHandler& getMouseHandler() const { return mouseHandler; }

    /**
     * @brief Undocumented method.
     * @param prefs [in] Description for prefs.
     */
    void cutPreferenceChanged(const MainDomain::CutPreferences& prefs) override;

private:
    /**
     * @brief Undocumented method.
     */
    void refreshMarkersVisibility();

    SessionState& sessionState;
    CutLayerView& cutLayerView;
    MouseHandler mouseHandler;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CutPresenter)
};

#endif // AUDIOFILER_CUTPRESENTER_H
