#include "SilenceDetector.h"
#include "ControlPanel.h" // Full header required for implementation details.
#include "AudioPlayer.h"

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
      currentInSilenceThreshold(Config::silenceThreshold),
      currentOutSilenceThreshold(Config::outSilenceThreshold)
{
    // Configure the 'In' silence threshold editor
    // This editor allows the user to define what level is considered "sound" at the beginning of a file.
    inSilenceThresholdEditor.setText(juce::String(static_cast<int>(currentInSilenceThreshold * 100.0f))); // Display as integer percentage
    inSilenceThresholdEditor.setReadOnly(false); // Added
    inSilenceThresholdEditor.setJustification(juce::Justification::centred);
    inSilenceThresholdEditor.setColour(juce::TextEditor::backgroundColourId, Config::textEditorBackgroundColour); // Added
    inSilenceThresholdEditor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Added
    inSilenceThresholdEditor.setFont(juce::Font(juce::FontOptions(Config::playbackTextSize))); // Added
    inSilenceThresholdEditor.applyFontToAllText(inSilenceThresholdEditor.getFont()); // Added
    inSilenceThresholdEditor.setMultiLine(false); // Added
    inSilenceThresholdEditor.setReturnKeyStartsNewLine(false); // Added
    inSilenceThresholdEditor.addListener(this);
    inSilenceThresholdEditor.setWantsKeyboardFocus(true); // Added
    inSilenceThresholdEditor.setTooltip("Threshold to detect start of sound (0.0 - 1.0)");
    inSilenceThresholdEditor.setSelectAllWhenFocused(true);

    // Configure the 'Out' silence threshold editor
    // This editor sets the threshold for finding the end of the sound, scanning backwards from the end of the file.
    outSilenceThresholdEditor.setText(juce::String(static_cast<int>(currentOutSilenceThreshold * 100.0f))); // Display as integer percentage
    outSilenceThresholdEditor.setReadOnly(false); // Added
    outSilenceThresholdEditor.setJustification(juce::Justification::centred);
    outSilenceThresholdEditor.setColour(juce::TextEditor::backgroundColourId, Config::textEditorBackgroundColour); // Added
    outSilenceThresholdEditor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Added
    outSilenceThresholdEditor.setFont(juce::Font(juce::FontOptions(Config::playbackTextSize))); // Added
    outSilenceThresholdEditor.applyFontToAllText(outSilenceThresholdEditor.getFont()); // Added
    outSilenceThresholdEditor.setMultiLine(false); // Added
    outSilenceThresholdEditor.setReturnKeyStartsNewLine(false); // Added
    outSilenceThresholdEditor.addListener(this);
    outSilenceThresholdEditor.setWantsKeyboardFocus(true); // Added
    outSilenceThresholdEditor.setTooltip("Threshold to detect end of sound (0.0 - 1.0)");
    outSilenceThresholdEditor.setSelectAllWhenFocused(true);
}

/**
 * @brief Destructor for the SilenceDetector.
 *
 * Ensures that this object is safely removed as a listener from the text editors. This prevents
 * dangling pointers and potential crashes if the SilenceDetector is destroyed while the editors
 * are still active. It's a crucial part of JUCE's listener management pattern.
 */
