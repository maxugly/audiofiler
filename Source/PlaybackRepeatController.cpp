

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
    auto& sessionState = controlPanel.getSessionState();
    const bool autoPlayPreference = sessionState.getCutPrefs().autoplay;
    const bool isPlaying = audioPlayer.isPlaying();

    if (autoPlayPreference)
    {
        if (!isPlaying)
        {
            if (lastIsPlaying)
            {
                // Condition B: User manually stopped or playback ended naturally
                sessionState.setAutoPlayActive(false);
            }
            else
            {
                // Condition A: AutoPlay is active but not playing yet
                audioPlayer.startPlayback();
            }
        }
    }

    lastIsPlaying = isPlaying;
    lastAutoPlayPreference = autoPlayPreference;
}
