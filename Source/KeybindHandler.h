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

/**
 * @class KeybindHandler
 * @brief Manages keyboard shortcuts for the application.
 */
class KeybindHandler final
{
public:
    KeybindHandler(MainComponent& mainComponentIn, AudioPlayer& audioPlayerIn, ControlPanel& controlPanelIn);

    /**
     * @brief Central entry point for key press handling.
     */
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

#endif // AUDIOFILER_KEYBINDHANDLER_H
