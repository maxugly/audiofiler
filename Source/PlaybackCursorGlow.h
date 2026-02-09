#pragma once

#include <JuceHeader.h>

class ControlPanel;

/**
 * @class PlaybackCursorGlow
 * @brief Provides helpers for rendering the playback cursor glow gradient.
 */
class PlaybackCursorGlow
{
public:
    static void renderGlow(juce::Graphics& g, const ControlPanel& controlPanel, const juce::Rectangle<int>& waveformBounds);
};

