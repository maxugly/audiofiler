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
    // Logic moved to AudioPlayer::getNextAudioBlock
}
