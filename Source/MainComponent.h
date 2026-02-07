#pragma once

#include <JuceHeader.h>
#include "ModernLookAndFeel.h"
#include "AudioPlayer.h"

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
  void initialiseButtons();    // New method declaration

  void timerCallback() override;
  void paint(juce::Graphics& g) override;
  void resized() override;
  void focusGained(juce::Component::FocusChangeType cause) override;
  void updateLoopLabels(); // Regular member function

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    //==============================================================================
    // Member Variables
    //==============================================================================
    std::unique_ptr<AudioPlayer> audioPlayer;
    std::unique_ptr<juce::FileChooser> chooser;
    ModernLookAndFeel modernLF;

    juce::TextButton openButton, playStopButton, modeButton, exitButton, statsButton, loopButton, channelViewButton, qualityButton;
    juce::TextButton clearLoopInButton, clearLoopOutButton;
    juce::TextEditor statsDisplay, loopInEditor, loopOutEditor, inSilenceThresholdEditor;
    float currentInSilenceThreshold = Config::silenceThreshold;
    juce::TextEditor outSilenceThresholdEditor;
    float currentOutSilenceThreshold = Config::outSilenceThreshold;
    juce::Rectangle<int> waveformBounds, statsBounds, contentAreaBounds;
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

    //==============================================================================
    // Private Methods
    //==============================================================================
    void updateQualityButtonText();
    void drawReducedQualityWaveform(juce::Graphics& g, int channel, int pixelsPerSample);
    void updateLoopButtonColors();
    void detectInSilence();
    void detectOutSilence();
    void ensureLoopOrder();
    juce::String formatTime(double seconds);
    void textEditorTextChanged (juce::TextEditor& editor) override;
    void textEditorReturnKeyPressed (juce::TextEditor& editor) override;
    void textEditorEscapeKeyPressed (juce::TextEditor& editor) override;
    void textEditorFocusLost (juce::TextEditor& editor) override;
    double parseTime(const juce::String& timeString);
    void updateComponentStates();
    void initialiseLoopEditors();
    void initialiseClearButtons();
    void finaliseSetup();
    void initialiseLookAndFeel();
    void initialiseLoopButtons();
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
    void layoutTopRowButtons(juce::Rectangle<int>& bounds, int rowHeight);
    void layoutLoopAndCutControls(juce::Rectangle<int>& bounds, int rowHeight);
    void layoutBottomRowAndTextDisplay(juce::Rectangle<int>& bounds, int rowHeight);
    void layoutWaveformAndStats(juce::Rectangle<int>& bounds);
    void updateGeneralButtonStates(bool enabled);
    void updateCutModeControlStates(bool isCutModeActive, bool enabled, bool shouldAutoCutIn, bool shouldAutoCutOut);
    bool handleGlobalKeybinds(const juce::KeyPress& key);
    bool handlePlaybackKeybinds(const juce::KeyPress& key);
    bool handleUIToggleKeybinds(const juce::KeyPress& key);
    bool handleLoopKeybinds(const juce::KeyPress& key);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent) };
