

#include "ControlStatePresenter.h"

#include "ControlPanel.h"
#include "StatsPresenter.h"
#include "SilenceDetector.h"
#include "AudioPlayer.h"

ControlStatePresenter::ControlStatePresenter(ControlPanel& ownerPanel)
    : owner(ownerPanel)
{
}

void ControlStatePresenter::refreshStates()
{
    const bool enabled = owner.getAudioPlayer().getThumbnail().getTotalLength() > 0.0;

    updateGeneralButtonStates(enabled);
    updateCutModeControlStates(owner.isCutModeActive(), enabled);
}

void ControlStatePresenter::updateGeneralButtonStates(bool enabled)
{
    owner.openButton.setEnabled(true);
    owner.exitButton.setEnabled(true);
    owner.repeatButton.setEnabled(true);
    owner.autoplayButton.setEnabled(true);
    owner.cutButton.setEnabled(true);

    owner.playStopButton.setEnabled(enabled);
    owner.stopButton.setEnabled(enabled);
    owner.modeButton.setEnabled(enabled);
    owner.statsButton.setEnabled(enabled);
    owner.channelViewButton.setEnabled(enabled);

    owner.elapsedTimeEditor.setEnabled(enabled);
    owner.remainingTimeEditor.setEnabled(enabled);
    owner.elapsedTimeEditor.setVisible(enabled);
    owner.remainingTimeEditor.setVisible(enabled);

    if (owner.statsPresenter != nullptr)
    {
        owner.statsPresenter->setDisplayEnabled(enabled);
    }
}

void ControlStatePresenter::updateCutModeControlStates(bool isCutModeActive, bool enabled)
{
    owner.cutInButton.setEnabled(enabled && isCutModeActive);
    owner.cutInEditor.setEnabled(enabled && isCutModeActive);
    owner.resetInButton.setEnabled(enabled && isCutModeActive);

    owner.cutOutButton.setEnabled(enabled && isCutModeActive);
    owner.cutOutEditor.setEnabled(enabled && isCutModeActive);
    owner.resetOutButton.setEnabled(enabled && isCutModeActive);

    owner.cutLengthEditor.setEnabled(enabled && isCutModeActive);
    owner.cutLengthEditor.setVisible(isCutModeActive);

    owner.autoCutInButton.setEnabled(isCutModeActive);
    owner.autoCutOutButton.setEnabled(isCutModeActive);

    owner.autoCutInButton.setToggleState(owner.getSilenceDetector().getIsAutoCutInActive(), juce::dontSendNotification);
    owner.autoCutOutButton.setToggleState(owner.getSilenceDetector().getIsAutoCutOutActive(), juce::dontSendNotification);

    owner.getSilenceDetector().getInSilenceThresholdEditor().setEnabled(enabled && isCutModeActive);
    owner.getSilenceDetector().getOutSilenceThresholdEditor().setEnabled(enabled && isCutModeActive);

    owner.cutInButton.setVisible(isCutModeActive);
    owner.cutOutButton.setVisible(isCutModeActive);
    owner.cutInEditor.setVisible(isCutModeActive);
    owner.cutOutEditor.setVisible(isCutModeActive);
    owner.resetInButton.setVisible(isCutModeActive);
    owner.resetOutButton.setVisible(isCutModeActive);
    owner.getSilenceDetector().getInSilenceThresholdEditor().setVisible(isCutModeActive);
    owner.getSilenceDetector().getOutSilenceThresholdEditor().setVisible(isCutModeActive);
    owner.autoCutInButton.setVisible(isCutModeActive);
    owner.autoCutOutButton.setVisible(isCutModeActive);
}
