#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

/**
 * @class SilenceAlgorithms
 * @brief Pure logic for silence detection, decoupled from UI and Application state.
 */
class SilenceAlgorithms
{
public:
    /**
     * @brief Finds the first sample index exceeding the threshold.
     * @param buffer The audio buffer to scan.
     * @param threshold Normalized threshold (0-1) for detecting sound.
     * @return The sample index of the first sound, or -1 if no sound is found.
     */
    static int findSilenceStart(const juce::AudioBuffer<float>& buffer, float threshold);

    /**
     * @brief Finds the last sample index exceeding the threshold.
     * @param buffer The audio buffer to scan.
     * @param threshold Normalized threshold (0-1) for detecting sound.
     * @return The sample index of the last sound, or -1 if no sound is found.
     */
    static int findSilenceEnd(const juce::AudioBuffer<float>& buffer, float threshold);
};
