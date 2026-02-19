/**
 * @file KeybindHandler.cpp
 * @brief Defines the KeybindHandler class.
 * @ingroup Engine
 */

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

    if (audioPlayer.getThumbnail().getTotalLength() > 0.0)
    {
        if (handlePlaybackKeybinds(key)) return true;
        if (handleUIToggleKeybinds(key)) return true;
        if (handleCutKeybinds(key)) return true;
    }
    return false;
}

bool KeybindHandler::handleGlobalKeybinds(const juce::KeyPress& key)
{
    const juce::juce_wchar keyChar = key.getTextCharacter();
    if (keyChar == 'e' || keyChar == 'E')
    {
        if (auto* app = juce::JUCEApplication::getInstance())
            app->systemRequestedQuit();
        return true;
    }
    if (keyChar == 'd' || keyChar == 'D')
    {
        mainComponent.openButtonClicked();
        return true;
    }
    return false;
}

bool KeybindHandler::handlePlaybackKeybinds(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::spaceKey)
    {
        audioPlayer.togglePlayStop();
        return true;
    }
    constexpr double seekStepSeconds = Config::Audio::keyboardSkipSeconds;
    if (key.getKeyCode() == juce::KeyPress::leftKey)
    {
        const double current = audioPlayer.getCurrentPosition();
        audioPlayer.setPlayheadPosition(current - seekStepSeconds);
        return true;
    }
    if (key.getKeyCode() == juce::KeyPress::rightKey)
    {
        const double current = audioPlayer.getCurrentPosition();
        audioPlayer.setPlayheadPosition(current + seekStepSeconds);
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
    if (keyChar == 'r' || keyChar == 'R') { controlPanel.triggerRepeatButton(); return true; }
    return false;
}

bool KeybindHandler::handleCutKeybinds(const juce::KeyPress& key)
{
    const auto keyChar = key.getTextCharacter();
    if (controlPanel.getPlacementMode() == AppEnums::PlacementMode::None)
    {
        if (keyChar == 'i' || keyChar == 'I')
        {
            controlPanel.setCutInPosition(audioPlayer.getCurrentPosition());
            controlPanel.setAutoCutInActive(false);
            controlPanel.jumpToCutIn();
            controlPanel.repaint();
            return true;
        }
        if (keyChar == 'o' || keyChar == 'O')
        {
            controlPanel.setCutOutPosition(audioPlayer.getCurrentPosition());
            controlPanel.setAutoCutOutActive(false);
            controlPanel.jumpToCutIn();
            controlPanel.repaint();
            return true;
        }
    }
    if (keyChar == 'u' || keyChar == 'U')
    {
        controlPanel.resetIn();
        return true;
    }
    if (keyChar == 'p' || keyChar == 'P')
    {
        controlPanel.resetOut();
        return true;
    }
    return false;
}
