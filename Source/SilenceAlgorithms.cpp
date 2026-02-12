#include "SilenceAlgorithms.h"

int SilenceAlgorithms::findSilenceStart(const juce::AudioBuffer<float>& buffer, float threshold)
{
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            if (std::abs(buffer.getSample(channel, sample)) > threshold)
            {
                return sample;
            }
        }
    }
    return -1;
}

int SilenceAlgorithms::findSilenceEnd(const juce::AudioBuffer<float>& buffer, float threshold)
{
    for (int sample = buffer.getNumSamples() - 1; sample >= 0; --sample)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            if (std::abs(buffer.getSample(channel, sample)) > threshold)
            {
                return sample;
            }
        }
    }
    return -1;
}
