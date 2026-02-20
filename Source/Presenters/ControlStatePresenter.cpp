

#include "Presenters/ControlStatePresenter.h"

#include "UI/ControlPanel.h"
#include "Presenters/StatsPresenter.h"
#include "Workers/SilenceDetector.h"
#include "Core/AudioPlayer.h"

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

    if (auto* ts = owner.getTransportStrip()) {
        ts->getRepeatButton().setEnabled(true);
        ts->getAutoplayButton().setEnabled(true);
        ts->getCutButton().setEnabled(true);
        ts->getPlayStopButton().setEnabled(enabled);
        ts->getStopButton().setEnabled(enabled);
    }

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
    if (owner.inStrip != nullptr) {
        owner.inStrip->setEnabled(enabled && isCutModeActive);
        owner.inStrip->setVisible(isCutModeActive);
    }

    if (owner.outStrip != nullptr) {
        owner.outStrip->setEnabled(enabled && isCutModeActive);
        owner.outStrip->setVisible(isCutModeActive);
    }

    owner.cutLengthEditor.setEnabled(enabled && isCutModeActive);
    owner.cutLengthEditor.setVisible(isCutModeActive);

    if (owner.inStrip != nullptr)
        owner.inStrip->updateAutoCutState(owner.getSilenceDetector().getIsAutoCutInActive());
    if (owner.outStrip != nullptr)
        owner.outStrip->updateAutoCutState(owner.getSilenceDetector().getIsAutoCutOutActive());
}
