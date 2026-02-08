#pragma once

#include <JuceHeader.h>

class MainComponent;
class AudioPlayer;
class ControlPanel;

/**
 * @class KeybindHandler
 * @brief Centralises keyboard shortcut logic so MainComponent stays focused on layout and wiring.
 *
 * By funnelling all KeyPress handling through this class we keep related code together,
 * making it easier to reason about shortcut behaviour and extend it safely.
 */
class KeybindHandler
{
public:
    /**
     * @brief Creates a handler that can act on MainComponent, AudioPlayer, and ControlPanel.
     * @param mainComponent Reference to the owning `MainComponent` for app-level actions.
     * @param audioPlayer Reference to the `AudioPlayer` for transport manipulation.
     * @param controlPanel Reference to the `ControlPanel` for UI toggles and loop operations.
     */
    KeybindHandler(MainComponent& mainComponent, AudioPlayer& audioPlayer, ControlPanel& controlPanel);

    /**
     * @brief Dispatches the key press to the various handler categories.
     * @param key The `juce::KeyPress` received from JUCE.
     * @return True if any handler consumed the key, false otherwise.
     */
    bool handleKeyPress(const juce::KeyPress& key);

private:
    /**
     * @brief Handles application-wide shortcuts such as quit and file-open.
     *
     * @param key Key press under evaluation.
     * @return True if handled.
     */
    bool handleGlobalKeybinds(const juce::KeyPress& key);

    /**
     * @brief Handles playback transport shortcuts (play/stop, scrubbing).
     *
     * @param key Key press under evaluation.
     * @return True if handled.
     */
    bool handlePlaybackKeybinds(const juce::KeyPress& key);

    /**
     * @brief Handles toggles that reveal or adjust UI panels.
     *
     * @param key Key press under evaluation.
     * @return True if handled.
     */
    bool handleUIToggleKeybinds(const juce::KeyPress& key);

    /**
     * @brief Handles loop placement and clearing shortcuts.
     *
     * @param key Key press under evaluation.
     * @return True if handled.
     */
    bool handleLoopKeybinds(const juce::KeyPress& key);

    MainComponent& mainComponent;
    AudioPlayer& audioPlayer;
    ControlPanel& controlPanel;
};
