

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

    static juce::int64 findSilenceIn(juce::AudioFormatReader& reader, float threshold,
                                     juce::Thread* thread = nullptr);

    static juce::int64 findSilenceOut(juce::AudioFormatReader& reader, float threshold,
                                      juce::Thread* thread = nullptr);
};

#endif 
