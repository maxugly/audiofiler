#ifndef AUDIOFILER_PLAYBACKLOOPCONTROLLER_H
#define AUDIOFILER_PLAYBACKLOOPCONTROLLER_H

#include <JuceHeader.h>

class AudioPlayer;
class ControlPanel;

/**
 * @class PlaybackLoopController
 * @brief Enforces loop boundaries during playback based on cut mode and loop state.
 *
 * This controller keeps loop enforcement logic out of MainComponent::timerCallback,
 * delegating playback boundary checks to a focused helper.
 */
class PlaybackLoopController final
{
public:
    /**
     * @brief Constructs the controller.
     * @param audioPlayerIn Reference to the AudioPlayer driving playback.
     * @param controlPanelIn Reference to the ControlPanel providing loop state.
     */
    PlaybackLoopController(AudioPlayer& audioPlayerIn, ControlPanel& controlPanelIn);

    /**
     * @brief Runs a single loop enforcement tick.
     */
    void tick();

private:
    AudioPlayer& audioPlayer;
    ControlPanel& controlPanel;
};

#endif // AUDIOFILER_PLAYBACKLOOPCONTROLLER_H
