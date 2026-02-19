#ifndef SILENCE_ANALYSIS_ALGORITHMS_H
#define SILENCE_ANALYSIS_ALGORITHMS_H

#ifdef JUCE_HEADLESS
 #include <juce_core/juce_core.h>
 #include <juce_audio_basics/juce_audio_basics.h>
 #include <juce_audio_formats/juce_audio_formats.h>
#else
 #include <JuceHeader.h>
#endif

/**
 * @ingroup AudioEngine
 * @class SilenceAnalysisAlgorithms
 * @brief Static utility class for audio sample analysis.
 * @details This class contains the core logic for detecting silence thresholds in audio data.
 *          It operates on `juce::AudioFormatReader` to support scanning files on disk without
 *          loading the entire file into memory.
 */
class SilenceAnalysisAlgorithms
{
public:

    /**
     * @brief Finds the first non-silent sample from the start of the file.
     * @details This function scans the audio file in chunks (typically 65536 samples)
     *          to find the point where the amplitude exceeds the given threshold.
     *          It processes channels independently and takes the earliest occurrence.
     *
     * @param reader The audio reader for the file.
     * @param threshold The amplitude threshold (0.0 to 1.0).
     * @param thread Optional pointer to the calling thread to check for cancellation (`thread->threadShouldExit()`).
     * @return The sample index of the start of the audio, or 0 if not found.
     */
    static juce::int64 findSilenceIn(juce::AudioFormatReader& reader, float threshold,
                                     juce::Thread* thread = nullptr);

    /**
     * @brief Finds the last non-silent sample from the end of the file.
     * @details This function scans the audio file backwards in chunks. This approach is
     *          critical for memory safety when handling very large files, as it avoids
     *          allocating a buffer for the entire file.
     *
     * @param reader The audio reader for the file.
     * @param threshold The amplitude threshold (0.0 to 1.0).
     * @param thread Optional pointer to the calling thread to check for cancellation.
     * @return The sample index of the end of the audio, or the file length if not found.
     */
    static juce::int64 findSilenceOut(juce::AudioFormatReader& reader, float threshold,
                                      juce::Thread* thread = nullptr);
};

#endif 
