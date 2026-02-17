#include "SilenceDetectionLogger.h"
#include "TimeUtils.h"

// Note: We use SilenceWorkerClient instead of ControlPanel to allow mocking/decoupling.
// However, the logger previously called 'getStatsDisplay()' which returned a TextEditor.
// The interface 'SilenceWorkerClient' has 'logStatusMessage(String, bool)'.
// So we should map these helper functions to 'logStatusMessage'.

void SilenceDetectionLogger::logNoAudioLoaded(SilenceWorkerClient& client)
{
    client.logStatusMessage("No audio loaded to detect silence.", true);
}

void SilenceDetectionLogger::logReadingSamples(SilenceWorkerClient& client,
                                               const juce::String& direction,
                                               juce::int64 length)
{
    client.logStatusMessage("SilenceDetector: Reading "
        + juce::String(length)
        + " samples for " + direction + " detection.");
}

void SilenceDetectionLogger::logZeroLength(SilenceWorkerClient& client)
{
    client.logStatusMessage("SilenceDetector: Audio length is 0, cannot detect silence.", true);
}

void SilenceDetectionLogger::logLoopStartSet(SilenceWorkerClient& client, juce::int64 sampleIndex, double sampleRate)
{
    client.logStatusMessage("Auto-set loop start to sample "
        + juce::String(sampleIndex)
        + " (" + TimeUtils::formatTime((double) sampleIndex / sampleRate) + ")");
}

void SilenceDetectionLogger::logLoopEndSet(SilenceWorkerClient& client, juce::int64 sampleIndex, double sampleRate)
{
    client.logStatusMessage("Auto-set loop end to sample "
        + juce::String(sampleIndex)
        + " (" + TimeUtils::formatTime((double) sampleIndex / sampleRate) + ")");
}

void SilenceDetectionLogger::logNoSoundFound(SilenceWorkerClient& client, const juce::String& boundaryDescription)
{
    client.logStatusMessage("Could not detect any sound at " + boundaryDescription + ".");
}

void SilenceDetectionLogger::logAudioTooLarge(SilenceWorkerClient& client)
{
    client.logStatusMessage("SilenceDetector: Audio file is too large for automated silence detection.", true);
}
