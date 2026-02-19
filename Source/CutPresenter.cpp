/**
 * @file CutPresenter.cpp
 * @brief Defines the CutPresenter class.
 * @ingroup Presenters
 */

#include "CutPresenter.h"
#include "CutLayerView.h"
#include "ControlPanel.h"

CutPresenter::CutPresenter(ControlPanel& controlPanel, SessionState& sessionStateIn, CutLayerView& cutLayerViewIn)
    : sessionState(sessionStateIn),
      cutLayerView(cutLayerViewIn),
      mouseHandler(controlPanel)
{
    sessionState.addListener(this);
    /**
     * @brief Undocumented method.
     */
    refreshMarkersVisibility();
}

CutPresenter::~CutPresenter()
{
    sessionState.removeListener(this);
}

void CutPresenter::cutPreferenceChanged(const MainDomain::CutPreferences&)
{
    /**
     * @brief Undocumented method.
     */
    refreshMarkersVisibility();
    cutLayerView.repaint();
}

void CutPresenter::refreshMarkersVisibility()
{
    const auto prefs = sessionState.getCutPrefs();
    cutLayerView.setMarkersVisible(prefs.active);
}
