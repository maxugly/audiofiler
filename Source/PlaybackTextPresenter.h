#ifndef AUDIOFILER_PLAYBACKTEXTPRESENTER_H
#define AUDIOFILER_PLAYBACKTEXTPRESENTER_H

#include <JuceHeader.h>

class ControlPanel;

/**
 * @class PlaybackTextPresenter
 * @brief Formats and renders the playback timing labels and handles their interaction.
 */
class PlaybackTextPresenter : public juce::TextEditor::Listener,
                              public juce::MouseListener
{
public:
    explicit PlaybackTextPresenter(ControlPanel& ownerPanel);
    ~PlaybackTextPresenter() override;

    void initialiseEditors();
    void updateEditors();
    void layoutEditors();

    void render(juce::Graphics& g) const;
    void setTotalTimeStaticString(const juce::String& text) { totalTimeStaticStr = text; }
    const juce::String& getTotalTimeStaticString() const { return totalTimeStaticStr; }

private:
    // juce::TextEditor::Listener overrides
    void textEditorTextChanged(juce::TextEditor& editor) override;
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;
    void textEditorEscapeKeyPressed(juce::TextEditor& editor) override;
    void textEditorFocusLost(juce::TextEditor& editor) override;
    void textEditorFocusGained(juce::TextEditor& editor);

    // juce::MouseListener overrides
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;
    void mouseUp(const juce::MouseEvent& event) override;

    void applyTimeEdit(juce::TextEditor& editor);
    void syncEditorToPosition(juce::TextEditor& editor, double positionSeconds, bool isRemaining = false);

    juce::String buildCutLengthText() const;

    ControlPanel& owner;
    juce::String totalTimeStaticStr;
};

#endif // AUDIOFILER_PLAYBACKTEXTPRESENTER_H
