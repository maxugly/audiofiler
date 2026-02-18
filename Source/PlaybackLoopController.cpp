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
    const double loopIn = controlPanel.getCutInPosition();
    const double loopOut = controlPanel.getCutOutPosition();

    if (loopOut <= loopIn)
        return;

    auto& transport = audioPlayer.getTransportSource();
    const double currentPosition = transport.getCurrentPosition();

    if (currentPosition < loopOut)
        return;

    if (controlPanel.isCutModeActive())
    {
        if (controlPanel.getShouldLoop())
            audioPlayer.setPlayheadPosition(loopIn);
        else
            transport.stop();
        return;
    }

    if (controlPanel.getShouldLoop())
        audioPlayer.setPlayheadPosition(loopIn);
}
