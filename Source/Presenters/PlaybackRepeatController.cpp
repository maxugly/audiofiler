

#include "Presenters/PlaybackRepeatController.h"

#include "Core/AudioPlayer.h"
#include "UI/ControlPanel.h"

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

    // Only take action if the preference has changed OR if playback state changed
    if (autoPlayPreference != lastAutoPlayPreference || isPlaying != lastIsPlaying)
    {
        if (autoPlayPreference && !lastAutoPlayPreference && !isPlaying)
        {
            // Transition: User just enabled AutoPlay while NOT playing -> Start immediately
            audioPlayer.startPlayback();
        }
        else if (autoPlayPreference && !isPlaying && lastIsPlaying)
        {
            // Transition: AutoPlay was on, and we JUST stopped (naturally or manual stop)
            // In CD-style transport, manual Stop or end-of-track kills AutoPlay
            sessionState.setAutoPlayActive(false);
        }
    }

    lastIsPlaying = isPlaying;
    lastAutoPlayPreference = autoPlayPreference;
}
