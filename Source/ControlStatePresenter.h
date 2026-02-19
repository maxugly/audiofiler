

#ifndef AUDIOFILER_CONTROLSTATEPRESENTER_H
#define AUDIOFILER_CONTROLSTATEPRESENTER_H

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
#else
    #include <JuceHeader.h>
#endif

class ControlPanel;

class ControlStatePresenter final
{
public:

    explicit ControlStatePresenter(ControlPanel& ownerPanel);

    void refreshStates();

private:

    void updateGeneralButtonStates(bool enabled);

    void updateCutModeControlStates(bool isCutModeActive, bool enabled);

    ControlPanel& owner;
};

#endif 
