

#ifndef AUDIOFILER_PLAYBACKREPEATCONTROLLER_H
#define AUDIOFILER_PLAYBACKREPEATCONTROLLER_H

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
#else
    #include <JuceHeader.h>
#endif

class AudioPlayer;

class ControlPanel;

class PlaybackRepeatController final
{
public:

    PlaybackRepeatController(AudioPlayer& audioPlayerIn, ControlPanel& controlPanelIn);

    void tick();

private:
    AudioPlayer& audioPlayer;
    ControlPanel& controlPanel;
};

#endif 
