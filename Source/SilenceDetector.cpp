#include "SilenceDetector.h"
#include "ControlPanel.h" // Full header required for implementation details.
#include "AudioPlayer.h"
#include "SilenceThresholdPresenter.h"
#include "SilenceAnalysisWorker.h"

/**
 * @file SilenceDetector.cpp
 * @brief Implements the SilenceDetector class for finding the start and end of audio content.
 *
 * This class provides functionality to automatically detect the first and last sounds in an audio
 * file, allowing for the automatic setting of loop points. It is designed to work within the context of a
 * `ControlPanel`, from which it accesses the `AudioPlayer` and other UI components. The core purpose is
 * to improve workflow efficiency by removing the need for manual trimming of silence from audio samples.
 */

/**
 * @brief Constructs the SilenceDetector and initializes its UI components.
 * @param ownerPanel A reference to the ControlPanel that owns this detector. This is necessary to
 *                   access shared resources like the AudioPlayer and UI display components.
 *
 * The constructor sets up the two text editors (`inSilenceThresholdEditor` and `outSilenceThresholdEditor`)
 * used for configuring the silence detection thresholds. It initializes their values from the global `Config`,
 * applies styling for a consistent look and feel, and attaches listeners to respond to user input.
 * The use of percentages in the UI (e.g., "5" for 5%) is a user-friendly abstraction over the internal
 * floating-point representation (0.05f).
 */
SilenceDetector::SilenceDetector(ControlPanel& ownerPanel)
    : owner(ownerPanel),
      currentInSilenceThreshold(Config::Audio::silenceThresholdIn),
      currentOutSilenceThreshold(Config::Audio::silenceThresholdOut)
{
    thresholdPresenter = std::make_unique<SilenceThresholdPresenter>(*this, owner);
}

/**
 * @brief Destructor for the SilenceDetector.
 *
 * Ensures that this object is safely removed as a listener from the text editors. This prevents
 * dangling pointers and potential crashes if the SilenceDetector is destroyed while the editors
 * are still active. It's a crucial part of JUCE's listener management pattern.
 */
SilenceDetector::~SilenceDetector() = default;

/**
 * @brief Detects the start of sound in the audio file and sets the loop start point.
 *
 * This function scans the audio file from the beginning to find the first sample that exceeds
 * the `currentInSilenceThreshold`. To do this efficiently, it first loads the entire audio
 * file into an in-memory `AudioBuffer`. This avoids slow, repeated disk access during the scan.
 * Playback is temporarily paused during the scan to ensure the audio data is not in a state of flux.
 * Upon finding the first sound, it updates the loop start position via the owner `ControlPanel`.
 */
void SilenceDetector::detectInSilence()
{
    SilenceAnalysisWorker::detectInSilence(owner, currentInSilenceThreshold);
}


/**
 * @brief Detects the end of sound in the audio file and sets the loop end point.
 *
 * This function works similarly to `detectInSilence` but scans the audio file backwards from
 * the end. Its goal is to find the last sample that exceeds the `currentOutSilenceThreshold`.
 * A small buffer (50ms) is added after the last detected sound. This is crucial for preserving
 * the natural decay or reverb tail of the audio, preventing an unnaturally abrupt cutoff.
 * Like the "in" detection, it operates on an in-memory buffer for performance and pauses
 * playback during analysis.
 */
void SilenceDetector::detectOutSilence()
{
    SilenceAnalysisWorker::detectOutSilence(owner, currentOutSilenceThreshold);
}


/**
 * @brief Provides real-time visual feedback as the user types in a threshold editor.
 * @param editor The TextEditor that has changed.
 *
 * This callback is triggered on every keystroke within the threshold editors. It checks if the
 * entered value (as a percentage) is within the valid range of 1-99. If it is, the text color
 * is normal. If not, the color changes to an "out of range" color. This immediate feedback
 * improves usability by guiding the user toward valid input without waiting for them to press
 * Enter or lose focus.
 */
