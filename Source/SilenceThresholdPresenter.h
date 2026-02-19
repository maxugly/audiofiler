/**
 * @file SilenceThresholdPresenter.h
 * @brief Defines the SilenceThresholdPresenter class.
 * @ingroup Presenters
 */

#ifndef AUDIOFILER_SILENCETHRESHOLDPRESENTER_H
#define AUDIOFILER_SILENCETHRESHOLDPRESENTER_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

/**
 * @class ControlPanel
 * @brief Home: View.
 *
 */
class ControlPanel;
/**
 * @class SilenceDetector
 * @brief Home: Engine.
 *
 */
class SilenceDetector;

/**
 * @class SilenceThresholdPresenter
 * @brief Manages the silence threshold editors, validation, and auto-cut re-triggers.
 */
class SilenceThresholdPresenter final : private juce::TextEditor::Listener,
                                 public juce::MouseListener
{
public:
    /**
     * @brief Constructs the presenter and configures the threshold editors.
     * @param detectorIn Owning SilenceDetector that stores threshold state.
     * @param ownerPanel ControlPanel used for look-and-feel and messaging.
     */
    SilenceThresholdPresenter(SilenceDetector& detectorIn, ControlPanel& ownerPanel);

    /**
     * @brief Removes listeners from the editors on destruction.
     */
    ~SilenceThresholdPresenter() override;

private:
    void configureEditor(juce::TextEditor& editor,
                         float initialValue,
                         const juce::String& tooltip);

    /**
     * @brief Undocumented method.
     * @param editor [in] Description for editor.
     */
    void textEditorTextChanged(juce::TextEditor& editor) override;
    /**
     * @brief Undocumented method.
     * @param editor [in] Description for editor.
     */
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;
    /**
     * @brief Undocumented method.
     * @param editor [in] Description for editor.
     */
    void textEditorFocusLost(juce::TextEditor& editor) override;

    /**
     * @brief Undocumented method.
     * @param event [in] Description for event.
     * @param wheel [in] Description for wheel.
     */
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

    /**
     * @brief Undocumented method.
     * @param editor [in] Description for editor.
     */
    void applyThresholdFromEditor(juce::TextEditor& editor);
    /**
     * @brief Undocumented method.
     * @param editor [in] Description for editor.
     */
    void updateThresholdFromEditorIfValid(juce::TextEditor& editor);
    /**
     * @brief Undocumented method.
     * @param editor [in] Description for editor.
     */
    void restoreEditorToCurrentValue(juce::TextEditor& editor);
    bool isInEditor(const juce::TextEditor& editor) const noexcept;
    bool isValidPercentage(int value) const noexcept { return value >= 1 && value <= 99; }

    SilenceDetector& detector;
    ControlPanel& owner;
};

#endif 
