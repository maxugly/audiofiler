/**
 * @file RepeatPresenter.h
 * @brief Defines the RepeatPresenter class.
 * @ingroup Presenters
 */

#ifndef AUDIOFILER_REPEATPRESENTER_H
#define AUDIOFILER_REPEATPRESENTER_H

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
 * @class RepeatPresenter
 * @brief Coordinates the repeat button and the session state repeat flag.
 * @see RepeatButton
 *        Also handles the cut boundary text editors.
 */
class RepeatPresenter : private juce::TextEditor::Listener,
                        public juce::MouseListener {
public:
  /**
   * @brief Constructs the presenter.
   */
  RepeatPresenter(ControlPanel &ownerPanel, SilenceDetector &detector,
                juce::TextEditor &cutIn, juce::TextEditor &cutOut);

  /**
   * @brief Configures the cut editors.
   */
  void initialiseEditors();

  /**
   * @brief Destructor.
   */
  ~RepeatPresenter() override;

  /** @brief Retrieves the cut-in position in seconds. */
  double getCutInPosition() const noexcept;

  /** @brief Retrieves the cut-out position in seconds. */
  double getCutOutPosition() const noexcept;

  /** @brief Sets the cut-in position. */
  void setCutInPosition(double positionSeconds);

  /** @brief Sets the cut-out position. */
  void setCutOutPosition(double positionSeconds);

  /** @brief Swaps cut-in/out values if they are inverted. */
  void ensureCutOrder();

  /** @brief Refreshes the editor text. */
  void updateCutLabels();

  /** @brief Sets the cut-in from a sample index. */
  void setCutStartFromSample(int sampleIndex);

  /** @brief Sets the cut-out from a sample index. */
  void setCutEndFromSample(int sampleIndex);

private:
  // juce::TextEditor::Listener overrides
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

  // juce::MouseListener overrides
  void mouseWheelMove(const juce::MouseEvent &event,
                      const juce::MouseWheelDetails &wheel) override;
  /**
   * @brief Undocumented method.
   * @param event [in] Description for event.
   */
  void mouseEnter(const juce::MouseEvent &event) override;
  /**
   * @brief Undocumented method.
   * @param event [in] Description for event.
   */
  void mouseExit(const juce::MouseEvent &event) override;
  /**
   * @brief Undocumented method.
   * @param event [in] Description for event.
   */
  void mouseUp(const juce::MouseEvent &event) override;

  /**
   * @brief Gets the AudioTotalLength.
   * @return double
   */
  double getAudioTotalLength() const;
  /**
   * @brief Undocumented method.
   * @param newPosition [in] Description for newPosition.
   * @param editor [in] Description for editor.
   * @return bool
   */
  bool applyCutInFromEditor(double newPosition, juce::TextEditor &editor);
  /**
   * @brief Undocumented method.
   * @param newPosition [in] Description for newPosition.
   * @param editor [in] Description for editor.
   * @return bool
   */
  bool applyCutOutFromEditor(double newPosition, juce::TextEditor &editor);
  /**
   * @brief Undocumented method.
   * @param editor [in] Description for editor.
   * @param positionSeconds [in] Description for positionSeconds.
   */
  void syncEditorToPosition(juce::TextEditor &editor, double positionSeconds);

  ControlPanel &owner;
  SilenceDetector &silenceDetector;
  juce::TextEditor &cutInEditor;
  juce::TextEditor &cutOutEditor;
  bool isEditingIn{false};
  bool isEditingOut{false};

  /**
   * @brief Undocumented method.
   * @param event [in] Description for event.
   */
  void mouseDown(const juce::MouseEvent &event) override;
};

#endif // AUDIOFILER_REPEATPRESENTER_H
