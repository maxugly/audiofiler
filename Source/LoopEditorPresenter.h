#ifndef AUDIOFILER_LOOPEDITORPRESENTER_H
#define AUDIOFILER_LOOPEDITORPRESENTER_H

#include <JuceHeader.h>

class ControlPanel;

/**
 * @class LoopEditorPresenter
 * @brief Configures and validates the loop start/end editors for ControlPanel.
 */
class LoopEditorPresenter final : private juce::TextEditor::Listener
{
public:
    explicit LoopEditorPresenter(ControlPanel& ownerPanel);
    ~LoopEditorPresenter() override;

    void initialiseEditors();

private:
    void textEditorTextChanged(juce::TextEditor& editor) override;
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;
    void textEditorEscapeKeyPressed(juce::TextEditor& editor) override;
    void textEditorFocusLost(juce::TextEditor& editor) override;

    void applyLoopEdit(juce::TextEditor& editor, bool isLoopIn);
    void restoreEditorValue(juce::TextEditor& editor, bool isLoopIn);

    ControlPanel& owner;
    juce::TextEditor& loopInEditor;
    juce::TextEditor& loopOutEditor;
};


#endif // AUDIOFILER_LOOPEDITORPRESENTER_H
