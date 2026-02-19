

#include "SilenceDetectionLogger.h"
#include "TimeUtils.h"

void SilenceDetectionLogger::logNoAudioLoaded(SilenceWorkerClient& client)
{
    client.logStatusMessage("No audio loaded to detect silence.", true);
}

void SilenceDetectionLogger::logReadingSamples(SilenceWorkerClient& client,
                                               const juce::String& direction,
                                               juce::int64 length)
{
    client.logStatusMessage("SilenceDetector: Scanning "
        + juce::String(length)
        + " samples for " + direction + " Silence Boundary.");
}

void SilenceDetectionLogger::logZeroLength(SilenceWorkerClient& client)
{
    client.logStatusMessage("SilenceDetector: Audio length is 0, cannot detect Silence Boundaries.", true);
}

void SilenceDetectionLogger::logCutInSet(SilenceWorkerClient& client, juce::int64 sampleIndex, double sampleRate)
{
    client.logStatusMessage("Silence Boundary (Start) set to sample "
        + juce::String(sampleIndex)
        + " (" + TimeUtils::formatTime((double) sampleIndex / sampleRate) + ")");
}

void SilenceDetectionLogger::logCutOutSet(SilenceWorkerClient& client, juce::int64 sampleIndex, double sampleRate)
{
    client.logStatusMessage("Silence Boundary (End) set to sample "
        + juce::String(sampleIndex)
        + " (" + TimeUtils::formatTime((double) sampleIndex / sampleRate) + ")");
}

void SilenceDetectionLogger::logNoSoundFound(SilenceWorkerClient& client, const juce::String& boundaryDescription)
{
    client.logStatusMessage("Could not detect any sound at " + boundaryDescription + " Silence Boundary.");
}

void SilenceDetectionLogger::logAudioTooLarge(SilenceWorkerClient& client)
{
    client.logStatusMessage("SilenceDetector: Audio file is too large for automated Cut Point detection.", true);
}
