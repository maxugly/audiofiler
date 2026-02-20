

#include "Presenters/TransportPresenter.h"

#include "UI/ControlPanel.h"
#include "Core/AudioPlayer.h"

TransportPresenter::TransportPresenter(ControlPanel& ownerPanel)
    : owner(ownerPanel)
{
}

void TransportPresenter::handleRepeatToggle(bool shouldRepeat)
{
    owner.setShouldRepeat(shouldRepeat);
    owner.getAudioPlayer().setRepeating(owner.getShouldRepeat());
}

void TransportPresenter::handleAutoplayToggle(bool shouldAutoplay)
{
    owner.getSessionState().setAutoPlayActive(shouldAutoplay);
}

void TransportPresenter::handleCutModeToggle(bool enableCutMode)
{
    owner.m_isCutModeActive = enableCutMode;
    owner.getSessionState().setCutActive(enableCutMode);
    owner.updateComponentStates();
    if (owner.m_isCutModeActive && owner.getAudioPlayer().isPlaying())

        enforceCutBounds();
}

void TransportPresenter::enforceCutBounds() const
{
    auto& audioPlayer = owner.getAudioPlayer();
    const double currentPosition = audioPlayer.getCurrentPosition();
    const double cutIn = owner.getCutInPosition();
    const double cutOut = owner.getCutOutPosition();

    if (cutOut > cutIn
        && (currentPosition < cutIn || currentPosition >= cutOut))
    {
        audioPlayer.setPlayheadPosition(cutIn);
    }
}
