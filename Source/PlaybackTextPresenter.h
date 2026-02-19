#ifndef AUDIOFILER_PLAYBACKTEXTPRESENTER_H
#define AUDIOFILER_PLAYBACKTEXTPRESENTER_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

class ControlPanel;

/**
 * @class PlaybackTextPresenter
 * @brief Formats and renders the playback timing labels and handles their
 * interaction.
 */
class PlaybackTextPresenter : public juce::TextEditor::Listener,
                              public juce::MouseListener {
public:
  explicit PlaybackTextPresenter(ControlPanel &ownerPanel);
  ~PlaybackTextPresenter() override;

  void initialiseEditors();
  void updateEditors();
  void layoutEditors();

  void render(juce::Graphics &g) const;
  void setTotalTimeStaticString(const juce::String &text) {
    totalTimeStaticStr = text;
  }
  const juce::String &getTotalTimeStaticString() const {
    return totalTimeStaticStr;
  }

private:
  void textEditorTextChanged(juce::TextEditor &editor) override;
  void textEditorReturnKeyPressed(juce::TextEditor &editor) override;
  void textEditorEscapeKeyPressed(juce::TextEditor &editor) override;
  void textEditorFocusLost(juce::TextEditor &editor) override;

  void mouseWheelMove(const juce::MouseEvent &event,
                      const juce::MouseWheelDetails &wheel) override;
  void mouseUp(const juce::MouseEvent &event) override;

  void applyTimeEdit(juce::TextEditor &editor);
  void syncEditorToPosition(juce::TextEditor &editor, double positionSeconds,
                            bool isRemaining = false);

  ControlPanel &owner;
  juce::String totalTimeStaticStr;

  bool isEditingElapsed{false};
  bool isEditingRemaining{false};
  bool isEditingCutLength{false};

  void mouseDown(const juce::MouseEvent &event) override;
};

#endif // AUDIOFILER_PLAYBACKTEXTPRESENTER_H
