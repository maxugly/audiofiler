#include "SilenceDetectionLogger.h"

#include "ControlPanel.h"

void SilenceDetectionLogger::logNoAudioLoaded(ControlPanel& panel)
{
    panel.getStatsDisplay().insertTextAtCaret("No audio loaded to detect silence.\n");
}

void SilenceDetectionLogger::logReadingSamples(ControlPanel& panel,
                                               const juce::String& direction,
                                               juce::int64 length)
{
    panel.getStatsDisplay().insertTextAtCaret("SilenceDetector: Reading "
        + juce::String(length)
        + " samples for " + direction + " detection.\n");
}

void SilenceDetectionLogger::logZeroLength(ControlPanel& panel)
{
    panel.getStatsDisplay().insertTextAtCaret("SilenceDetector: Audio length is 0, cannot detect silence.\n");
}

void SilenceDetectionLogger::logLoopStartSet(ControlPanel& panel, juce::int64 sampleIndex, double sampleRate)
{
    panel.getStatsDisplay().insertTextAtCaret("Auto-set loop start to sample "
        + juce::String(sampleIndex)
        + " (" + panel.formatTime((double) sampleIndex / sampleRate) + ")\n");
}

void SilenceDetectionLogger::logLoopEndSet(ControlPanel& panel, juce::int64 sampleIndex, double sampleRate)
{
    panel.getStatsDisplay().insertTextAtCaret("Auto-set loop end to sample "
        + juce::String(sampleIndex)
        + " (" + panel.formatTime((double) sampleIndex / sampleRate) + ")\n");
}

void SilenceDetectionLogger::logNoSoundFound(ControlPanel& panel, const juce::String& boundaryDescription)
{
    panel.getStatsDisplay().insertTextAtCaret("Could not detect any sound at " + boundaryDescription + ".\n");
}

void SilenceDetectionLogger::logAudioTooLarge(ControlPanel& panel)
{
    panel.getStatsDisplay().insertTextAtCaret("SilenceDetector: Audio file is too large for automated silence detection.\n");
}
