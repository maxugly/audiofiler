

#include "CutPresenter.h"
#include "CutLayerView.h"
#include "ControlPanel.h"

CutPresenter::CutPresenter(ControlPanel& controlPanel, SessionState& sessionStateIn, CutLayerView& cutLayerViewIn)
    : sessionState(sessionStateIn),
      cutLayerView(cutLayerViewIn),
      mouseHandler(controlPanel)
{
    sessionState.addListener(this);

    refreshMarkersVisibility();
}

CutPresenter::~CutPresenter()
{
    sessionState.removeListener(this);
}

void CutPresenter::cutPreferenceChanged(const MainDomain::CutPreferences&)
{

    refreshMarkersVisibility();
    cutLayerView.repaint();
}

void CutPresenter::refreshMarkersVisibility()
{
    const auto prefs = sessionState.getCutPrefs();
    cutLayerView.setMarkersVisible(prefs.active);
}
