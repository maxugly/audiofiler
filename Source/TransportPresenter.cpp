#include "TransportPresenter.h"

#include "ControlPanel.h"
#include "AudioPlayer.h"

TransportPresenter::TransportPresenter(ControlPanel& ownerPanel)
    : owner(ownerPanel)
{
}

void TransportPresenter::handleCutModeToggle(bool active)
{
    owner.setCutModeActive(active);
    owner.getSessionState().cutModeActive = active;
    owner.getAudioPlayer().setCutModeActive(active);

    if (active && owner.getAudioPlayer().isPlaying())
        enforceCutLoopBounds();
}

void TransportPresenter::handleAutoplayToggle(bool shouldAutoplay)
{
    owner.m_shouldAutoplay = shouldAutoplay;
    owner.getSessionState().autoplay = shouldAutoplay;

    if (shouldAutoplay)
    {
        auto& audioPlayer = owner.getAudioPlayer();
        #if !defined(JUCE_HEADLESS)
        if (audioPlayer.getThumbnail().getTotalLength() > 0.0 && !audioPlayer.isPlaying())
        {
            audioPlayer.togglePlayStop();
        }
        #endif
    }
}

void TransportPresenter::enforceCutLoopBounds() const
{
    auto& audioPlayer = owner.getAudioPlayer();
    auto& transport = audioPlayer.getTransportSource();
    const double currentPosition = transport.getCurrentPosition();
    const double cutIn = owner.getCutInPosition();
    const double cutOut = owner.getCutOutPosition();

    if (cutOut > cutIn
        && (currentPosition < cutIn || currentPosition >= cutOut))
    {
        transport.setPosition(cutIn);
    }
}
