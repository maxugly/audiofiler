

#ifndef AUDIOFILER_TRANSPORTPRESENTER_H
#define AUDIOFILER_TRANSPORTPRESENTER_H

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
#else
    #include <JuceHeader.h>
#endif

class ControlPanel;

class TransportPresenter final
{
public:

    explicit TransportPresenter(ControlPanel& ownerPanel);

    void handleRepeatToggle(bool shouldRepeat);

    void handleAutoplayToggle(bool shouldAutoplay);

    void handleCutModeToggle(bool enableCutMode);

private:

    void enforceCutBounds() const;

    ControlPanel& owner;
};

#endif 
