/**
 * @file PlaybackTextPresenter.h
 * @brief Defines the PlaybackTextPresenter class.
 * @ingroup Presenters
 */

#ifndef AUDIOFILER_PLAYBACKTEXTPRESENTER_H
#define AUDIOFILER_PLAYBACKTEXTPRESENTER_H

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
 * @class PlaybackTextPresenter
 * @brief Formats and renders the playback timing labels and handles their
 * interaction.
 */
class PlaybackTextPresenter : public juce::TextEditor::Listener,
                              public juce::MouseListener {
public:
  /**
   * @brief Undocumented method.
   * @param ownerPanel [in] Description for ownerPanel.
   */
  explicit PlaybackTextPresenter(ControlPanel &ownerPanel);
  /**
   * @brief Undocumented method.
   */
  ~PlaybackTextPresenter() override;

  /**
   * @brief Undocumented method.
   */
  void initialiseEditors();
  /**
   * @brief Undocumented method.
   */
  void updateEditors();
  /**
   * @brief Undocumented method.
   */
  void layoutEditors();

  /**
   * @brief Undocumented method.
   * @param g [in] Description for g.
   */
  void render(juce::Graphics &g) const;
  /**
   * @brief Sets the TotalTimeStaticString.
   * @param text [in] Description for text.
   */
  void setTotalTimeStaticString(const juce::String &text) {
    totalTimeStaticStr = text;
  }
  const juce::String &getTotalTimeStaticString() const {
    return totalTimeStaticStr;
  }

private:
  /**
   * @brief Undocumented method.
   * @param editor [in] Description for editor.
   */
  void textEditorTextChanged(juce::TextEditor &editor) override;
  /**
   * @brief Undocumented method.
   * @param editor [in] Description for editor.
   */
  void textEditorReturnKeyPressed(juce::TextEditor &editor) override;
  /**
   * @brief Undocumented method.
   * @param editor [in] Description for editor.
   */
  void textEditorEscapeKeyPressed(juce::TextEditor &editor) override;
  /**
   * @brief Undocumented method.
   * @param editor [in] Description for editor.
   */
  void textEditorFocusLost(juce::TextEditor &editor) override;

  void mouseWheelMove(const juce::MouseEvent &event,
                      const juce::MouseWheelDetails &wheel) override;
  /**
   * @brief Undocumented method.
   * @param event [in] Description for event.
   */
  void mouseUp(const juce::MouseEvent &event) override;

  /**
   * @brief Undocumented method.
   * @param editor [in] Description for editor.
   */
  void applyTimeEdit(juce::TextEditor &editor);
  void syncEditorToPosition(juce::TextEditor &editor, double positionSeconds,
                            bool isRemaining = false);

  ControlPanel &owner;
  juce::String totalTimeStaticStr;

  bool isEditingElapsed{false};
  bool isEditingRemaining{false};
  bool isEditingCutLength{false};

  /**
   * @brief Undocumented method.
   * @param event [in] Description for event.
   */
  void mouseDown(const juce::MouseEvent &event) override;
};

#endif // AUDIOFILER_PLAYBACKTEXTPRESENTER_H
