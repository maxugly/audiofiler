#include "UI/Components/TransportStrip.h"
#include "Utils/Config.h"
#include "Core/AppEnums.h"

TransportStrip::TransportStrip(AudioPlayer& player, SessionState& state)
    : audioPlayer(player), sessionState(state)
{
    initialiseButtons();
}

void TransportStrip::initialiseButtons()
{
    // Play/Stop Button
    addAndMakeVisible(playStopButton);
    playStopButton.setButtonText(Config::Labels::playButton);
    playStopButton.getProperties().set("GroupPosition", (int)AppEnums::GroupPosition::Left);
    playStopButton.onClick = [this] { audioPlayer.togglePlayStop(); };
    playStopButton.setEnabled(false);

    // Stop Button
    addAndMakeVisible(stopButton);
    stopButton.setButtonText(Config::Labels::stopButton);
    stopButton.getProperties().set("GroupPosition", (int)AppEnums::GroupPosition::Middle);
    stopButton.onClick = [this] {
        audioPlayer.stopPlaybackAndReset();
        sessionState.setAutoPlayActive(false);
    };
    stopButton.setEnabled(false);

    // Autoplay Button
    addAndMakeVisible(autoplayButton);
    autoplayButton.setButtonText(Config::Labels::autoplayButton);
    autoplayButton.getProperties().set("GroupPosition", (int)AppEnums::GroupPosition::Middle);
    autoplayButton.setClickingTogglesState(true);
    autoplayButton.setToggleState(sessionState.getCutPrefs().autoplay, juce::dontSendNotification);
    autoplayButton.onClick = [this] {
        sessionState.setAutoPlayActive(autoplayButton.getToggleState());
    };

    // Repeat Button
    addAndMakeVisible(repeatButton);
    repeatButton.setButtonText(Config::Labels::repeatButton);
    repeatButton.getProperties().set("GroupPosition", (int)AppEnums::GroupPosition::Middle);
    repeatButton.setClickingTogglesState(true);
    repeatButton.onClick = [this] {
        audioPlayer.setRepeating(repeatButton.getToggleState());
    };

    // Cut Button
    addAndMakeVisible(cutButton);
    cutButton.setButtonText(Config::Labels::cutButton);
    cutButton.getProperties().set("GroupPosition", (int)AppEnums::GroupPosition::Right);
    cutButton.setClickingTogglesState(true);
    cutButton.setToggleState(sessionState.getCutPrefs().active, juce::dontSendNotification);
    cutButton.onClick = [this] {
        const bool active = cutButton.getToggleState();
        sessionState.setCutActive(active);
        
        // If enabling cut mode while playing, we might need to jump to cut in
        if (active && audioPlayer.isPlaying())
        {
            const double pos = audioPlayer.getCurrentPosition();
            const double cutIn = sessionState.getCutIn();
            const double cutOut = sessionState.getCutOut();
            if (pos < cutIn || pos >= cutOut)
                audioPlayer.setPlayheadPosition(cutIn);
        }
    };
}

void TransportStrip::resized()
{
    auto b = getLocalBounds();
    const int buttonWidth = Config::Layout::buttonWidth;
    const int spacing = (int)Config::UI::GroupSpacing;

    playStopButton.setBounds(b.removeFromLeft(buttonWidth));
    b.removeFromLeft(spacing);
    stopButton.setBounds(b.removeFromLeft(buttonWidth));
    b.removeFromLeft(spacing);
    autoplayButton.setBounds(b.removeFromLeft(buttonWidth));
    b.removeFromLeft(spacing);
    repeatButton.setBounds(b.removeFromLeft(buttonWidth));
    b.removeFromLeft(spacing);
    cutButton.setBounds(b.removeFromLeft(buttonWidth));
}

void TransportStrip::updatePlayButtonText(bool isPlaying)
{
    playStopButton.setButtonText(isPlaying ? Config::Labels::stopButton : Config::Labels::playButton);
}

void TransportStrip::updateCutModeState(bool isCutModeActive)
{
    cutButton.setToggleState(isCutModeActive, juce::dontSendNotification);
}

void TransportStrip::updateAutoplayState(bool isAutoplayActive)
{
    autoplayButton.setToggleState(isAutoplayActive, juce::dontSendNotification);
}

void TransportStrip::updateRepeatState(bool isRepeating)
{
    repeatButton.setToggleState(isRepeating, juce::dontSendNotification);
}
