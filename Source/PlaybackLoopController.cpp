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
    const double loopIn = controlPanel.getLoopInPosition();
    const double loopOut = controlPanel.getLoopOutPosition();

    if (loopOut <= loopIn)
        return;

    auto& transport = audioPlayer.getTransportSource();
    const double currentPosition = transport.getCurrentPosition();

    if (currentPosition < loopOut)
        return;

    if (controlPanel.isCutModeActive())
    {
        if (controlPanel.getShouldLoop())
            transport.setPosition(loopIn);
        else
            transport.stop();
        return;
    }

    if (controlPanel.getShouldLoop())
        transport.setPosition(loopIn);
}