SilenceDetector::~SilenceDetector()
{
    inSilenceThresholdEditor.removeListener(this);
    outSilenceThresholdEditor.removeListener(this);
}

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
    AudioPlayer& audioPlayer = owner.getAudioPlayer();
    bool wasPlaying = audioPlayer.isPlaying();
    if (wasPlaying) {
        audioPlayer.getTransportSource().stop();
    }

    juce::AudioFormatReader* reader = audioPlayer.getAudioFormatReader();
    if (!reader)
    {
        owner.getStatsDisplay().insertTextAtCaret("No audio loaded to detect silence.\\n");
        if (wasPlaying) { audioPlayer.getTransportSource().start(); } // Resume if was playing
        return;
    }

    juce::int64 lengthInSamples = reader->lengthInSamples;
    owner.getStatsDisplay().insertTextAtCaret("SilenceDetector: Reading " + juce::String(lengthInSamples) + " samples for In detection.\\n");

    if (lengthInSamples == 0) // Added check for 0-length audio
    {
        owner.getStatsDisplay().insertTextAtCaret("SilenceDetector: Audio length is 0, cannot detect silence.\\n");
        if (wasPlaying) { audioPlayer.getTransportSource().start(); } // Resume if was playing
        return;
    }

    // Create a temporary buffer and read the entire audio into it for fast analysis.
    auto buffer = std::make_unique<juce::AudioBuffer<float>>(reader->numChannels, (int)lengthInSamples);
    reader->read(buffer.get(), 0, (int)lengthInSamples, 0, true, true);
    
    const int numSamples = buffer->getNumSamples();

    // Scan from the start to find the first sample above the threshold.
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Check across all channels for a non-silent sample.
        for (int channel = 0; channel < buffer->getNumChannels(); ++channel)
        {
            if (std::abs(buffer->getSample(channel, sample)) > currentInSilenceThreshold)
            {
                owner.setLoopStart(sample);
                owner.getStatsDisplay().insertTextAtCaret("Auto-set loop start to sample " + juce::String(sample) + " (" + owner.formatTime((double)sample / reader->sampleRate) + ")\\n");
                if (wasPlaying) { audioPlayer.getTransportSource().start(); } // Resume playback
                return; // Exit after finding the first non-silent sample.
            }
        }
    }
    owner.getStatsDisplay().insertTextAtCaret("Could not detect any sound at start.\\n");
    if (wasPlaying) { audioPlayer.getTransportSource().start(); } // Resume playback
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
    AudioPlayer& audioPlayer = owner.getAudioPlayer();
    bool wasPlaying = audioPlayer.isPlaying();
    if (wasPlaying) {
        audioPlayer.getTransportSource().stop();
    }

    juce::AudioFormatReader* reader = audioPlayer.getAudioFormatReader();
    if (!reader)
    {
        owner.getStatsDisplay().insertTextAtCaret("No audio loaded to detect silence.\\n");
        if (wasPlaying) { audioPlayer.getTransportSource().start(); } // Resume if was playing
        return;
    }

    juce::int64 lengthInSamples = reader->lengthInSamples;
    owner.getStatsDisplay().insertTextAtCaret("SilenceDetector: Reading " + juce::String(lengthInSamples) + " samples for Out detection.\\n");

    if (lengthInSamples == 0) // Added check for 0-length audio
    {
        owner.getStatsDisplay().insertTextAtCaret("SilenceDetector: Audio length is 0, cannot detect silence.\\n");
        if (wasPlaying) { audioPlayer.getTransportSource().start(); } // Resume if was playing
        return;
    }

    // Read audio into memory for fast reverse scanning.
    auto buffer = std::make_unique<juce::AudioBuffer<float>>(reader->numChannels, (int)lengthInSamples);
    reader->read(buffer.get(), 0, (int)lengthInSamples, 0, true, true);
    
    const int numSamples = buffer->getNumSamples();

    // Find the last sample that is not silence to avoid clipping the audio tail.
    for (int sample = numSamples - 1; sample >= 0; --sample)
    {
        // Check across all channels for a non-silent sample.
        for (int channel = 0; channel < buffer->getNumChannels(); ++channel)
        {
            if (std::abs(buffer->getSample(channel, sample)) > currentOutSilenceThreshold)
            {
                // Set loop end slightly after the last sound to include reverb tails.
                // This 50ms buffer provides a more natural-sounding loop end.
                int endPoint = sample + (int)(reader->sampleRate * 0.05); // 50ms buffer
                endPoint = juce::jmin(endPoint, numSamples);
                
                owner.setLoopEnd(endPoint);
                owner.getStatsDisplay().insertTextAtCaret("Auto-set loop end to sample " + juce::String(endPoint) + " (" + owner.formatTime((double)endPoint / reader->sampleRate) + ")\\n");
                if (wasPlaying) { audioPlayer.getTransportSource().start(); } // Resume playback
                return; // Exit after finding the last non-silent sample.
            }
        }
    }
    owner.getStatsDisplay().insertTextAtCaret("Could not detect any sound at end.\\n");
    if (wasPlaying) { audioPlayer.getTransportSource().start(); } // Resume playback
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
void SilenceDetector::textEditorTextChanged(juce::TextEditor& editor)
{
    // Implement dynamic text color feedback as per original ControlPanel behavior.
    int newPercentage = editor.getText().getIntValue();
    if (newPercentage >= 1 && newPercentage <= 99) {
        editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Valid
        editor.setColour(juce::TextEditor::backgroundColourId, owner.getLookAndFeel().findColour(juce::TextEditor::backgroundColourId)); // Reset background
    } else {
        editor.setColour(juce::TextEditor::textColourId, Config::textEditorOutOfRangeColour); // Out of range
        editor.setColour(juce::TextEditor::backgroundColourId, owner.getLookAndFeel().findColour(juce::TextEditor::backgroundColourId)); // Reset background
    }
}

