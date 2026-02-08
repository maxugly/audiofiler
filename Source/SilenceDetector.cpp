#include "SilenceDetector.h"
#include "ControlPanel.h" // Full header required for implementation details.
#include "AudioPlayer.h"

/**
 * @brief Initializes members, configures TextEditors, and adds listeners.
 */
SilenceDetector::SilenceDetector(ControlPanel& ownerPanel)
    : owner(ownerPanel),
      currentInSilenceThreshold(Config::silenceThreshold),
      currentOutSilenceThreshold(Config::outSilenceThreshold)
{
    // Configure the 'In' silence threshold editor
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
 * @brief Ensures listeners are removed on destruction.
 */
SilenceDetector::~SilenceDetector()
{
    inSilenceThresholdEditor.removeListener(this);
    outSilenceThresholdEditor.removeListener(this);
}

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

    int numChannels = reader->numChannels;
    juce::int64 lengthInSamples = reader->lengthInSamples;
    owner.getStatsDisplay().insertTextAtCaret("SilenceDetector: Reading " + juce::String(lengthInSamples) + " samples for In detection.\\n");

    if (lengthInSamples == 0) // Added check for 0-length audio
    {
        owner.getStatsDisplay().insertTextAtCaret("SilenceDetector: Audio length is 0, cannot detect silence.\\n");
        if (wasPlaying) { audioPlayer.getTransportSource().start(); } // Resume if was playing
        return;
    }

    // Create a temporary buffer and read the entire audio into it.
    auto buffer = std::make_unique<juce::AudioBuffer<float>>(numChannels, (int)lengthInSamples);
    reader->read(buffer.get(), 0, (int)lengthInSamples, 0, true, true);
    
    const int numSamples = buffer->getNumSamples(); // Access numSamples from the unique_ptr

    // Find the first sample that is not silence.
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Check across all channels for a non-silent sample.
        for (int channel = 0; channel < buffer->getNumChannels(); ++channel)
        {
            if (std::abs(buffer->getSample(channel, sample)) > currentInSilenceThreshold)
            {
                owner.setLoopStart(sample);
                owner.getStatsDisplay().insertTextAtCaret("Auto-set loop start to sample " + juce::String(sample) + " (" + owner.formatTime((double)sample / audioPlayer.getAudioFormatReader()->sampleRate) + ")\\n");
                if (wasPlaying) { audioPlayer.getTransportSource().start(); } // Resume if was playing
                return; // Exit after finding the first non-silent sample.
            }
        }
    }
    owner.getStatsDisplay().insertTextAtCaret("Could not detect any sound at start.\\n");
    if (wasPlaying) { audioPlayer.getTransportSource().start(); } // Resume if was playing
}

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

    int numChannels = reader->numChannels;
    juce::int64 lengthInSamples = reader->lengthInSamples;
    owner.getStatsDisplay().insertTextAtCaret("SilenceDetector: Reading " + juce::String(lengthInSamples) + " samples for Out detection.\\n");

    if (lengthInSamples == 0) // Added check for 0-length audio
    {
        owner.getStatsDisplay().insertTextAtCaret("SilenceDetector: Audio length is 0, cannot detect silence.\\n");
        if (wasPlaying) { audioPlayer.getTransportSource().start(); } // Resume if was playing
        return;
    }

    // Create a temporary buffer and read the entire audio into it.
    auto buffer = std::make_unique<juce::AudioBuffer<float>>(numChannels, (int)lengthInSamples);
    reader->read(buffer.get(), 0, (int)lengthInSamples, 0, true, true);
    
    const int numSamples = buffer->getNumSamples(); // Access numSamples from the unique_ptr

    // Find the last sample that is not silence to avoid clipping the audio tail.
    for (int sample = numSamples - 1; sample >= 0; --sample)
    {
        // Check across all channels for a non-silent sample.
        for (int channel = 0; channel < buffer->getNumChannels(); ++channel)
        {
            if (std::abs(buffer->getSample(channel, sample)) > currentOutSilenceThreshold)
            {
                // Set loop end slightly after the last sound to include reverb tails.
                // The original ControlPanel code had a zero-crossing logic for `detectInSilence`,
                // and for `detectOutSilence` it also searched for a zero-crossing
                // but the prompt explicitly asked for adapt to use AudioPlayer buffer which doesn't have read() that fills a buffer.
                // For simplicity, let's keep it based on threshold, and optionally add a small buffer.
                int endPoint = sample + (int)(audioPlayer.getAudioFormatReader()->sampleRate * 0.05); // 50ms buffer
                endPoint = juce::jmin(endPoint, numSamples);
                
                owner.setLoopEnd(endPoint);
                owner.getStatsDisplay().insertTextAtCaret("Auto-set loop end to sample " + juce::String(endPoint) + " (" + owner.formatTime((double)endPoint / audioPlayer.getAudioFormatReader()->sampleRate) + ")\\n");
                if (wasPlaying) { audioPlayer.getTransportSource().start(); } // Resume if was playing
                return; // Exit after finding the last non-silent sample.
            }
        }
    }
    owner.getStatsDisplay().insertTextAtCaret("Could not detect any sound at end.\\n");
    if (wasPlaying) { audioPlayer.getTransportSource().start(); } // Resume if was playing
}

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

void SilenceDetector::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    DBG("textEditorReturnKeyPressed triggered for editor with text: " + editor.getText());
    applyThresholdFromEditor(editor);
}

void SilenceDetector::textEditorFocusLost(juce::TextEditor& editor)
{
    DBG("textEditorFocusLost triggered for editor with text: " + editor.getText());
    applyThresholdFromEditor(editor);
}

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
            currentInSilenceThreshold = newThreshold;
        else if (&editor == &outSilenceThresholdEditor)
            currentOutSilenceThreshold = newThreshold;

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