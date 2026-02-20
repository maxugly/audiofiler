

#ifndef AUDIOFILER_KEYBINDHANDLER_H
#define AUDIOFILER_KEYBINDHANDLER_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

class MainComponent;

class AudioPlayer;

class ControlPanel;

class KeybindHandler final
{
public:

    KeybindHandler(MainComponent& mainComponentIn, AudioPlayer& audioPlayerIn, ControlPanel& controlPanelIn);

    bool handleKeyPress(const juce::KeyPress& key);

private:

    bool handleGlobalKeybinds(const juce::KeyPress& key);

    bool handlePlaybackKeybinds(const juce::KeyPress& key);

    bool handleUIToggleKeybinds(const juce::KeyPress& key);

    bool handleCutKeybinds(const juce::KeyPress& key);

    MainComponent& mainComponent;
    AudioPlayer& audioPlayer;
    ControlPanel& controlPanel;
};

#endif 