/**
 * @brief Applies the new threshold when the user presses Enter.
 * @param editor The TextEditor where the Return key was pressed.
 *
 * This confirms the user's input and triggers the logic to update the threshold value.
 * It's a standard UI pattern for text field input.
 */
void SilenceDetector::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    DBG("textEditorReturnKeyPressed triggered for editor with text: " + editor.getText());
    applyThresholdFromEditor(editor);
}

/**
 * @brief Applies the new threshold when the editor loses focus.
 * @param editor The TextEditor that lost focus.
 *
 * This ensures that the value is applied even if the user just clicks away from the editor
 * instead of pressing Enter. It provides a more seamless and intuitive user experience.
 */
void SilenceDetector::textEditorFocusLost(juce::TextEditor& editor)
{
    DBG("textEditorFocusLost triggered for editor with text: " + editor.getText());
    applyThresholdFromEditor(editor);
}

/**
 * @brief Core logic for validating and applying a new threshold from an editor.
 * @param editor The TextEditor containing the new value to apply.
 *
 * This function is the central point for handling user input for thresholds. It parses the
 * percentage value, validates it is within the 1-99 range, and if so, updates the
 * corresponding `current...SilenceThreshold` member. A key feature is that if an "auto-cut"
 * mode is active, it immediately re-runs the detection, providing a responsive, live update.
 * If the input is invalid, it reverts the editor's text to the last valid value and provides
 * clear visual and textual feedback about the error.
 */
void SilenceDetector::applyThresholdFromEditor(juce::TextEditor& editor)
{
    float newThreshold = 0.0f;
    juce::String text = editor.getText();
    
    // Assume integer percentage input (1-99)
    int intValue = text.getIntValue();
    newThreshold = static_cast<float>(intValue) / 100.0f;

    // Validate that the threshold is within a sensible range (0.01 to 0.99 float, corresponding to 1-99 integer).
    // The original logic checked for 1-99%.
    if (intValue >= 1 && intValue <= 99) // Check integer percentage range
    {
        if (&editor == &inSilenceThresholdEditor)
        {
            currentInSilenceThreshold = newThreshold;
            if (getIsAutoCutInActive()) // If auto-cut-in is active, re-run detection
                detectInSilence();
        }
        else if (&editor == &outSilenceThresholdEditor)
        {
            currentOutSilenceThreshold = newThreshold;
            if (getIsAutoCutOutActive()) // If auto-cut-out is active, re-run detection
                detectOutSilence();
        }

        // On success, reset text color to playbackTextColor and background to default
        editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
        editor.setColour(juce::TextEditor::backgroundColourId, owner.getLookAndFeel().findColour(juce::TextEditor::backgroundColourId)); // Reset background color on success
        editor.setText(juce::String(intValue), juce::dontSendNotification); // Display as integer
    }
    else
    {
        // On failure (out of range 1-99), restore the last valid value and show a warning color.
        float oldValue = (&editor == &inSilenceThresholdEditor) ? currentInSilenceThreshold : currentOutSilenceThreshold;
        
        // Display old value as integer percentage
        editor.setText(juce::String(static_cast<int>(oldValue * 100.0f)), juce::dontSendNotification);

        // Visual feedback based on validity (out of range here)
        editor.setColour(juce::TextEditor::textColourId, Config::textEditorWarningColor); // Use warning for out of range
        owner.getStatsDisplay().insertTextAtCaret("Warning: Threshold value must be between 1 and 99. Restored to last valid value.\\n");
    }
}