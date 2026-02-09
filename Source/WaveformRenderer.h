#pragma once

#include <JuceHeader.h>

class ControlPanel;
class AudioPlayer;

/**
 * @class WaveformRenderer
 * @brief Renders the waveform, loop overlays, playback cursor, and mouse feedback for the ControlPanel.
 *
 * Extracting this class keeps ControlPanel::paint concise while making it easier to reason about
 * future visual tweaks or alternate render modes.
 */
class WaveformRenderer
{
public:
    /**
     * @brief Constructs a renderer bound to a ControlPanel.
     * @param controlPanel Reference to the owning ControlPanel for accessing state and helpers.
     */
    explicit WaveformRenderer(ControlPanel& controlPanel);

    /**
     * @brief Paints the waveform and overlays for the ControlPanel.
     * @param g Graphics context supplied by ControlPanel::paint.
     */
    void render(juce::Graphics& g);

private:
    void drawWaveform(juce::Graphics& g, AudioPlayer& audioPlayer) const;
    void drawReducedQualityWaveform(juce::Graphics& g, AudioPlayer& audioPlayer, int channel, int pixelsPerSample) const;
    void drawCutModeOverlays(juce::Graphics& g, AudioPlayer& audioPlayer, float audioLength) const;
    void drawPlaybackCursor(juce::Graphics& g, AudioPlayer& audioPlayer, float audioLength) const;
    void drawMouseCursorOverlays(juce::Graphics& g, AudioPlayer& audioPlayer, float audioLength) const;

    ControlPanel& controlPanel;
};
