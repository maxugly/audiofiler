#ifndef AUDIOFILER_REPEATPRESENTER_H
#define AUDIOFILER_REPEATPRESENTER_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

class ControlPanel;
class SilenceDetector;

/**
 * @class RepeatPresenter
 * @brief Coordinates the repeat button and the session state repeat flag.
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
  void textEditorTextChanged(juce::TextEditor &editor) override;
  void textEditorReturnKeyPressed(juce::TextEditor &editor) override;
  void textEditorEscapeKeyPressed(juce::TextEditor &editor) override;
  void textEditorFocusLost(juce::TextEditor &editor) override;

  // juce::MouseListener overrides
  void mouseWheelMove(const juce::MouseEvent &event,
                      const juce::MouseWheelDetails &wheel) override;
  void mouseEnter(const juce::MouseEvent &event) override;
  void mouseExit(const juce::MouseEvent &event) override;
  void mouseUp(const juce::MouseEvent &event) override;

  double getAudioTotalLength() const;
  bool applyCutInFromEditor(double newPosition, juce::TextEditor &editor);
  bool applyCutOutFromEditor(double newPosition, juce::TextEditor &editor);
  void syncEditorToPosition(juce::TextEditor &editor, double positionSeconds);

  ControlPanel &owner;
  SilenceDetector &silenceDetector;
  juce::TextEditor &cutInEditor;
  juce::TextEditor &cutOutEditor;
  bool isEditingIn{false};
  bool isEditingOut{false};

  void mouseDown(const juce::MouseEvent &event) override;
};

#endif // AUDIOFILER_REPEATPRESENTER_H
