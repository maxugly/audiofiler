#ifndef SILENCE_ANALYSIS_ALGORITHMS_H
#define SILENCE_ANALYSIS_ALGORITHMS_H

#ifdef JUCE_HEADLESS
 #include <juce_core/juce_core.h>
 #include <juce_audio_basics/juce_audio_basics.h>
 #include <juce_audio_formats/juce_audio_formats.h>
#else
 #include <JuceHeader.h>
#endif

class SilenceAnalysisAlgorithms
{
public:
    /**
     * @brief Finds the sample index where silence ends (sound begins).
     * @param reader The audio source to scan.
     * @param threshold The normalized amplitude threshold (0-1).
     * @return The sample index of the first sound, or -1 if none found.
     */
    static juce::int64 findSilenceIn(juce::AudioFormatReader& reader, float threshold);

    /**
     * @brief Finds the sample index where sound ends (silence begins).
     * @param reader The audio source to scan.
     * @param threshold The normalized amplitude threshold (0-1).
     * @return The sample index where sound ends, or -1 if none found.
     */
    static juce::int64 findSilenceOut(juce::AudioFormatReader& reader, float threshold);
};

#endif // SILENCE_ANALYSIS_ALGORITHMS_H
