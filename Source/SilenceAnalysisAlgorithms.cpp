

#include "SilenceAnalysisAlgorithms.h"
#include <limits>
#include <algorithm>
#include <cmath>

namespace
{
    constexpr int kChunkSize = 65536;

    constexpr int kMaxChannels = 128; 
}

juce::int64 SilenceAnalysisAlgorithms::findSilenceIn(juce::AudioFormatReader& reader, float threshold,
                                                     juce::Thread* thread)
{
    const juce::int64 lengthInSamples = reader.lengthInSamples;

    if (reader.numChannels <= 0 || reader.numChannels > kMaxChannels)
        return -1;

    juce::AudioBuffer<float> buffer(reader.numChannels, kChunkSize);

    juce::int64 currentPos = 0;
    while (currentPos < lengthInSamples)
    {
        const int numThisTime = (int) std::min((juce::int64) kChunkSize, lengthInSamples - currentPos);
        buffer.clear();
        if (!reader.read(&buffer, 0, numThisTime, currentPos, true, true))
            return -1;

        for (int sample = 0; sample < numThisTime; ++sample)
        {
            if (thread != nullptr && thread->threadShouldExit())
                return -1;
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                if (std::abs(buffer.getSample(channel, sample)) > threshold)
                {
                    return currentPos + sample;
                }
            }
        }
        currentPos += numThisTime;
    }
    return -1;
}

juce::int64 SilenceAnalysisAlgorithms::findSilenceOut(juce::AudioFormatReader& reader, float threshold,
                                                      juce::Thread* thread)
{
    const juce::int64 lengthInSamples = reader.lengthInSamples;

    if (reader.numChannels <= 0 || reader.numChannels > kMaxChannels)
        return -1;

    juce::AudioBuffer<float> buffer(reader.numChannels, kChunkSize);

    juce::int64 currentPos = lengthInSamples;
    while (currentPos > 0)
    {
        const int numThisTime = (int) std::min((juce::int64) kChunkSize, currentPos);
        const juce::int64 startSample = currentPos - numThisTime;

        buffer.clear();
        if (!reader.read(&buffer, 0, numThisTime, startSample, true, true))
            return -1;

        for (int sample = numThisTime - 1; sample >= 0; --sample)
        {
            if (thread != nullptr && thread->threadShouldExit())
                return -1;
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                if (std::abs(buffer.getSample(channel, sample)) > threshold)
                {
                    return startSample + sample;
                }
            }
        }
        currentPos -= numThisTime;
    }
    return -1;
}
