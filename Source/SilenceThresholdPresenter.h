

#ifndef AUDIOFILER_SILENCETHRESHOLDPRESENTER_H
#define AUDIOFILER_SILENCETHRESHOLDPRESENTER_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

class ControlPanel;

class SilenceDetector;

class SilenceThresholdPresenter final : private juce::TextEditor::Listener,
                                 public juce::MouseListener
{
public:

    SilenceThresholdPresenter(SilenceDetector& detectorIn, ControlPanel& ownerPanel);

    ~SilenceThresholdPresenter() override;

private:
    void configureEditor(juce::TextEditor& editor,
                         float initialValue,
                         const juce::String& tooltip);

    void textEditorTextChanged(juce::TextEditor& editor) override;

    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;

    void textEditorFocusLost(juce::TextEditor& editor) override;

    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

    void applyThresholdFromEditor(juce::TextEditor& editor);

    void updateThresholdFromEditorIfValid(juce::TextEditor& editor);

    void restoreEditorToCurrentValue(juce::TextEditor& editor);
    bool isInEditor(const juce::TextEditor& editor) const noexcept;
    bool isValidPercentage(int value) const noexcept { return value >= 1 && value <= 99; }

    SilenceDetector& detector;
    ControlPanel& owner;
};

#endif 
