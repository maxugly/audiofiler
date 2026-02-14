#ifndef AUDIOFILER_PLAYBACKCURSORGLOW_H
#define AUDIOFILER_PLAYBACKCURSORGLOW_H

#include <JuceHeader.h>

class ControlPanel;

/**
 * @file PlaybackCursorGlow.h
 * @brief Defines the PlaybackCursorGlow class for rendering the playback cursor with a glow effect.
 */

/**
 * @class PlaybackCursorGlow
 * @brief Handles the rendering of the playback cursor, including its glow effect.
 *
 * This class isolates the visual logic for the playback cursor to keep WaveformRenderer
 * focused on the overall waveform display. It ensures the cursor is drawn with a
 * consistent glow using gradient fills.
 */
class PlaybackCursorGlow
{
public:
    /**
     * @brief Renders the playback cursor and its glow at the current playback position.
     * @param g The graphics context to draw into.
     * @param controlPanel Reference to the ControlPanel to access audio player and state.
     * @param waveformBounds The bounds of the waveform display area.
     */
    static void renderGlow(juce::Graphics& g, const ControlPanel& controlPanel, const juce::Rectangle<int>& waveformBounds);
};

#endif // AUDIOFILER_PLAYBACKCURSORGLOW_H
