#include "SilenceAnalysisWorker.h"

#include "ControlPanel.h"
#include "AudioPlayer.h"
#include "SilenceDetectionLogger.h"
#include <limits>
#include <algorithm>
#include <cmath>

namespace
{
    constexpr int kChunkSize = 65536;
    constexpr juce::int64 kMaxAnalyzableSamples = 2147483647; // INT_MAX, to prevent DoS
    constexpr int kMaxChannels = 128; // Reasonable limit for audio channels

    void resumeIfNeeded(AudioPlayer& player, bool wasPlaying)
    {
        if (wasPlaying)
            player.getTransportSource().start();
    }
}

void SilenceAnalysisWorker::detectInSilence(ControlPanel& ownerPanel, float threshold)
{
    AudioPlayer& audioPlayer = ownerPanel.getAudioPlayer();
    const bool wasPlaying = audioPlayer.isPlaying();
    if (wasPlaying)
        audioPlayer.getTransportSource().stop();

    juce::AudioFormatReader* reader = audioPlayer.getAudioFormatReader();
    if (reader == nullptr)
    {
        SilenceDetectionLogger::logNoAudioLoaded(ownerPanel);
        resumeIfNeeded(audioPlayer, wasPlaying);
        return;
    }

    const juce::int64 lengthInSamples = reader->lengthInSamples;
    SilenceDetectionLogger::logReadingSamples(ownerPanel, "In", lengthInSamples);

    // Security Fix: Check for invalid length to prevent processing errors
    if (lengthInSamples <= 0)
    {
        SilenceDetectionLogger::logZeroLength(ownerPanel);
        resumeIfNeeded(audioPlayer, wasPlaying);
        return;
    }

    // Security Fix: Prevent DoS by rejecting excessively large files
    if (lengthInSamples > kMaxAnalyzableSamples)
    {
        SilenceDetectionLogger::logAudioTooLarge(ownerPanel);
        resumeIfNeeded(audioPlayer, wasPlaying);
        return;
    }

    // Security Fix: Validate channel count to prevent invalid buffer allocation
    if (reader->numChannels <= 0 || reader->numChannels > kMaxChannels)
    {
        // Log error if needed, for now just bail out
        resumeIfNeeded(audioPlayer, wasPlaying);
        return;
    }

    // Security Fix: Process in chunks to avoid large memory allocation (unbounded allocation vulnerability) and integer overflow
    juce::AudioBuffer<float> buffer(reader->numChannels, kChunkSize);

    juce::int64 currentPos = 0;
    while (currentPos < lengthInSamples)
    {
        const int numThisTime = (int) std::min((juce::int64) kChunkSize, lengthInSamples - currentPos);
        buffer.clear();
        if (!reader->read(&buffer, 0, numThisTime, currentPos, true, true))
        {
            resumeIfNeeded(audioPlayer, wasPlaying);
            return;
        }

        for (int sample = 0; sample < numThisTime; ++sample)
        {
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                if (std::abs(buffer.getSample(channel, sample)) > threshold)
                {
                    const juce::int64 globalSample = currentPos + sample;
                    ownerPanel.setLoopInPosition((double)globalSample / reader->sampleRate);
                    SilenceDetectionLogger::logLoopStartSet(ownerPanel, globalSample, reader->sampleRate);

                    // Move playhead to the new loop-in position in cut mode
                    if (ownerPanel.isCutModeActive())
                        audioPlayer.getTransportSource().setPosition(ownerPanel.getLoopInPosition());

                    resumeIfNeeded(audioPlayer, wasPlaying);
                    return;
                }
            }
        }
        currentPos += numThisTime;
    }

    SilenceDetectionLogger::logNoSoundFound(ownerPanel, "start");
    resumeIfNeeded(audioPlayer, wasPlaying);
}

void SilenceAnalysisWorker::detectOutSilence(ControlPanel& ownerPanel, float threshold)
{
    AudioPlayer& audioPlayer = ownerPanel.getAudioPlayer();
    const bool wasPlaying = audioPlayer.isPlaying();
    if (wasPlaying)
        audioPlayer.getTransportSource().stop();

    juce::AudioFormatReader* reader = audioPlayer.getAudioFormatReader();
    if (reader == nullptr)
    {
        SilenceDetectionLogger::logNoAudioLoaded(ownerPanel);
        resumeIfNeeded(audioPlayer, wasPlaying);
        return;
    }

    const juce::int64 lengthInSamples = reader->lengthInSamples;
    SilenceDetectionLogger::logReadingSamples(ownerPanel, "Out", lengthInSamples);

    // Security Fix: Check for invalid length
    if (lengthInSamples <= 0)
    {
        SilenceDetectionLogger::logZeroLength(ownerPanel);
        resumeIfNeeded(audioPlayer, wasPlaying);
        return;
    }

    // Security Fix: Prevent DoS by rejecting excessively large files
    if (lengthInSamples > kMaxAnalyzableSamples)
    {
        SilenceDetectionLogger::logAudioTooLarge(ownerPanel);
        resumeIfNeeded(audioPlayer, wasPlaying);
        return;
    }

    // Security Fix: Validate channel count
    if (reader->numChannels <= 0 || reader->numChannels > kMaxChannels)
    {
        resumeIfNeeded(audioPlayer, wasPlaying);
        return;
    }

    // Security Fix: Process in chunks backwards to avoid unbounded memory allocation
    juce::AudioBuffer<float> buffer(reader->numChannels, kChunkSize);

    juce::int64 currentPos = lengthInSamples;
    while (currentPos > 0)
    {
        const int numThisTime = (int) std::min((juce::int64) kChunkSize, currentPos);
        const juce::int64 startSample = currentPos - numThisTime;

        buffer.clear();
        if (!reader->read(&buffer, 0, numThisTime, startSample, true, true))
        {
            resumeIfNeeded(audioPlayer, wasPlaying);
            return;
        }

        for (int sample = numThisTime - 1; sample >= 0; --sample)
        {
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            {
                if (std::abs(buffer.getSample(channel, sample)) > threshold)
                {
                    const juce::int64 globalSample = startSample + sample;
                    const juce::int64 tailSamples = (juce::int64) (reader->sampleRate * 0.05); // 50ms tail
                    const juce::int64 endPoint64 = globalSample + tailSamples;
                    const juce::int64 finalEndPoint = std::min(endPoint64, lengthInSamples);

                    ownerPanel.setLoopOutPosition((double)finalEndPoint / reader->sampleRate);
                    SilenceDetectionLogger::logLoopEndSet(ownerPanel, finalEndPoint, reader->sampleRate);
                    resumeIfNeeded(audioPlayer, wasPlaying);
                    return;
                }
            }
        }
        currentPos -= numThisTime;
    }

    SilenceDetectionLogger::logNoSoundFound(ownerPanel, "end");
    resumeIfNeeded(audioPlayer, wasPlaying);
}
