#pragma once

#include <JuceHeader.h>
#include "Config.h"

// Forward declaration to avoid circular header dependencies.
class ControlPanel;

/**
 * @file SilenceDetector.h
 * @brief Defines the SilenceDetector class for managing silence detection logic and UI.
 *
 * This class encapsulates the functionality for detecting periods of silence at the
 * beginning and end of an audio sample. It also owns and manages the UI components
 * (TextEditors) for setting the silence thresholds, decoupling this specific
 * functionality from the main `ControlPanel`.
 */
class SilenceDetector : public juce::TextEditor::Listener
{
public:
    /**
     * @brief Constructs a SilenceDetector.
     * @param owner The ControlPanel that owns this detector. This is required to
     *              access the AudioPlayer and to update the main application state
     *              (e.g., setting loop points).
     */
    explicit SilenceDetector(ControlPanel& owner);

    /**
     * @brief Destructor.
     *
     * Ensures this object is correctly removed as a listener from its
     * TextEditors to prevent dangling pointers and crashes.
     */
    ~SilenceDetector() override;

    //==============================================================================
    /** @name Core Detection Logic
     *  Methods for running the silence detection algorithms.
     *  @{
     */

    /**
     * @brief Finds the first audible sound to automatically set the loop start point.
     *
     * This method scans the audio file from the beginning, sample by sample,
     * to find the first moment the audio level exceeds the `currentInSilenceThreshold`.
     * If successful, it instructs the parent `ControlPanel` to update the loop start time.
     */
    void detectInSilence();

    /**
     * @brief Finds the last audible sound to automatically set the loop end point.
     *
     * This method scans the audio file backwards from the end to find the last
     * moment the audio level exceeds the `currentOutSilenceThreshold`. This is useful
     * for trimming silence from the end of a sample while preserving the natural
     * decay (reverb tail) of the sound.
     */
    void detectOutSilence();

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name UI Component Access
     *  Getters for the UI elements owned by this class.
     *  @{
     */

    /** @brief Returns a reference to the TextEditor for the 'in' silence threshold. */
    juce::TextEditor& getInSilenceThresholdEditor() { return inSilenceThresholdEditor; }

    /** @brief Returns a reference to the TextEditor for the 'out' silence threshold. */
    juce::TextEditor& getOutSilenceThresholdEditor() { return outSilenceThresholdEditor; }

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name State Getters and Setters
     *  Methods for accessing and modifying the internal state of the detector.
     *  @{
     */

    /** @brief Gets the current threshold for detecting the start of sound.
     *  @return The threshold as a normalized float (0.0 to 1.0).
     */
    float getCurrentInSilenceThreshold() const { return currentInSilenceThreshold; }

    /** @brief Gets the current threshold for detecting the end of sound.
     *  @return The threshold as a normalized float (0.0 to 1.0).
     */
    float getCurrentOutSilenceThreshold() const { return currentOutSilenceThreshold; }

    /** @brief Checks if the "auto-cut in" feature is currently active.
     *  @return True if active, false otherwise.
     */
    bool getIsAutoCutInActive() const { return m_isAutoCutInActive; }

    /** @brief Sets the "auto-cut in" feature's active state.
     *  @param value The new state for the feature.
     */
    void setIsAutoCutInActive(bool value) { m_isAutoCutInActive = value; }

    /** @brief Checks if the "auto-cut out" feature is currently active.
     *  @return True if active, false otherwise.
     */
    bool getIsAutoCutOutActive() const { return m_isAutoCutOutActive; }

    /** @brief Sets the "auto-cut out" feature's active state.
     *  @param value The new state for the feature.
     */
    void setIsAutoCutOutActive(bool value) { m_isAutoCutOutActive = value; }

    /** @} */
    //==============================================================================

private:
    //==============================================================================
    /** @name juce::TextEditor::Listener Overrides
     *  Private callbacks to handle UI events from the threshold editors.
     *  @{
     */

    /**
     * @brief Called when the text in an editor changes. Provides real-time validation feedback.
     * @param editor A reference to the editor that was modified.
     */
    void textEditorTextChanged(juce::TextEditor& editor) override;

    /**
     * @brief Called when the user presses Return in an editor. Finalizes the value.
     * @param editor A reference to the editor where Return was pressed.
     */
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;

    /**
     * @brief Called when an editor loses focus. Finalizes the value.
     * @param editor A reference to the editor that lost focus.
     */
    void textEditorFocusLost(juce::TextEditor& editor) override;

    /** @} */
    //==============================================================================

    /**
     * @brief Validates and applies the threshold value from a TextEditor.
     *
     * This is the internal logic triggered by `textEditorReturnKeyPressed` or
     * `textEditorFocusLost`. It parses the text, validates it's within a sensible
     * range (1-99%), and updates the internal threshold state. If an auto-cut
     * feature is active, it will also trigger the corresponding detection method.
     * @param editor The TextEditor containing the new threshold value to apply.
     */
    void applyThresholdFromEditor(juce::TextEditor& editor);
    
    //==============================================================================
    // Member Variables
    //==============================================================================
    
    /// @brief A reference to the owning ControlPanel, for callbacks and state access.
    ControlPanel& owner;

    // --- UI Components ---
    juce::TextEditor inSilenceThresholdEditor;  ///< Editor for the 'in' silence threshold (e.g., 1-99%).
    juce::TextEditor outSilenceThresholdEditor; ///< Editor for the 'out' silence threshold (e.g., 1-99%).

    // --- Internal State ---
    float currentInSilenceThreshold;  ///< Normalized (0.0-1.0) threshold for detecting the start of sound.
    float currentOutSilenceThreshold; ///< Normalized (0.0-1.0) threshold for detecting the end of sound.
    bool m_isAutoCutInActive = false;  ///< If true, `detectInSilence` is run automatically when the threshold changes.
    bool m_isAutoCutOutActive = false; ///< If true, `detectOutSilence` is run automatically when the threshold changes.
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SilenceDetector)
};