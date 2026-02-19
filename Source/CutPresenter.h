

#ifndef AUDIOFILER_CUTPRESENTER_H
#define AUDIOFILER_CUTPRESENTER_H

#include "SessionState.h"
#include "MouseHandler.h"

class CutLayerView;

class ControlPanel;

class CutPresenter : public SessionState::Listener
{
public:

    CutPresenter(ControlPanel& controlPanel, SessionState& sessionState, CutLayerView& cutLayerView);

    ~CutPresenter() override;

    MouseHandler& getMouseHandler() { return mouseHandler; }

    const MouseHandler& getMouseHandler() const { return mouseHandler; }

    void cutPreferenceChanged(const MainDomain::CutPreferences& prefs) override;

private:

    void refreshMarkersVisibility();

    SessionState& sessionState;
    CutLayerView& cutLayerView;
    MouseHandler mouseHandler;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CutPresenter)
};

#endif 
