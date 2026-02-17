#include "ControlStatePresenter.h"

#include "ControlPanel.h"
#include "StatsPresenter.h"
#include "SilenceDetector.h"
#include "AudioPlayer.h"

ControlStatePresenter::ControlStatePresenter(ControlPanel& ownerPanel)
    : owner(ownerPanel)
{
}

/**
 * @brief Refreshes the enabled/visible state of all controls based on current app state.
 *
 * Checks if a file is loaded and if cut mode is active, then delegates to specific helpers.
 */
void ControlStatePresenter::refreshStates()
{
    const bool enabled = owner.getAudioPlayer().getThumbnail().getTotalLength() > 0.0;

    updateGeneralButtonStates(enabled);
    updateCutModeControlStates(owner.m_isCutModeActive, enabled);
}



/**
 * @brief Updates the state of general transport and mode buttons.
 * @param enabled True if an audio file is loaded.
 */
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



/**
 * @brief Updates the state of cut-mode specific controls (looping, silence detection).
 * @param isCutModeActive True if the UI is in Cut Mode.
 * @param enabled True if an audio file is loaded.
 */
void ControlStatePresenter::updateCutModeControlStates(bool isCutModeActive, bool enabled)
{
    owner.loopInButton.setEnabled(enabled && isCutModeActive);
    owner.cutInEditor.setEnabled(enabled && isCutModeActive);
    owner.resetInButton.setEnabled(enabled && isCutModeActive);

    owner.loopOutButton.setEnabled(enabled && isCutModeActive);
    owner.cutOutEditor.setEnabled(enabled && isCutModeActive);
    owner.resetOutButton.setEnabled(enabled && isCutModeActive);

    owner.loopLengthEditor.setEnabled(enabled && isCutModeActive);
    owner.loopLengthEditor.setVisible(isCutModeActive);

    owner.autoCutInButton.setEnabled(isCutModeActive);
    owner.autoCutOutButton.setEnabled(isCutModeActive);

    owner.autoCutInButton.setToggleState(owner.silenceDetector->getIsAutoCutInActive(), juce::dontSendNotification);
    owner.autoCutOutButton.setToggleState(owner.silenceDetector->getIsAutoCutOutActive(), juce::dontSendNotification);

    owner.silenceDetector->getInSilenceThresholdEditor().setEnabled(enabled && isCutModeActive);
    owner.silenceDetector->getOutSilenceThresholdEditor().setEnabled(enabled && isCutModeActive);

    owner.loopInButton.setVisible(isCutModeActive);
    owner.loopOutButton.setVisible(isCutModeActive);
    owner.cutInEditor.setVisible(isCutModeActive);
    owner.cutOutEditor.setVisible(isCutModeActive);
    owner.resetInButton.setVisible(isCutModeActive);
    owner.resetOutButton.setVisible(isCutModeActive);
    owner.silenceDetector->getInSilenceThresholdEditor().setVisible(isCutModeActive);
    owner.silenceDetector->getOutSilenceThresholdEditor().setVisible(isCutModeActive);
    owner.autoCutInButton.setVisible(isCutModeActive);
    owner.autoCutOutButton.setVisible(isCutModeActive);
}
