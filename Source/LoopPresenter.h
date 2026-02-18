#ifndef AUDIOFILER_LOOPPRESENTER_H
#define AUDIOFILER_LOOPPRESENTER_H

#include <JuceHeader.h>

class ControlPanel;
class SilenceDetector;

/**
 * @class LoopPresenter
 * @brief Owns the loop-in/out positions and keeps the loop editors in sync with
 * the audio state.
 *
 * This helper centralises the parsing, validation, and formatting logic for the
 * loop controls. It listens to the loop TextEditors, updates ControlPanel's
 * buttons as needed, and ensures the stored positions always reflect valid
 * ranges relative to the loaded audio file.
 */
class LoopPresenter : private juce::TextEditor::Listener,
                      public juce::MouseListener {
public:
  /**
   * @brief Constructs the presenter and attaches listeners to the loop editors.
   * @param ownerPanel Reference to the owning ControlPanel for callbacks
   * (repaint, colours, audio).
   * @param detector Reference to the SilenceDetector so manual edits can
   * disable auto-cut modes.
   * @param loopIn Reference to the loop-in TextEditor UI.
   * @param loopOut Reference to the loop-out TextEditor UI.
   */
  LoopPresenter(ControlPanel &ownerPanel, SilenceDetector &detector,
                juce::TextEditor &loopIn, juce::TextEditor &loopOut);

  /**
   * @brief Configures the loop editors (fonts, colors, etc.) and makes them
   * visible.
   */
  void initialiseEditors();

  /**
   * @brief Destructor removes the presenter as a TextEditor listener.
   */
  ~LoopPresenter() override;

  /**
   * @brief Retrieves the loop-in position in seconds.
   * @return Current loop-in value.
   */
  double getCutInPosition() const noexcept;

  /**
   * @brief Retrieves the loop-out position in seconds.
   * @return Current loop-out value.
   */
  double getCutOutPosition() const noexcept;

  /**
   * @brief Directly sets the loop-in position without additional validation.
   * @param positionSeconds The desired loop-in position in seconds.
   */
  void setCutInPosition(double positionSeconds);

  /**
   * @brief Directly sets the loop-out position without additional validation.
   * @param positionSeconds The desired loop-out position in seconds.
   */
  void setCutOutPosition(double positionSeconds);

  /**
   * @brief Swaps loop-in/out values if they are inverted.
   */
  void ensureLoopOrder();

  /**
   * @brief Refreshes the editor text to match the cached positions.
   */
  void updateCutLabels();

  /**
   * @brief Converts a sample index to seconds and stores it as the cut-in
   * position.
   * @param sampleIndex Sample index relative to the loaded file.
   */
  void setCutStartFromSample(int sampleIndex);

  /**
   * @brief Converts a sample index to seconds and stores it as the cut-out
   * position.
   * @param sampleIndex Sample index relative to the loaded file.
   */
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
  bool applyLoopInFromEditor(double newPosition, juce::TextEditor &editor);
  bool applyLoopOutFromEditor(double newPosition, juce::TextEditor &editor);
  void syncEditorToPosition(juce::TextEditor &editor, double positionSeconds);

  ControlPanel &owner;
  SilenceDetector &silenceDetector;
  juce::TextEditor &cutInEditor;
  juce::TextEditor &cutOutEditor;
  bool isEditingIn{false};
  bool isEditingOut{false};

  void mouseDown(const juce::MouseEvent &event) override;
};

#endif // AUDIOFILER_LOOPPRESENTER_H
