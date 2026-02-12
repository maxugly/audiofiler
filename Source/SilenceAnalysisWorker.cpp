#include "SilenceAnalysisWorker.h"

#include "ControlPanel.h"
#include "AudioPlayer.h"
#include "SilenceDetectionLogger.h"
#include "SilenceAlgorithms.h"

namespace
{
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
    if (lengthInSamples == 0)
    {
        SilenceDetectionLogger::logZeroLength(ownerPanel);
        resumeIfNeeded(audioPlayer, wasPlaying);
        return;
    }

    auto buffer = std::make_unique<juce::AudioBuffer<float>>(reader->numChannels, (int) lengthInSamples);
    reader->read(buffer.get(), 0, (int) lengthInSamples, 0, true, true);

    int sample = SilenceAlgorithms::findSilenceStart(*buffer, threshold);
    if (sample != -1)
    {
        ownerPanel.setLoopStart(sample);
        SilenceDetectionLogger::logLoopStartSet(ownerPanel, sample, reader->sampleRate);
        // Move playhead to the new loop-in position in cut mode
        if (ownerPanel.isCutModeActive())
            audioPlayer.getTransportSource().setPosition(ownerPanel.getLoopInPosition());
        resumeIfNeeded(audioPlayer, wasPlaying);
        return;
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
    if (lengthInSamples == 0)
    {
        SilenceDetectionLogger::logZeroLength(ownerPanel);
        resumeIfNeeded(audioPlayer, wasPlaying);
        return;
    }

    auto buffer = std::make_unique<juce::AudioBuffer<float>>(reader->numChannels, (int) lengthInSamples);
    reader->read(buffer.get(), 0, (int) lengthInSamples, 0, true, true);

    int sample = SilenceAlgorithms::findSilenceEnd(*buffer, threshold);
    if (sample != -1)
    {
        int endPoint = sample + (int) (reader->sampleRate * 0.05); // 50ms tail
        endPoint = juce::jmin(endPoint, buffer->getNumSamples());
        ownerPanel.setLoopEnd(endPoint);
        SilenceDetectionLogger::logLoopEndSet(ownerPanel, endPoint, reader->sampleRate);
        resumeIfNeeded(audioPlayer, wasPlaying);
        return;
    }

    SilenceDetectionLogger::logNoSoundFound(ownerPanel, "end");
    resumeIfNeeded(audioPlayer, wasPlaying);
}
