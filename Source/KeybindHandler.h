/**
 * @file KeybindHandler.h
 * @brief Defines the KeybindHandler class.
 * @ingroup Engine
 */

#ifndef AUDIOFILER_KEYBINDHANDLER_H
#define AUDIOFILER_KEYBINDHANDLER_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

/**
 * @class MainComponent
 * @brief Home: View.
 *
 */
class MainComponent;
/**
 * @class AudioPlayer
 * @brief Home: Engine.
 *
 */
class AudioPlayer;
/**
 * @class ControlPanel
 * @brief Home: View.
 *
 */
class ControlPanel;

/**
 * @class KeybindHandler
 * @brief Manages keyboard shortcuts for the application.
 */
class KeybindHandler final
{
public:
    /**
     * @brief Undocumented method.
     * @param mainComponentIn [in] Description for mainComponentIn.
     * @param audioPlayerIn [in] Description for audioPlayerIn.
     * @param controlPanelIn [in] Description for controlPanelIn.
     */
    KeybindHandler(MainComponent& mainComponentIn, AudioPlayer& audioPlayerIn, ControlPanel& controlPanelIn);

    /**
     * @brief Central entry point for key press handling.
     */
    bool handleKeyPress(const juce::KeyPress& key);

private:
    /**
     * @brief Undocumented method.
     * @param key [in] Description for key.
     * @return bool
     */
    bool handleGlobalKeybinds(const juce::KeyPress& key);
    /**
     * @brief Undocumented method.
     * @param key [in] Description for key.
     * @return bool
     */
    bool handlePlaybackKeybinds(const juce::KeyPress& key);
    /**
     * @brief Undocumented method.
     * @param key [in] Description for key.
     * @return bool
     */
    bool handleUIToggleKeybinds(const juce::KeyPress& key);
    /**
     * @brief Undocumented method.
     * @param key [in] Description for key.
     * @return bool
     */
    bool handleCutKeybinds(const juce::KeyPress& key);

    MainComponent& mainComponent;
    AudioPlayer& audioPlayer;
    ControlPanel& controlPanel;
};

#endif // AUDIOFILER_KEYBINDHANDLER_H
