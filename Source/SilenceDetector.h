#pragma once

#include <JuceHeader.h>
#include "Config.h"

// Forward declaration to avoid circular header dependencies.
class ControlPanel;

/**
 * @class SilenceDetector
 * @brief Manages silence detection logic and associated UI components.
 *
 * @details This class encapsulates the functionality for detecting silence at the start
 * and end of an audio sample. It owns the UI elements (TextEditors) for
 * setting silence thresholds and handles the detection logic, decoupling these
 * concerns from the main ControlPanel per the Single Responsibility Principle.
 */
class SilenceDetector : public juce::TextEditor::Listener
{
public:
    /**
     * @brief Constructs a SilenceDetector.
     * @param owner The ControlPanel that owns this detector, used to access
     *              the AudioPlayer and other shared UI components like the stats display.
     */
    explicit SilenceDetector(ControlPanel& owner);

    /**
     * @brief Destructor. Removes listeners to prevent dangling pointers.
     */
    ~SilenceDetector() override;

    /**
     * @brief Finds the first audible sound and sets the audio loop start point.
     * @details Scans from the beginning of the audio buffer to find the first
     *          sample that exceeds the defined 'in' silence threshold. It communicates
     *          the result back to the owner ControlPanel.
     */
    void detectInSilence();

    /**
     * @brief Finds the last audible sound and sets the audio loop end point.
     * @details Scans from the end of the audio buffer backwards to find the last
     *          sample that exceeds the defined 'out' silence threshold, which helps
     *          avoid clipping the natural decay of the sound.
     */
    void detectOutSilence();

    //================================s==============================================
    // juce::TextEditor::Listener overrides
    /**
     * @brief Callback for when a listened-to TextEditor's text changes.
     * @param editor A reference to the editor that triggered the callback.
     */
    void textEditorTextChanged(juce::TextEditor& editor) override;

    /**
     * @brief Callback for when return is pressed in a TextEditor.
     * @param editor A reference to the editor that triggered the callback.
     */
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;

    /**
     * @brief Callback for when a TextEditor loses focus.
     * @param editor A reference to the editor that triggered the callback.
     */
    void textEditorFocusLost(juce::TextEditor& editor) override;

    /**
     * @brief Returns a reference to the inSilenceThresholdEditor.
     * @return A reference to the inSilenceThresholdEditor.
     */
    juce::TextEditor& getInSilenceThresholdEditor() { return inSilenceThresholdEditor; }

    /**
     * @brief Returns a reference to the outSilenceThresholdEditor.
     * @return A reference to the outSilenceThresholdEditor.
     */
    juce::TextEditor& getOutSilenceThresholdEditor() { return outSilenceThresholdEditor; }

    // --- Member Components ---
    juce::TextEditor inSilenceThresholdEditor;
    juce::TextEditor outSilenceThresholdEditor;
    
    // --- Getters for internal state ---
    float getCurrentInSilenceThreshold() const { return currentInSilenceThreshold; }
    float getCurrentOutSilenceThreshold() const { return currentOutSilenceThreshold; }

    /**
     * @brief Returns whether auto-cut in is currently active.
     * @return True if auto-cut in is active, false otherwise.
     */
    bool getIsAutoCutInActive() const { return m_isAutoCutInActive; }

    /**
     * @brief Sets whether auto-cut in should be active.
     * @param value True to activate, false to deactivate.
     */
    void setIsAutoCutInActive(bool value) { m_isAutoCutInActive = value; }

    /**
     * @brief Returns whether auto-cut out is currently active.
     * @return True if auto-cut out is active, false otherwise.
     */
    bool getIsAutoCutOutActive() const { return m_isAutoCutOutActive; }

    /**
     * @brief Sets whether auto-cut out should be active.
     * @param value True to activate, false to deactivate.
     */
    void setIsAutoCutOutActive(bool value) { m_isAutoCutOutActive = value; }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SilenceDetector)

private:
    /**
     * @brief Validates and applies the threshold value from a TextEditor.
     * @details This is the single point of entry for updating threshold values from the UI,
     *          ensuring input is validated and state is updated consistently.
     * @param editor The TextEditor containing the new threshold value.
     */
    void applyThresholdFromEditor(juce::TextEditor& editor);

    /// @brief A reference to the owning ControlPanel to access the AudioPlayer and other components.
    ControlPanel& owner;

    // --- Internal State ---
    float currentInSilenceThreshold;
    float currentOutSilenceThreshold;
    bool m_isAutoCutInActive = false;
    bool m_isAutoCutOutActive = false;
};