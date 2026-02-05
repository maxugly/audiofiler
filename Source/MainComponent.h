#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "ModernLookAndFeel.h"

class LoopButton : public juce::TextButton {
public:
  std::function<void()> onLeftClick;
  std::function<void()> onRightClick;
  LoopButton (const juce::String& name = {}) : juce::TextButton (name) {}

private:
  void mouseUp (const juce::MouseEvent& event) override {
    if (isEnabled()) {
      if (event.mods.isRightButtonDown()) {
        if (onRightClick) onRightClick();}
      else if (event.mods.isLeftButtonDown()) {
        if (onLeftClick) onLeftClick(); }}
    juce::TextButton::mouseUp(event); }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoopButton)};

class MainComponent  : public juce::AudioAppComponent,
                       public juce::ChangeListener,
                       public juce::Timer,
                       public juce::TextEditor::Listener { // Added TextEditor::Listener

public:
  enum class ViewMode { Classic, Overlay };
  enum class PlacementMode { None, LoopIn, LoopOut };
  enum class ChannelViewMode { Mono, Stereo };
  enum class ThumbnailQuality { Low, Medium, High };

  MainComponent();

  ~MainComponent() override;
  void mouseDown (const juce::MouseEvent& e) override;
  void mouseDrag (const juce::MouseEvent& e) override;
  void mouseMove (const juce::MouseEvent& e) override;
  void mouseUp (const juce::MouseEvent& e) override;
  void openButtonClicked();

  void seekToPosition (int x);
  bool keyPressed (const juce::KeyPress& key) override;
  void playStopButtonClicked();
  void updateButtonText();
  void updateLoopLabels();
  void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;

  void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill);
  void releaseResources() override;
  void changeListenerCallback (juce::ChangeBroadcaster*) override;
  void timerCallback();
  void paint (juce::Graphics& g);
  void resized();
  void focusGained (juce::Component::FocusChangeType cause) override;

private:
  juce::AudioFormatManager formatManager;
  std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
  juce::AudioTransportSource transportSource;
  juce::AudioThumbnailCache thumbnailCache;
  juce::AudioThumbnail thumbnail;
  juce::FlexBox getBottomRowFlexBox();

  ModernLookAndFeel modernLF;

  juce::TextButton openButton, playStopButton, modeButton, exitButton, statsButton, loopButton, channelViewButton, qualityButton, detectInSilenceButton;
  juce::TextButton clearLoopInButton, clearLoopOutButton;
  juce::TextEditor statsDisplay, loopInEditor, loopOutEditor, inSilenceThresholdEditor;
  juce::Label inSilenceThresholdLabel;
  float currentInSilenceThreshold = Config::silenceThreshold;

  juce::TextButton detectOutSilenceButton;
  juce::Label outSilenceThresholdLabel;
  juce::TextEditor outSilenceThresholdEditor;
  float currentOutSilenceThreshold = Config::outSilenceThreshold;
  juce::Rectangle<int> waveformBounds, statsBounds, contentAreaBounds;
  juce::FlexBox getTopRowFlexBox();
  juce::FlexBox getLoopRowFlexBox();

  std::unique_ptr<juce::FileChooser> chooser;

  ViewMode currentMode = ViewMode::Classic;
  ChannelViewMode currentChannelViewMode = ChannelViewMode::Mono;
  ThumbnailQuality currentQuality = ThumbnailQuality::Low;
  PlacementMode currentPlacementMode = PlacementMode::None;
  bool showStats = false;
  bool shouldLoop = false;
  double loopInPosition = -1.0;
  double loopOutPosition = -1.0;
  int mouseCursorX = -1, mouseCursorY = -1;
  int bottomRowTopY = 0; // Initialize to 0
  int playbackLeftTextX = 0;
  int playbackRightTextX = 0;
  int playbackCenterTextX = 0;
  juce::String totalTimeStaticStr;

  juce::String loopInDisplayString;
  juce::String loopOutDisplayString;
  int loopInTextX = 0;
  int loopOutTextX = 0;
  int loopTextY = 0;

  LoopButton loopInButton, loopOutButton;
  juce::TextButton autoplayButton; // New autoplay button
  bool shouldAutoplay = false;      // Autoplay state
  juce::TextButton autoCutInButton;     // New auto cut in button
  juce::TextButton autoCutOutButton;    // New auto cut out button
  bool shouldAutoCutIn = false;         // Auto cut in state
  bool shouldAutoCutOut = false;        // Auto cut out state


  void updateQualityButtonText();
  void drawReducedQualityWaveform(juce::Graphics& g, int channel, int pixelsPerSample);
  void updateLoopButtonColors();
  void detectInSilence();
  void detectOutSilence();
  void ensureLoopOrder(); // New method declaration

  juce::String formatTime(double seconds);

  // juce::TextEditor::Listener callbacks
  void textEditorTextChanged (juce::TextEditor& editor) override;
  void textEditorReturnKeyPressed (juce::TextEditor& editor) override;
  void textEditorEscapeKeyPressed (juce::TextEditor& editor) override;
  void textEditorFocusLost (juce::TextEditor& editor) override;

  // Helper function
  double parseTime(const juce::String& timeString); // Declared as a member function

  bool isFileLoaded = false; // New member variable
  void updateComponentStates(); // New method declaration

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent) };
