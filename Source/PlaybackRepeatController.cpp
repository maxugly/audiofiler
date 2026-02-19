/**
 * @file PlaybackRepeatController.cpp
 * @brief Defines the PlaybackRepeatController class.
 * @ingroup Presenters
 */

#include "PlaybackRepeatController.h"

#include "AudioPlayer.h"
#include "ControlPanel.h"

PlaybackRepeatController::PlaybackRepeatController(AudioPlayer& audioPlayerIn, ControlPanel& controlPanelIn)
    : audioPlayer(audioPlayerIn),
      controlPanel(controlPanelIn)
{
}

void PlaybackRepeatController::tick()
{
    
}
