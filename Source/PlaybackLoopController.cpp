#include "PlaybackLoopController.h"

#include "AudioPlayer.h"
#include "ControlPanel.h"

PlaybackLoopController::PlaybackLoopController(AudioPlayer& audioPlayerIn, ControlPanel& controlPanelIn)
    : audioPlayer(audioPlayerIn),
      controlPanel(controlPanelIn)
{
}

void PlaybackLoopController::tick()
{
    enforceLoopBounds();
}

void PlaybackLoopController::enforceLoopBounds() const
{
    const double cutIn = controlPanel.getCutInPosition();
    const double cutOut = controlPanel.getCutOutPosition();

    if (cutOut <= cutIn)
        return;

    auto& transport = audioPlayer.getTransportSource();
    const double currentPosition = transport.getCurrentPosition();

    if (currentPosition < cutOut)
        return;

    if (controlPanel.isCutModeActive())
    {
        if (controlPanel.getShouldLoop())
            transport.setPosition(cutIn);
        else
            transport.stop();
        return;
    }

    if (controlPanel.getShouldLoop())
        transport.setPosition(cutIn);
}
