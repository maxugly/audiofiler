#ifndef AUDIOFILER_SILENCETHRESHOLDPRESENTER_H
#define AUDIOFILER_SILENCETHRESHOLDPRESENTER_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

class ControlPanel;
class SilenceDetector;

/**
 * @class SilenceThresholdPresenter
 * @brief Manages the silence threshold editors, validation, and auto-cut re-triggers.
 */
class SilenceThresholdPresenter final : private juce::TextEditor::Listener
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

    void textEditorTextChanged(juce::TextEditor& editor) override;
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;
    void textEditorFocusLost(juce::TextEditor& editor) override;

    void applyThresholdFromEditor(juce::TextEditor& editor);
    void updateThresholdFromEditorIfValid(juce::TextEditor& editor);
    void restoreEditorToCurrentValue(juce::TextEditor& editor);
    bool isInEditor(const juce::TextEditor& editor) const noexcept;
    bool isValidPercentage(int value) const noexcept { return value >= 1 && value <= 99; }

    SilenceDetector& detector;
    ControlPanel& owner;
};

#endif // AUDIOFILER_SILENCETHRESHOLDPRESENTER_H
