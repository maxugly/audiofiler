

#ifndef AUDIOFILER_REPEATPRESENTER_H
#define AUDIOFILER_REPEATPRESENTER_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

class ControlPanel;

class SilenceDetector;

class RepeatPresenter : private juce::TextEditor::Listener,
                        public juce::MouseListener {
public:

  RepeatPresenter(ControlPanel &ownerPanel, SilenceDetector &detector,
                juce::TextEditor &cutIn, juce::TextEditor &cutOut);

  void initialiseEditors();

  ~RepeatPresenter() override;

  double getCutInPosition() const noexcept;

  double getCutOutPosition() const noexcept;

  void setCutInPosition(double positionSeconds);

  void setCutOutPosition(double positionSeconds);

  void ensureCutOrder();

  void updateCutLabels();

  void setCutStartFromSample(int sampleIndex);

  void setCutEndFromSample(int sampleIndex);

private:

  void textEditorTextChanged(juce::TextEditor &editor) override;

  void textEditorReturnKeyPressed(juce::TextEditor &editor) override;

  void textEditorEscapeKeyPressed(juce::TextEditor &editor) override;

  void textEditorFocusLost(juce::TextEditor &editor) override;

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

#endif 
