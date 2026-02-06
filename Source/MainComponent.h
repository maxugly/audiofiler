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

  juce::TextButton openButton, playStopButton, modeButton, exitButton, statsButton, loopButton, channelViewButton, qualityButton;
  juce::TextButton clearLoopInButton, clearLoopOutButton;
  juce::TextEditor statsDisplay, loopInEditor, loopOutEditor, inSilenceThresholdEditor;
  float currentInSilenceThreshold = Config::silenceThreshold;


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
  float glowAlpha = 0.0f;                 // Glow animation alpha
  juce::TextButton cutButton;             // New cut button
  bool isCutModeActive = false;           // Cut mode state


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
  void initialiseLoopEditors(); // New method declaration
  void initialiseClearButtons(); // New method declaration
  void initialiseButtons();    // New method declaration
  void finaliseSetup();        // New method declaration
  void initialiseAudioFormatsAndThumbnail(); // New method declaration
  void initialiseLookAndFeel();  // New method declaration
  void initialiseLoopButtons();  // New method declaration

  // New private methods for individual button initialisation
  void initialiseOpenButton();
  void initialisePlayStopButton();
  void initialiseModeButton();
  void initialiseChannelViewButton();
  void initialiseQualityButton();
  void initialiseExitButton();
  void initialiseStatsButton();
  void initialiseLoopButton();
  void initialiseAutoplayButton();
  void initialiseAutoCutInButton();
  void initialiseAutoCutOutButton();
  void initialiseCutButton();
  void layoutTopRowButtons(juce::Rectangle<int>& bounds, int rowHeight); // New method declaration
  void layoutLoopAndCutControls(juce::Rectangle<int>& bounds, int rowHeight); // New method declaration
  /**
   * @brief Lays out the bottom row buttons and configures the playback text displays.
   *
   * This function positions the quality, channel view, stats, and mode buttons
   * in the bottom row of the component. It also calculates the positions for
   * the playback time text displays (current, total, remaining).
   *
   * @param bounds The current bounds of the component, which will be modified
   *               as sections are laid out.
   * @param rowHeight The calculated height for each row of buttons.
   */
  void layoutBottomRowAndTextDisplay(juce::Rectangle<int>& bounds, int rowHeight); // New method declaration
  /**
   * @brief Lays out the waveform area and conditionally displays the stats.
   *
   * This function determines the bounds for the audio waveform display based on the
   * current view mode (Classic or Overlay) and sets the visibility and position
   * of the stats display based on the `showStats` flag.
   *
   * @param bounds The current bounds of the component, which will be used to
   *               determine the waveform area.
   */
  void layoutWaveformAndStats(juce::Rectangle<int>& bounds); // New method declaration
  /**
   * @brief Updates the enabled and visible states of general UI buttons.
   *
   * This function sets the enabled and visible states for buttons that are
   * always active or depend only on whether a file is currently loaded,
   * regardless of the cut mode.
   *
   * @param enabled A boolean indicating whether a file is loaded (true) or not (false).
   */
  void updateGeneralButtonStates(bool enabled); // New method declaration
  /**
   * @brief Updates the enabled and visible states of controls related to "Cut" mode.
   *
   * This function manages the interactive states and visibility of loop buttons,
   * loop editors, clear loop buttons, silence threshold editors, and auto-cut buttons,
   * all based on whether the "Cut" mode is active and if an audio file is loaded.
   *
   * @param isCutModeActive A boolean indicating whether the "Cut" mode is currently active.
   * @param enabled A boolean indicating whether a file is loaded (true) or not (false).
   * @param shouldAutoCutIn A boolean indicating if auto-cut-in is active.
   * @param shouldAutoCutOut A boolean indicating if auto-cut-out is active.
   */
  void updateCutModeControlStates(bool isCutModeActive, bool enabled, bool shouldAutoCutIn, bool shouldAutoCutOut); // New method declaration
  /**
   * @brief Handles global keybinds for application-wide actions.
   *
   * This function processes key presses for actions that should be available
   * regardless of the current application state, such as quitting or opening a file.
   *
   * @param key The `juce::KeyPress` object representing the key that was pressed.
   * @return `true` if the key press was handled, `false` otherwise.
   */
  bool handleGlobalKeybinds(const juce::KeyPress& key); // New method declaration
  bool handlePlaybackKeybinds(const juce::KeyPress& key);
  bool handleUIToggleKeybinds(const juce::KeyPress& key);
  bool handleLoopKeybinds(const juce::KeyPress& key);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent) };
