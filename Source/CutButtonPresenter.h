

#ifndef AUDIOFILER_CUTBUTTONPRESENTER_H
#define AUDIOFILER_CUTBUTTONPRESENTER_H

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
#else
    #include <JuceHeader.h>
#endif

class ControlPanel;

class CutButtonPresenter
{
public:

    explicit CutButtonPresenter(ControlPanel& ownerPanel);

    void updateColours();

private:
    ControlPanel& owner;
};

#endif 
