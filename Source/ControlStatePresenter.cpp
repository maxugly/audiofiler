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
    DBG("ControlPanel::updateComponentStates() - START");
    const bool enabled = owner.getAudioPlayer().getThumbnail().getTotalLength() > 0.0;
    DBG("  - enabled (file loaded): " << (enabled ? "true" : "false"));
    DBG("  - m_isCutModeActive: " << (owner.m_isCutModeActive ? "true" : "false"));

    updateGeneralButtonStates(enabled);
    updateCutModeControlStates(owner.m_isCutModeActive, enabled);
    DBG("ControlPanel::updateComponentStates() - END");
}

void ControlStatePresenter::updateGeneralButtonStates(bool enabled)
{
    DBG("ControlPanel::updateGeneralButtonStates() - START (parent enabled: " << (enabled ? "true" : "false") << ")");
    owner.openButton.setEnabled(true); DBG("  - openButton enabled: " << (owner.openButton.isEnabled() ? "true" : "false"));
    owner.exitButton.setEnabled(true); DBG("  - exitButton enabled: " << (owner.exitButton.isEnabled() ? "true" : "false"));
    owner.loopButton.setEnabled(true); DBG("  - loopButton enabled: " << (owner.loopButton.isEnabled() ? "true" : "false"));
    owner.autoplayButton.setEnabled(true); DBG("  - autoplayButton enabled: " << (owner.autoplayButton.isEnabled() ? "true" : "false"));
    owner.cutButton.setEnabled(true); DBG("  - cutButton enabled: " << (owner.cutButton.isEnabled() ? "true" : "false"));

    owner.playStopButton.setEnabled(enabled); DBG("  - playStopButton enabled: " << (owner.playStopButton.isEnabled() ? "true" : "false"));
    owner.modeButton.setEnabled(enabled); DBG("  - modeButton enabled: " << (owner.modeButton.isEnabled() ? "true" : "false"));
    owner.statsButton.setEnabled(enabled); DBG("  - statsButton enabled: " << (owner.statsButton.isEnabled() ? "true" : "false"));
    owner.channelViewButton.setEnabled(enabled); DBG("  - channelViewButton enabled: " << (owner.channelViewButton.isEnabled() ? "true" : "false"));
    owner.qualityButton.setEnabled(enabled); DBG("  - qualityButton enabled: " << (owner.qualityButton.isEnabled() ? "true" : "false"));
    if (owner.statsPresenter != nullptr)
    {
        owner.statsPresenter->setDisplayEnabled(enabled);
        DBG("  - statsDisplay enabled: " << (owner.statsPresenter->getDisplay().isEnabled() ? "true" : "false"));
    }
    DBG("ControlPanel::updateGeneralButtonStates() - END");
}

void ControlStatePresenter::updateCutModeControlStates(bool isCutModeActive, bool enabled)
{
    DBG("ControlPanel::updateCutModeControlStates() - START (isCutModeActive parameter: " << (isCutModeActive ? "true" : "false") << ", parent enabled: " << (enabled ? "true" : "false") << ")");
    owner.loopInButton.setEnabled(enabled && isCutModeActive); DBG("  - loopInButton enabled: " << (owner.loopInButton.isEnabled() ? "true" : "false"));
    owner.loopInEditor.setEnabled(enabled && isCutModeActive); DBG("  - loopInEditor enabled: " << (owner.loopInEditor.isEnabled() ? "true" : "false"));
    owner.clearLoopInButton.setEnabled(enabled && isCutModeActive); DBG("  - clearLoopInButton enabled: " << (owner.clearLoopInButton.isEnabled() ? "true" : "false"));

    owner.loopOutButton.setEnabled(enabled && isCutModeActive); DBG("  - loopOutButton enabled: " << (owner.loopOutButton.isEnabled() ? "true" : "false"));
    owner.loopOutEditor.setEnabled(enabled && isCutModeActive); DBG("  - loopOutEditor enabled: " << (owner.loopOutEditor.isEnabled() ? "true" : "false"));
    owner.clearLoopOutButton.setEnabled(enabled && isCutModeActive); DBG("  - clearLoopOutButton enabled: " << (owner.clearLoopOutButton.isEnabled() ? "true" : "false"));

    owner.autoCutInButton.setEnabled(isCutModeActive); DBG("  - autoCutInButton enabled: " << (owner.autoCutInButton.isEnabled() ? "true" : "false"));
    owner.autoCutOutButton.setEnabled(isCutModeActive); DBG("  - autoCutOutOutton enabled: " << (owner.autoCutOutButton.isEnabled() ? "true" : "false"));

    owner.autoCutInButton.setToggleState(owner.silenceDetector->getIsAutoCutInActive(), juce::dontSendNotification);
    owner.autoCutOutButton.setToggleState(owner.silenceDetector->getIsAutoCutOutActive(), juce::dontSendNotification);

    owner.silenceDetector->getInSilenceThresholdEditor().setEnabled(true); DBG("  - inSilenceThresholdEditor enabled: true");
    owner.silenceDetector->getOutSilenceThresholdEditor().setEnabled(true); DBG("  - outSilenceThresholdEditor enabled: true");

    owner.loopInButton.setVisible(isCutModeActive); DBG("  - loopInButton visible: " << (owner.loopInButton.isVisible() ? "true" : "false"));
    owner.loopOutButton.setVisible(isCutModeActive); DBG("  - loopOutButton visible: " << (owner.loopOutButton.isVisible() ? "true" : "false"));
    owner.loopInEditor.setVisible(isCutModeActive); DBG("  - loopInEditor visible: " << (owner.loopInEditor.isVisible() ? "true" : "false"));
    owner.loopOutEditor.setVisible(isCutModeActive); DBG("  - loopOutEditor visible: " << (owner.loopOutEditor.isVisible() ? "true" : "false"));
    owner.clearLoopInButton.setVisible(isCutModeActive); DBG("  - clearLoopInButton visible: " << (owner.clearLoopInButton.isVisible() ? "true" : "false"));
    owner.clearLoopOutButton.setVisible(isCutModeActive); DBG("  - clearLoopOutButton visible: " << (owner.clearLoopOutButton.isEnabled() ? "true" : "false"));
    owner.silenceDetector->getInSilenceThresholdEditor().setVisible(isCutModeActive); DBG("  - inSilenceThresholdEditor visible: " << (owner.silenceDetector->getInSilenceThresholdEditor().isVisible() ? "true" : "false"));
    owner.silenceDetector->getOutSilenceThresholdEditor().setVisible(isCutModeActive); DBG("  - outSilenceThresholdEditor visible: " << (owner.silenceDetector->getOutSilenceThresholdEditor().isVisible() ? "true" : "false"));
    owner.autoCutInButton.setVisible(isCutModeActive); DBG("  - autoCutInButton visible: " << (owner.autoCutInButton.isVisible() ? "true" : "false"));
    owner.autoCutOutButton.setVisible(isCutModeActive); DBG("  - autoCutOutButton visible: " << (owner.autoCutOutButton.isVisible() ? "true" : "false"));
    DBG("ControlPanel::updateCutModeControlStates() - END");
}
