#include "SilenceAnalysisWorker.h"
#include "SilenceAnalysisAlgorithms.h"

#include "ControlPanel.h"
#include "AudioPlayer.h"
#include "SilenceDetectionLogger.h"
#include <limits>
#include <algorithm>
#include <cmath>

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

    // Security Fix: Check for invalid length to prevent processing errors
    if (lengthInSamples <= 0)
    {
        SilenceDetectionLogger::logZeroLength(ownerPanel);
        resumeIfNeeded(audioPlayer, wasPlaying);
        return;
    }

    // Note: DoS check for kMaxAnalyzableSamples removed as chunking handles large files safely

    const juce::int64 result = SilenceAnalysisAlgorithms::findSilenceIn(*reader, threshold);

    if (result != -1)
    {
        ownerPanel.setLoopInPosition((double)result / reader->sampleRate);
        SilenceDetectionLogger::logLoopStartSet(ownerPanel, result, reader->sampleRate);

        // Move playhead to the new loop-in position in cut mode
        if (ownerPanel.isCutModeActive())
            audioPlayer.getTransportSource().setPosition(ownerPanel.getLoopInPosition());
    }
    else
    {
        SilenceDetectionLogger::logNoSoundFound(ownerPanel, "start");
    }

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

    // Note: DoS check for kMaxAnalyzableSamples removed as chunking handles large files safely

    const juce::int64 result = SilenceAnalysisAlgorithms::findSilenceOut(*reader, threshold);

    if (result != -1)
    {
        const juce::int64 tailSamples = (juce::int64) (reader->sampleRate * 0.05); // 50ms tail
        const juce::int64 endPoint64 = result + tailSamples;
        const juce::int64 finalEndPoint = std::min(endPoint64, lengthInSamples);

        ownerPanel.setLoopOutPosition((double)finalEndPoint / reader->sampleRate);
        SilenceDetectionLogger::logLoopEndSet(ownerPanel, finalEndPoint, reader->sampleRate);
    }
    else
    {
        SilenceDetectionLogger::logNoSoundFound(ownerPanel, "end");
    }

    resumeIfNeeded(audioPlayer, wasPlaying);
}
