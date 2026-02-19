/**
 * @file PlaybackRepeatController.h
 * @brief Defines the PlaybackRepeatController class.
 * @ingroup Presenters
 */

#ifndef AUDIOFILER_PLAYBACKREPEATCONTROLLER_H
#define AUDIOFILER_PLAYBACKREPEATCONTROLLER_H

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
#else
    #include <JuceHeader.h>
#endif

/**
 * @class AudioPlayer
 * @brief Home: Engine.
 *
 */
class AudioPlayer;
/**
 * @class ControlPanel
 * @brief Home: View.
 *
 */
class ControlPanel;

/**
 * @class PlaybackRepeatController
 * @brief Enforces repeat boundaries during playback.
 */
class PlaybackRepeatController final
{
public:
    /**
     * @brief Undocumented method.
     * @param audioPlayerIn [in] Description for audioPlayerIn.
     * @param controlPanelIn [in] Description for controlPanelIn.
     */
    PlaybackRepeatController(AudioPlayer& audioPlayerIn, ControlPanel& controlPanelIn);

    /**
     * @brief Runs a single repeat enforcement tick.
     */
    void tick();

private:
    AudioPlayer& audioPlayer;
    ControlPanel& controlPanel;
};

#endif 
