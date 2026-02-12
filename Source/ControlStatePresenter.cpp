#include "ControlStatePresenter.h"

#include "ControlPanel.h"
#include "StatsPresenter.h"
#include "SilenceDetector.h"
#include "AudioPlayer.h"
#include "Config.h"

ControlStatePresenter::ControlStatePresenter(ControlPanel& ownerPanel)
    : owner(ownerPanel)
{
}

void ControlStatePresenter::refreshStates()
{
    const bool enabled = owner.getAudioPlayer().getThumbnail().getTotalLength() > 0.0;

    updateGeneralButtonStates(enabled);
    updateCutModeControlStates(owner.m_isCutModeActive, enabled);
}

void ControlStatePresenter::updateGeneralButtonStates(bool enabled)
{
    owner.openButton.setEnabled(true);
    owner.exitButton.setEnabled(true);
    owner.loopButton.setEnabled(true);
    owner.autoplayButton.setEnabled(true);
    owner.cutButton.setEnabled(true);

    owner.playStopButton.setEnabled(enabled);
    owner.modeButton.setEnabled(enabled);
    owner.statsButton.setEnabled(enabled);
    owner.channelViewButton.setEnabled(enabled);
    owner.qualityButton.setEnabled(enabled);

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
    owner.loopInButton.setEnabled(enabled && isCutModeActive);
    owner.loopInEditor.setEnabled(enabled && isCutModeActive);
    owner.clearLoopInButton.setEnabled(enabled && isCutModeActive);

    owner.loopOutButton.setEnabled(enabled && isCutModeActive);
    owner.loopOutEditor.setEnabled(enabled && isCutModeActive);
    owner.clearLoopOutButton.setEnabled(enabled && isCutModeActive);

    owner.loopLengthEditor.setEnabled(enabled && isCutModeActive);
    owner.loopLengthEditor.setVisible(isCutModeActive);

    owner.autoCutInButton.setEnabled(isCutModeActive);
    owner.autoCutOutButton.setEnabled(isCutModeActive);

    owner.autoCutInButton.setToggleState(owner.silenceDetector->getIsAutoCutInActive(), juce::dontSendNotification);
    owner.autoCutOutButton.setToggleState(owner.silenceDetector->getIsAutoCutOutActive(), juce::dontSendNotification);

    owner.silenceDetector->getInSilenceThresholdEditor().setEnabled(true);
    owner.silenceDetector->getOutSilenceThresholdEditor().setEnabled(true);

    owner.loopInButton.setVisible(isCutModeActive);
    owner.loopOutButton.setVisible(isCutModeActive);
    owner.loopInEditor.setVisible(isCutModeActive);
    owner.loopOutEditor.setVisible(isCutModeActive);
    owner.clearLoopInButton.setVisible(isCutModeActive);
    owner.clearLoopOutButton.setVisible(isCutModeActive);
    owner.silenceDetector->getInSilenceThresholdEditor().setVisible(isCutModeActive);
    owner.silenceDetector->getOutSilenceThresholdEditor().setVisible(isCutModeActive);
    owner.autoCutInButton.setVisible(isCutModeActive);
    owner.autoCutOutButton.setVisible(isCutModeActive);
}
