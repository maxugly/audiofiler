#include "KeybindHandler.h"

#include "MainComponent.h"
#include "AudioPlayer.h"
#include "ControlPanel.h"
#include "AppEnums.h"
#include "Config.h"

KeybindHandler::KeybindHandler(MainComponent& mainComponentIn,
                               AudioPlayer& audioPlayerIn,
                               ControlPanel& controlPanelIn)
    : mainComponent(mainComponentIn),
      audioPlayer(audioPlayerIn),
      controlPanel(controlPanelIn)
{
}

bool KeybindHandler::handleKeyPress(const juce::KeyPress& key)
{
    if (handleGlobalKeybinds(key))
        return true;

    // Why: Playback/UI/loop shortcuts only make sense when audio is loaded.
    if (audioPlayer.getThumbnail().getTotalLength() > 0.0)
    {
        if (handlePlaybackKeybinds(key)) return true;
        if (handleUIToggleKeybinds(key)) return true;
        if (handleLoopKeybinds(key)) return true;
    }
    return false;
}

bool KeybindHandler::handleGlobalKeybinds(const juce::KeyPress& key)
{
    const juce::juce_wchar keyChar = key.getTextCharacter();
    if (keyChar == 'e' || keyChar == 'E')
    {
        // Why: Provide a keyboard-only exit without reaching for the window controls.
        if (auto* app = juce::JUCEApplication::getInstance())
            app->systemRequestedQuit();
        return true;
    }
    if (keyChar == 'd' || keyChar == 'D')
    {
        // Why: Quick access to the file picker keeps audition workflows fast.
        mainComponent.openButtonClicked();
        return true;
    }
    return false;
}

bool KeybindHandler::handlePlaybackKeybinds(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::spaceKey)
    {
        // Why: Space bar acts as the universal transport toggle in most DAWs.
        audioPlayer.togglePlayStop();
        return true;
    }
    constexpr double seekStepSeconds = Config::keyboardSkipAmountSeconds;
    if (key.getKeyCode() == juce::KeyPress::leftKey)
    {
        // Why: Provide quick, predictable scrubbing in fixed steps.
        const double current = audioPlayer.getTransportSource().getCurrentPosition();
        const double loopIn = controlPanel.getLoopInPosition();
        audioPlayer.getTransportSource().setPosition(juce::jmax(loopIn, current - seekStepSeconds));
        return true;
    }
    if (key.getKeyCode() == juce::KeyPress::rightKey)
    {
        const double current = audioPlayer.getTransportSource().getCurrentPosition();
        const double loopOut = controlPanel.getLoopOutPosition();
        audioPlayer.getTransportSource().setPosition(juce::jlimit(0.0, loopOut, current + seekStepSeconds));
        return true;
    }
    return false;
}

bool KeybindHandler::handleUIToggleKeybinds(const juce::KeyPress& key)
{
    const auto keyChar = key.getTextCharacter();
    if (keyChar == 's' || keyChar == 'S') { controlPanel.toggleStats(); return true; }
    if (keyChar == 'v' || keyChar == 'V') { controlPanel.triggerModeButton(); return true; }
    if (keyChar == 'c' || keyChar == 'C') { controlPanel.triggerChannelViewButton(); return true; }
    if (keyChar == 'q' || keyChar == 'Q') { controlPanel.triggerQualityButton(); return true; }
    if (keyChar == 'l' || keyChar == 'L') { controlPanel.triggerLoopButton(); return true; }
    return false;
}

bool KeybindHandler::handleLoopKeybinds(const juce::KeyPress& key)
{
    const auto keyChar = key.getTextCharacter();
    if (controlPanel.getPlacementMode() == AppEnums::PlacementMode::None)
    {
        if (keyChar == 'i' || keyChar == 'I')
        {
            // Why: Snapshot the current playhead as loop-in when no placement mode is active.
            controlPanel.setLoopInPosition(audioPlayer.getTransportSource().getCurrentPosition());
            controlPanel.repaint();
            return true;
        }
        if (keyChar == 'o' || keyChar == 'O')
        {
            // Why: Snapshot the current playhead as loop-out when no placement mode is active.
            controlPanel.setLoopOutPosition(audioPlayer.getTransportSource().getCurrentPosition());
            controlPanel.repaint();
            return true;
        }
    }
    if (keyChar == 'u' || keyChar == 'U')
    {
        // Why: Clearing loop-in must remain possible even while placing loop points.
        controlPanel.clearLoopIn();
        return true;
    }
    if (keyChar == 'p' || keyChar == 'P')
    {
        controlPanel.clearLoopOut();
        return true;
    }
    return false;
}
