#include "MainComponent.h"
#include "Config.h"
#include "AudioPlayer.h"

MainComponent::MainComponent()
{
    audioPlayer = std::make_unique<AudioPlayer>();
    audioPlayer->addChangeListener(this);
  initialiseLookAndFeel();
  initialiseLoopButtons(); // Call the new function
  initialiseButtons();


  initialiseClearButtons();

  initialiseLoopEditors();

  finaliseSetup();

#if TESTING_MODE
  juce::File audioFile(TEST_FILE_PATH);
  if (audioFile.existsAsFile())
  {
      auto result = audioPlayer->loadFile(audioFile);
      if (result.wasOk())
      {
          statsDisplay.setText("File loaded: " + audioFile.getFileName(), juce::dontSendNotification);
          playStopButton.setEnabled(true);
          totalTimeStaticStr = formatTime(audioPlayer->getThumbnail().getTotalLength());
          loopInPosition  = 0.0;
          loopOutPosition = audioPlayer->getThumbnail().getTotalLength();
          // Set flags as requested
          shouldAutoplay = true;
          shouldLoop = true; // Use audioPlayer->setLooping(true) instead of this. This will be handled by the updateComponentStates call implicitly.
          isCutModeActive = true;
          shouldAutoCutIn = true;
          shouldAutoCutOut = true;

          // Update button states
          autoplayButton.setToggleState (shouldAutoplay, juce::dontSendNotification);
          loopButton.setToggleState (shouldLoop, juce::dontSendNotification);
          cutButton.setToggleState (isCutModeActive, juce::dontSendNotification);
          autoCutInButton.setToggleState (shouldAutoCutIn, juce::dontSendNotification);
          autoCutOutButton.setToggleState (shouldAutoCutOut, juce::dontSendNotification);

          // Trigger auto-cut and autoplay
          detectInSilence();
          detectOutSilence();
          updateComponentStates(); // Ensure UI reflects the new state

          DBG("TESTING_MODE: Autoplay, Loop, Cut, AutoCutIn/Out enabled.");
      }
      else
      {
          statsDisplay.setText(result.getErrorMessage(), juce::dontSendNotification);
          statsDisplay.setColour(juce::Label::textColourId, juce::Colours::red);
          DBG("TESTING_MODE: Failed to load audio file: " << audioFile.getFileName() << " - " << result.getErrorMessage());
      }
  }
  else
  {
      DBG("TESTING_MODE: " << audioFile.getFileName() << " not found at " << audioFile.getFullPathName());
  }
#endif
}



void MainComponent::initialiseLoopButtons()
{
  addAndMakeVisible (loopInButton);
  loopInButton.setButtonText (Config::loopInButtonText);
  loopInButton.onLeftClick = [this] {
    loopInPosition = audioPlayer->getTransportSource().getCurrentPosition();
    DBG("Loop In Button Left Clicked. Position: " << loopInPosition);
    ensureLoopOrder(); // Call helper
    updateLoopButtonColors();
    repaint();
  };
  loopInButton.onRightClick = [this] {
    DBG("Loop In Button Right Clicked. Setting placement mode to LoopIn.");
    currentPlacementMode = PlacementMode::LoopIn;
    updateLoopButtonColors();
    repaint();
  };

  addAndMakeVisible (loopOutButton);
  loopOutButton.setButtonText (Config::loopOutButtonText);
  loopOutButton.onLeftClick = [this] {
    loopOutPosition = audioPlayer->getTransportSource().getCurrentPosition();
    DBG("Loop Out Button Left Clicked. Position: " << loopOutPosition);
    ensureLoopOrder(); // Call helper
    updateLoopButtonColors();
    repaint();
  };
  loopOutButton.onRightClick = [this] {
    DBG("Loop Out Button Right Clicked. Setting placement mode to LoopOut.");
    currentPlacementMode = PlacementMode::LoopOut;
    updateLoopButtonColors();
    repaint();
  };
}

void MainComponent::initialiseLookAndFeel()
{
  setLookAndFeel (&modernLF);
  modernLF.setBaseOffColor(Config::buttonBaseColour);
  modernLF.setBaseOnColor(Config::buttonOnColour);
  modernLF.setTextColor(Config::buttonTextColour);
}



void MainComponent::initialiseButtons()
{
  initialiseOpenButton();
  initialisePlayStopButton();
  initialiseModeButton();
  initialiseChannelViewButton();
  initialiseQualityButton();
  initialiseExitButton();
  initialiseStatsButton();
  initialiseLoopButton();
  initialiseAutoplayButton();
  initialiseAutoCutInButton();
  initialiseAutoCutOutButton();
  initialiseCutButton();

  addAndMakeVisible (statsDisplay);
  statsDisplay.setReadOnly (true);
  statsDisplay.setMultiLine (true);
  statsDisplay.setWantsKeyboardFocus (false);
  statsDisplay.setColour (juce::TextEditor::backgroundColourId, Config::statsDisplayBackgroundColour);
  statsDisplay.setColour (juce::TextEditor::textColourId, Config::statsDisplayTextColour);
  statsDisplay.setVisible (false);
}

/**
 * @brief Initializes the open file button.
 *
 * Sets up the visual properties and attaches the click listener for the
 * button that triggers the file chooser dialog.
 */
void MainComponent::initialiseOpenButton()
{
  addAndMakeVisible (openButton);
  openButton.setButtonText (Config::openButtonText);
  openButton.onClick = [this] { openButtonClicked(); };
}

/**
 * @brief Initializes the play/stop button.
 *
 * Sets up the visual properties and attaches the click listener for the
 * button that controls audio playback (play/pause).
 */
void MainComponent::initialisePlayStopButton()
{
  addAndMakeVisible (playStopButton);
  playStopButton.onClick = [this] { audioPlayer->togglePlayStop(); };
  playStopButton.setEnabled (false);
}

/**
 * @brief Initializes the view mode button.
 *
 * Configures the button that toggles between different waveform view modes
 * (e.g., Classic, Overlay) and updates the UI accordingly.
 */
void MainComponent::initialiseModeButton()
{
  addAndMakeVisible (modeButton);
  modeButton.setButtonText (Config::viewModeClassicText); // Set initial text from config
  modeButton.setClickingTogglesState (true);
  modeButton.onClick = [this] {
    DBG("Button Clicked: Mode, new state: " << (modeButton.getToggleState() ? "Overlay" : "Classic"));
    currentMode = modeButton.getToggleState() ? ViewMode::Overlay : ViewMode::Classic;
    modeButton.setButtonText (currentMode == ViewMode::Classic ? Config::viewModeClassicText : Config::viewModeOverlayText);
    resized();
    repaint(); };
}

/**
 * @brief Initializes the channel view button.
 *
 * Configures the button that toggles between mono and stereo channel views
 * for the waveform display.
 */
void MainComponent::initialiseChannelViewButton()
{
  addAndMakeVisible (channelViewButton);
  channelViewButton.setButtonText (Config::channelViewMonoText); // Set initial text from config
  channelViewButton.setClickingTogglesState (true);
  channelViewButton.onClick = [this] {
    DBG("Button Clicked: Channel View, new state: " << (channelViewButton.getToggleState() ? "Stereo" : "Mono"));
    currentChannelViewMode = channelViewButton.getToggleState() ? ChannelViewMode::Stereo : ChannelViewMode::Mono;
    channelViewButton.setButtonText (currentChannelViewMode == ChannelViewMode::Mono ? Config::channelViewMonoText : Config::channelViewStereoText);
    repaint(); };
}

/**
 * @brief Initializes the quality button.
 *
 * Sets up the button that cycles through different waveform rendering quality
 * settings (High, Medium, Low).
 */
void MainComponent::initialiseQualityButton()
{
  addAndMakeVisible (qualityButton);
  qualityButton.setButtonText (Config::qualityButtonText);
  qualityButton.onClick = [this] {
    DBG("Button Clicked: Quality");
    if (currentQuality == ThumbnailQuality::High)
      currentQuality = ThumbnailQuality::Medium;
    else if (currentQuality == ThumbnailQuality::Medium)
      currentQuality = ThumbnailQuality::Low;
    else
      currentQuality = ThumbnailQuality::High;
   updateQualityButtonText();
   repaint(); };
  updateQualityButtonText();
}

/**
 * @brief Initializes the exit button.
 *
 * Configures the button that allows the user to quit the application.
 */
void MainComponent::initialiseExitButton()
{
  addAndMakeVisible (exitButton);
  exitButton.setButtonText (Config::exitButtonText);
  exitButton.setColour (juce::TextButton::buttonColourId, Config::exitButtonColor);
  exitButton.onClick = [] {
    DBG("Button Clicked: Exit - System Quit Requested");
    juce::JUCEApplication::getInstance()->systemRequestedQuit(); };
}

/**
 * @brief Initializes the stats button.
 *
 * Sets up the button that toggles the visibility of the statistics display.
 */
void MainComponent::initialiseStatsButton()
{
  addAndMakeVisible (statsButton);
  statsButton.setButtonText (Config::statsButtonText);
  statsButton.setClickingTogglesState (true);
  statsButton.onClick = [this] {
    DBG("Button Clicked: Stats, new state: " << (statsButton.getToggleState() ? "Visible" : "Hidden"));
    showStats = statsButton.getToggleState();
    resized();
    updateComponentStates(); };
}

/**
 * @brief Initializes the loop button.
 *
 * Configures the button that toggles whether audio playback should loop.
 */
void MainComponent::initialiseLoopButton()
{
  addAndMakeVisible (loopButton);
  loopButton.setButtonText (Config::loopButtonText);
  loopButton.setClickingTogglesState (true);
  loopButton.onClick = [this] {
    DBG("Button Clicked: Loop, new state: " << (loopButton.getToggleState() ? "On" : "Off"));
    shouldLoop = loopButton.getToggleState(); };
}

/**
 * @brief Initializes the autoplay button.
 *
 * Sets up the button that toggles the autoplay feature, which starts
 * playback automatically after a file is loaded.
 */
void MainComponent::initialiseAutoplayButton()
{
  addAndMakeVisible (autoplayButton);
  autoplayButton.setButtonText (Config::autoplayButtonText);
  autoplayButton.setClickingTogglesState (true);
  autoplayButton.setToggleState (shouldAutoplay, juce::dontSendNotification);
  autoplayButton.onClick = [this] {
    DBG("Button Clicked: Autoplay, new state: " << (autoplayButton.getToggleState() ? "On" : "Off"));
    shouldAutoplay = autoplayButton.getToggleState();
  };
}

/**
 * @brief Initializes the auto-cut in button.
 *
 * Configures the button that toggles the automatic detection of the
 * "in" loop point based on silence.
 */
void MainComponent::initialiseAutoCutInButton()
{
  addAndMakeVisible (autoCutInButton);
  autoCutInButton.setButtonText (Config::autoCutInButtonText);
  autoCutInButton.setClickingTogglesState (true);
  autoCutInButton.setToggleState (shouldAutoCutIn, juce::dontSendNotification);
  autoCutInButton.onClick = [this] {
    DBG("Button Clicked: Auto Cut In, new state: " << (autoCutInButton.getToggleState() ? "On" : "Off"));
    shouldAutoCutIn = autoCutInButton.getToggleState();
    if (shouldAutoCutIn && audioPlayer->getThumbnail().getTotalLength() > 0.0) { // Only detect if a file is loaded and auto-cut is turned ON
        detectInSilence();
    }
    updateComponentStates(); // Update component states to reflect changes in shouldAutoCutIn/Out
  };
}

/**
 * @brief Initializes the auto-cut out button.
 *
 * Configures the button that toggles the automatic detection of the
 * "out" loop point based on silence.
 */
void MainComponent::initialiseAutoCutOutButton()
{
  addAndMakeVisible (autoCutOutButton);
  autoCutOutButton.setButtonText (Config::autoCutOutButtonText);
  autoCutOutButton.setClickingTogglesState (true);
  autoCutOutButton.setToggleState (shouldAutoCutOut, juce::dontSendNotification);
  autoCutOutButton.onClick = [this] {
    DBG("Button Clicked: Auto Cut Out, new state: " << (autoCutOutButton.getToggleState() ? "On" : "Off"));
    shouldAutoCutOut = autoCutOutButton.getToggleState();
    if (shouldAutoCutOut && audioPlayer->getThumbnail().getTotalLength() > 0.0) { // Only detect if a file is loaded and auto-cut is turned ON
        detectOutSilence();
    }
    updateComponentStates(); // Update component states to reflect changes in shouldAutoCutIn/Out
  };
}

/**
 * @brief Initializes the cut mode button.
 *
 * Sets up the button that toggles the overall "Cut" mode, enabling or
 * disabling related UI controls and visualizations.
 */
void MainComponent::initialiseCutButton()
{
  addAndMakeVisible (cutButton);
  cutButton.setButtonText (Config::cutButtonText);
  cutButton.setClickingTogglesState (true);
  cutButton.setToggleState (isCutModeActive, juce::dontSendNotification);
  cutButton.onClick = [this] {
    DBG("Button Clicked: Cut Mode, new state: " << (cutButton.getToggleState() ? "Active" : "Inactive"));
    isCutModeActive = cutButton.getToggleState();
    updateComponentStates(); // Update visibility/enabled state of cut-related controls
  };
}

void MainComponent::initialiseClearButtons()
{
  addAndMakeVisible (clearLoopInButton);
  clearLoopInButton.setButtonText (Config::clearButtonText);
  clearLoopInButton.setColour(juce::TextButton::buttonColourId, Config::clearButtonColor);
  clearLoopInButton.onClick = [this]
  {
    DBG("Button Clicked: Clear Loop In (reset to start)");
    loopInPosition = 0.0;
    ensureLoopOrder();
    updateLoopButtonColors();
    updateLoopLabels();
    repaint();
  };

  addAndMakeVisible (clearLoopOutButton);
  clearLoopOutButton.setButtonText (Config::clearButtonText);
  clearLoopOutButton.setColour(juce::TextButton::buttonColourId, Config::clearButtonColor);
  clearLoopOutButton.onClick = [this]
  {
    DBG("Button Clicked: Clear Loop Out (reset to end)");
    loopOutPosition = audioPlayer->getThumbnail().getTotalLength();
    ensureLoopOrder();
    updateLoopButtonColors();
    updateLoopLabels();
    repaint();
  };
}

void MainComponent::initialiseLoopEditors()
{
  addAndMakeVisible (loopInEditor);
  loopInEditor.setReadOnly (false);
  loopInEditor.setJustification(juce::Justification::centred);
  loopInEditor.setColour (juce::TextEditor::backgroundColourId, Config::textEditorBackgroundColour);
  loopInEditor.setColour (juce::TextEditor::textColourId, Config::playbackTextColor);
  loopInEditor.setFont (juce::Font (juce::FontOptions (Config::playbackTextSize)));
  loopInEditor.setMultiLine (false);
  loopInEditor.setReturnKeyStartsNewLine (false);
  loopInEditor.addListener (this);
  loopInEditor.setWantsKeyboardFocus (true); // Allow to take focus for editing

  addAndMakeVisible (loopOutEditor);
  loopOutEditor.setReadOnly (false);
  loopOutEditor.setJustification(juce::Justification::centred);
  loopOutEditor.setColour (juce::TextEditor::backgroundColourId, Config::textEditorBackgroundColour);
  loopOutEditor.setColour (juce::TextEditor::textColourId, Config::playbackTextColor);
  loopOutEditor.setFont (juce::Font (juce::FontOptions (Config::playbackTextSize)));
  loopOutEditor.setMultiLine (false);
  loopOutEditor.setReturnKeyStartsNewLine (false);
  loopOutEditor.addListener (this);
  loopOutEditor.setWantsKeyboardFocus (true); // Allow to take focus for editing

  addAndMakeVisible (inSilenceThresholdEditor);
  inSilenceThresholdEditor.setText (juce::String (static_cast<int>(Config::silenceThreshold * 100.0f)));
  inSilenceThresholdEditor.setInputRestrictions (0, "0123456789");
  inSilenceThresholdEditor.setJustification(juce::Justification::centred);
  inSilenceThresholdEditor.setColour (juce::TextEditor::backgroundColourId, Config::textEditorBackgroundColour);
  inSilenceThresholdEditor.setColour (juce::TextEditor::textColourId, Config::playbackTextColor);
  inSilenceThresholdEditor.setFont (juce::Font (juce::FontOptions (Config::playbackTextSize)));
  inSilenceThresholdEditor.applyFontToAllText(inSilenceThresholdEditor.getFont());
  inSilenceThresholdEditor.setMultiLine (false);
  inSilenceThresholdEditor.setReturnKeyStartsNewLine (false);
  inSilenceThresholdEditor.addListener (this);
  inSilenceThresholdEditor.setWantsKeyboardFocus (true);

  addAndMakeVisible (outSilenceThresholdEditor);
  outSilenceThresholdEditor.setText (juce::String (static_cast<int>(Config::outSilenceThreshold * 100.0f)));
  outSilenceThresholdEditor.setInputRestrictions (0, "0123456789");
  outSilenceThresholdEditor.setJustification(juce::Justification::centred);
  outSilenceThresholdEditor.setColour (juce::TextEditor::backgroundColourId, Config::textEditorBackgroundColour);
  outSilenceThresholdEditor.setColour (juce::TextEditor::textColourId, Config::playbackTextColor);
  outSilenceThresholdEditor.setFont (juce::Font (juce::FontOptions (Config::playbackTextSize)));
  outSilenceThresholdEditor.applyFontToAllText(outSilenceThresholdEditor.getFont());
  outSilenceThresholdEditor.setMultiLine (false);
  outSilenceThresholdEditor.setReturnKeyStartsNewLine (false);
  outSilenceThresholdEditor.addListener (this);
  outSilenceThresholdEditor.setWantsKeyboardFocus (true);
}

void MainComponent::finaliseSetup()
{
  updateLoopLabels();
  setSize(Config::initialWindowWidth, Config::initialWindowHeight);

  setAudioChannels (0, 2);
  startTimerHz (60);
  setWantsKeyboardFocus (true);
  grabKeyboardFocus(); // Request keyboard focus for MainComponent

  updateComponentStates(); // Call to set initial button states
}

void MainComponent::layoutLoopAndCutControls(juce::Rectangle<int>& bounds, int rowHeight)
{
  auto loopRow = bounds.removeFromTop(rowHeight).reduced(Config::windowBorderMargins);
  loopInButton.setBounds(loopRow.removeFromLeft(Config::buttonWidth)); loopRow.removeFromLeft(Config::windowBorderMargins);

  // Position loopInEditor
  loopInTextX = loopRow.getX(); // Left edge of the space for loopIn
  loopTextY = loopRow.getY() + (loopRow.getHeight() / 2) - Config::loopTextOffsetY; // Vertically center 20px high text
  loopInEditor.setBounds(loopInTextX, loopTextY, Config::loopTextWidth, Config::playbackTextHeight); // Set bounds for loopInEditor
  loopRow.removeFromLeft(Config::loopTextWidth); // Space for loopInEditor
  loopRow.removeFromLeft(Config::windowBorderMargins / 2);

  clearLoopInButton.setBounds(loopRow.getX(), loopTextY, Config::clearButtonWidth, Config::playbackTextHeight);
  loopRow.removeFromLeft(Config::clearButtonMargin);

  loopRow.removeFromLeft(Config::windowBorderMargins * 2); // Doubled distance

  loopOutButton.setBounds(loopRow.removeFromLeft(Config::buttonWidth)); loopRow.removeFromLeft(Config::windowBorderMargins);

  // Position loopOutEditor
  loopOutTextX = loopRow.getX(); // Left edge of the space for loopOut
  // loopTextY is already set once, assuming numbers are on the same line
  loopOutEditor.setBounds(loopOutTextX, loopTextY, Config::loopTextWidth, Config::playbackTextHeight); // Set bounds for loopOutEditor
  loopRow.removeFromLeft(Config::loopTextWidth); // Space for loopOutEditor
  loopRow.removeFromLeft(Config::windowBorderMargins / 2);

  clearLoopOutButton.setBounds(loopRow.getX(), loopTextY, Config::clearButtonWidth, Config::playbackTextHeight);
  loopRow.removeFromLeft(Config::clearButtonMargin);

  loopRow.removeFromLeft(Config::windowBorderMargins * 2);

  inSilenceThresholdEditor.setBounds(loopRow.getX(), loopTextY, Config::thresholdEditorWidth, Config::playbackTextHeight);
  loopRow.removeFromLeft(Config::thresholdEditorWidth);
  autoCutInButton.setBounds(loopRow.removeFromLeft(Config::buttonWidth));
  loopRow.removeFromLeft(Config::windowBorderMargins * 2);

  outSilenceThresholdEditor.setBounds(loopRow.getX(), loopTextY, Config::thresholdEditorWidth, Config::playbackTextHeight);
  loopRow.removeFromLeft(Config::thresholdEditorWidth);
  autoCutOutButton.setBounds(loopRow.removeFromLeft(Config::buttonWidth));
  loopRow.removeFromLeft(Config::windowBorderMargins * 2);
}

void MainComponent::layoutTopRowButtons(juce::Rectangle<int>& bounds, int rowHeight)
{
  auto topRow = bounds.removeFromTop(rowHeight).reduced(Config::windowBorderMargins);
  openButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
  playStopButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
  autoplayButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
  loopButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
  cutButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
  modeButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
  statsButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
  autoCutInButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
  autoCutOutButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
  exitButton.setBounds(topRow.removeFromRight(Config::buttonWidth)); topRow.removeFromRight(Config::windowBorderMargins);
}

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
void MainComponent::layoutBottomRowAndTextDisplay(juce::Rectangle<int>& bounds, int rowHeight)
{
  auto bottomRow = bounds.removeFromBottom(rowHeight).reduced(Config::windowBorderMargins);
  bottomRowTopY = bottomRow.getY();
  contentAreaBounds = bounds.reduced(Config::windowBorderMargins); // Store the actual content area bounds here
  qualityButton.setBounds(bottomRow.removeFromRight(Config::buttonWidth)); bottomRow.removeFromRight(Config::windowBorderMargins);
  channelViewButton.setBounds(bottomRow.removeFromRight(Config::buttonWidth)); bottomRow.removeFromRight(Config::windowBorderMargins);
  statsButton.setBounds(bottomRow.removeFromRight(Config::buttonWidth)); bottomRow.removeFromRight(Config::windowBorderMargins);
  modeButton.setBounds(bottomRow.removeFromRight(Config::buttonWidth));

  playbackLeftTextX = getLocalBounds().getX() + Config::windowBorderMargins;
  playbackCenterTextX = (getLocalBounds().getWidth() / 2) - (Config::playbackTextWidth / 2); // Use Config::playbackTextWidth
  playbackRightTextX = getLocalBounds().getRight() - Config::windowBorderMargins - Config::playbackTextWidth; // Use Config::playbackTextWidth
}

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
void MainComponent::layoutWaveformAndStats(juce::Rectangle<int>& bounds)
{
  if (currentMode == ViewMode::Overlay) {waveformBounds = getLocalBounds(); }
  else {waveformBounds = bounds.reduced(Config::windowBorderMargins);} // Use the remaining bounds

  if (showStats) {
    statsBounds = contentAreaBounds.withHeight(100).reduced(10); // Use contentAreaBounds
    statsDisplay.setBounds(statsBounds);
    statsDisplay.setVisible(true);
    statsDisplay.toFront(true); }
  else {statsDisplay.setVisible(false); }
}

void MainComponent::updateQualityButtonText() {
  if (currentQuality == MainComponent::ThumbnailQuality::High) qualityButton.setButtonText(Config::qualityHighText);
  else if (currentQuality == MainComponent::ThumbnailQuality::Medium) qualityButton.setButtonText(Config::qualityMediumText);
  else qualityButton.setButtonText(Config::qualityLowText); }

void MainComponent::updateLoopButtonColors() {
  if (currentPlacementMode == PlacementMode::LoopIn) {
    loopInButton.setColour(juce::TextButton::buttonColourId, Config::loopButtonPlacementModeColor);
  } else {
    loopInButton.setColour(juce::TextButton::buttonColourId, Config::loopButtonActiveColor);
  }
  if (currentPlacementMode == PlacementMode::LoopOut) {
    loopOutButton.setColour(juce::TextButton::buttonColourId, Config::loopButtonPlacementModeColor);
  } else {
    loopOutButton.setColour(juce::TextButton::buttonColourId, Config::loopButtonActiveColor);
  }
  updateLoopLabels();
}

MainComponent::~MainComponent() {
  audioPlayer->removeChangeListener(this);
  shutdownAudio();
  stopTimer();
  setLookAndFeel(nullptr); }

juce::String MainComponent::formatTime(double seconds) {
  if (seconds < 0)
    seconds = 0;
  int hours = (int)(seconds / 3600.0);
  int minutes = ((int)(seconds / 60.0)) % 60;
  int secs = ((int)seconds) % 60;
  int milliseconds = (int)((seconds - (int)seconds) * 1000.0);
  return juce::String::formatted("%02d:%02d:%02d:%03d", hours, minutes, secs, milliseconds); }

double MainComponent::parseTime(const juce::String& timeString) {
    auto parts = juce::StringArray::fromTokens(timeString, ":", "");

    if (parts.size() != 4)
        return -1.0;

    int hours = parts[0].getIntValue();
    int minutes = parts[1].getIntValue();
    int seconds = parts[2].getIntValue();
    int milliseconds = parts[3].getIntValue();

    if (hours < 0 || minutes < 0 || minutes >= 60 || seconds < 0 || seconds >= 60 || milliseconds < 0 || milliseconds >= 1000)
        return -1.0;

    return (double)hours * 3600.0 + (double)minutes * 60.0 + (double)seconds + (double)milliseconds / 1000.0; }

void MainComponent::drawReducedQualityWaveform(juce::Graphics& g, int channel, int pixelsPerSample) {
  auto audioLength = audioPlayer->getThumbnail().getTotalLength();
  if (audioLength <= 0.0)
  return;
  auto width = waveformBounds.getWidth();
  auto height = waveformBounds.getHeight();
  auto centerY = waveformBounds.getCentreY();
  for (int x = 0; x < width; x += pixelsPerSample) {
    auto proportion = (double)x / (double)width;
    auto time = proportion * audioLength;
    float minVal, maxVal;
    audioPlayer->getThumbnail().getApproximateMinMax(time, time + (audioLength / width) * pixelsPerSample, channel, minVal, maxVal);
    auto topY = centerY - (maxVal * height * 0.5f);
    auto bottomY = centerY - (minVal * height * 0.5f);
    g.drawVerticalLine(waveformBounds.getX() + x, topY, bottomY); }}

void MainComponent::openButtonClicked() {
  DBG("Button Clicked: Open");
  updateComponentStates(); // Disable buttons until a file is successfully loaded

  chooser = std::make_unique<juce::FileChooser> ("Select Audio...",
    juce::File::getSpecialLocation (juce::File::userHomeDirectory), audioPlayer->getFormatManager().getWildcardForAllFormats());
  auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
  chooser->launchAsync (flags, [this] (const juce::FileChooser& fc) {
    auto file = fc.getResult();
    if (file.exists()) {
      auto result = audioPlayer->loadFile(file);
      if (result.wasOk())
      {
        statsDisplay.setText("File loaded: " + file.getFileName(), juce::dontSendNotification);
        playStopButton.setEnabled(true);
        totalTimeStaticStr = formatTime(audioPlayer->getThumbnail().getTotalLength()); // Set total time static string
        loopInPosition  = 0.0;
        loopOutPosition = audioPlayer->getThumbnail().getTotalLength();
        
        // --- NEW LOGIC FOR AUTO-CUT ON LOAD ---
        // If auto-cut-in is active, perform silence detection for the in point
        if (shouldAutoCutIn) {
            detectInSilence();
        }
        // If auto-cut-out is active, perform silence detection for the out point
        if (shouldAutoCutOut) {
            detectOutSilence();
        }
        // --- END NEW LOGIC ---

        updateComponentStates(); // Update component states after loading file and potentially auto-cutting
        grabKeyboardFocus(); // Re-grab focus after file chooser closes
        
        if (shouldAutoplay && audioPlayer->isPlaying() == false) { // Autoplay only if explicitly enabled
            audioPlayer->togglePlayStop(); // Simulate click to start playback
        }
      }
      else
      {
        statsDisplay.setText(result.getErrorMessage(), juce::dontSendNotification);
        statsDisplay.setColour(juce::Label::textColourId, juce::Colours::red);
      }
    }
  });}



bool MainComponent::keyPressed (const juce::KeyPress& key) {
    auto keyCode = key.getTextCharacter();
    DBG("Key Pressed: " << key.getTextDescription());

    if (handleGlobalKeybinds(key))
        return true;

    // All other actions require a file to be loaded
    if (audioPlayer->getThumbnail().getTotalLength() > 0.0) {
        if (handlePlaybackKeybinds(key))
            return true;
        if (handleUIToggleKeybinds(key))
            return true;
        if (handleLoopKeybinds(key))
            return true;
    }

    // If none of the above conditions were met, return false
    return false;
}

/**
 * @brief Handles key presses related to audio playback and seeking.
 *
 * This function processes key presses for actions like skipping forward/backward
 * and toggling play/stop, but only if an audio file is loaded.
 *
 * @param key The `juce::KeyPress` object representing the key that was pressed.
 * @return `true` if the key press was handled, `false` otherwise.
 */
bool MainComponent::handlePlaybackKeybinds(const juce::KeyPress& key)
{
    if (audioPlayer->getThumbnail().getTotalLength() > 0.0) {
        constexpr double skipAmountSeconds = Config::keyboardSkipAmountSeconds;
        if (key == juce::KeyPress::leftKey) {
            auto newPos = juce::jmax (0.0, audioPlayer->getTransportSource().getCurrentPosition() - skipAmountSeconds);
            audioPlayer->getTransportSource().setPosition (newPos);
            DBG("  Left arrow key pressed. Seeking to " << newPos);
            return true;
        }
        if (key == juce::KeyPress::rightKey) {
            auto newPos = juce::jmin (audioPlayer->getThumbnail().getTotalLength(), audioPlayer->getTransportSource().getCurrentPosition() + skipAmountSeconds);
            audioPlayer->getTransportSource().setPosition (newPos);
            DBG("  Right arrow key pressed. Seeking to " << newPos);
            return true;
        }
    }

    if (key == juce::KeyPress::spaceKey) {
        DBG("  Space key pressed. Toggling play/stop.");
        audioPlayer->togglePlayStop();
        return true;
    }
    return false;
}

/**
 * @brief Handles key presses related to toggling UI elements and views.
 *
 * This function processes key presses for actions like toggling stats display,
 * view modes, channel views, quality settings, and loop mode.
 *
 * @param key The `juce::KeyPress` object representing the key that was pressed.
 * @return `true` if the key press was handled, `false` otherwise.
 */
bool MainComponent::handleUIToggleKeybinds(const juce::KeyPress& key)
{
    auto keyCode = key.getTextCharacter();
    if (keyCode == 's' || keyCode == 'S') {
        DBG("  's' key pressed. Toggling stats.");
        statsButton.triggerClick();
        return true;
    }
    if (keyCode == 'v' || keyCode == 'V') {
        DBG("  'v' key pressed. Toggling view mode.");
        modeButton.triggerClick();
        return true;
    }
    if (keyCode == 'c' || keyCode == 'C') {
        DBG("  'c' key pressed. Toggling channel view mode.");
        channelViewButton.triggerClick();
        return true;
    }
    if (keyCode == 'q' || keyCode == 'Q') {
        DBG("  'q' key pressed. Toggling quality.");
        qualityButton.triggerClick();
        return true;
    }
    if (keyCode == 'l' || keyCode == 'L') {
        DBG("  'l' key pressed. Toggling loop.");
        loopButton.triggerClick();
        return true;
    }
    return false;
}

/**
 * @brief Handles key presses related to setting and clearing loop points.
 *
 * This function processes key presses for manually setting 'in' and 'out'
 * loop points, and for clearing them. It considers whether auto-cut modes
 * are active to prevent manual override.
 *
 * @param key The `juce::KeyPress` object representing the key that was pressed.
 * @return `true` if the key press was handled, `false` otherwise.
 */
bool MainComponent::handleLoopKeybinds(const juce::KeyPress& key)
{
    auto keyCode = key.getTextCharacter();
    if (keyCode == 'i' || keyCode == 'I') {
        if (shouldAutoCutIn) {
            DBG("  'i' key pressed ignored: Auto Cut In is active.");
            return true;
        }
        loopInPosition = audioPlayer->getTransportSource().getCurrentPosition();
        DBG("  'i' key pressed. Setting loop in position to " << loopInPosition);
        ensureLoopOrder(); // Call helper
        updateLoopButtonColors();
        updateLoopLabels();
        repaint();
        return true;
    }
    if (keyCode == 'o' || keyCode == 'O') {
        if (shouldAutoCutOut) {
            DBG("  'o' key pressed ignored: Auto Cut Out is active.");
            return true;
        }
        loopOutPosition = audioPlayer->getTransportSource().getCurrentPosition();
        DBG("  'o' key pressed. Setting loop out position to " << loopOutPosition);
        ensureLoopOrder(); // Call helper
        updateLoopButtonColors();
        updateLoopLabels();
        repaint();
        return true;
    }
    if (keyCode == 'u' || keyCode == 'U') {
        if (shouldAutoCutIn) {
            DBG("  'u' key pressed ignored: Auto Cut In is active.");
            return true;
        }
        DBG("  'u' key pressed. Clearing loop in.");
        clearLoopInButton.triggerClick();
        return true;
    }
    if (keyCode == 'p' || keyCode == 'P') {
        if (shouldAutoCutOut) {
            DBG("  'p' key pressed ignored: Auto Cut Out is active.");
            return true;
        }
        DBG("  'p' key pressed. Clearing loop out.");
        clearLoopOutButton.triggerClick();
        return true;
    }
    return false;
}

/**
 * @brief Handles global keybinds for application-wide actions.
 *
 * This function processes key presses for actions that should be available
 * regardless of the current application state, such as quitting or opening a file.
 *
 * @param key The `juce::KeyPress` object representing the key that was pressed.
 * @return `true` if the key press was handled, `false` otherwise.
 */
bool MainComponent::handleGlobalKeybinds(const juce::KeyPress& key) {
    auto keyCode = key.getTextCharacter();
    if (keyCode == 'e' || keyCode == 'E') {
        DBG("  'e' key pressed. Quitting application.");
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
        return true;
    }
    if (keyCode == 'd' || keyCode == 'D') {
        DBG("  'd' key pressed. Opening file chooser.");
        openButtonClicked();
        return true;
    }
    return false;
}

/**
 * @brief Seeks the audio playback to a specific position based on an X-coordinate.
 *
 * This function calculates the target playback position within the audio file
 * based on the provided X-coordinate relative to the waveform display.
 *
 * @param x The X-coordinate in pixels within the waveform display.
 */
void MainComponent::seekToPosition (int x) {
  if (audioPlayer->getThumbnail().getTotalLength() > 0.0) {
    auto relativeX = (double)(x - waveformBounds.getX());
    auto proportion = relativeX / (double)waveformBounds.getWidth();
    auto newPosition = juce::jlimit (0.0, 1.0, proportion) * audioPlayer->getThumbnail().getTotalLength();
    DBG("Seeking to position " << newPosition << " (x: " << x << ")");
    audioPlayer->getTransportSource().setPosition (newPosition);
  }
}

void MainComponent::mouseUp (const juce::MouseEvent& e) {
  DBG("Mouse Up event at (" << e.x << ", " << e.y << ")");
}

void MainComponent::mouseDown (const juce::MouseEvent& e) {
  DBG("Mouse Down event at (" << e.x << ", " << e.y << ")");
  if (e.mods.isRightButtonDown()) {
    DBG("  (right-click, ignoring)");
    return;
  }

  if (waveformBounds.contains (e.getMouseDownPosition())) {
    auto newPosition = (double)(e.x - waveformBounds.getX()) / (double)waveformBounds.getWidth() * audioPlayer->getThumbnail().getTotalLength();
    if (newPosition < 0.0) newPosition = 0.0;
    if (newPosition > audioPlayer->getThumbnail().getTotalLength()) newPosition = audioPlayer->getThumbnail().getTotalLength();

    if (currentPlacementMode == PlacementMode::LoopIn) {
      loopInPosition = newPosition;
      DBG("  Setting Loop In to: " << loopInPosition);
      currentPlacementMode = PlacementMode::None; // Reset mode
      ensureLoopOrder();
      updateLoopButtonColors();
      updateLoopLabels();
      repaint();
    } else if (currentPlacementMode == PlacementMode::LoopOut) {
      loopOutPosition = newPosition;
      DBG("  Setting Loop Out to: " << loopOutPosition);
      currentPlacementMode = PlacementMode::None; // Reset mode
      ensureLoopOrder();
      updateLoopButtonColors();
      updateLoopLabels();
      repaint();
    } else { // Normal seeking behavior
      DBG("  Seeking to position " << e.x);
      seekToPosition (e.x);
    }
  }
}

void MainComponent::mouseDrag (const juce::MouseEvent& e) {
  DBG("Mouse Drag event at (" << e.x << ", " << e.y << ")");
  if (e.mods.isRightButtonDown()) {
    DBG("  (right-click drag, ignoring)");
    return;
  }
  if (currentPlacementMode == PlacementMode::None && waveformBounds.contains (e.getPosition())) {
    DBG("  Seeking to position " << e.x);
    seekToPosition (e.x);
  }
}

void MainComponent::mouseMove (const juce::MouseEvent& e) {
  if (currentPlacementMode != PlacementMode::None && waveformBounds.contains (e.getPosition())) {
    juce::MouseCursor cursor = juce::MouseCursor::PointingHandCursor;
    setMouseCursor (cursor); }
    else {
      setMouseCursor (juce::MouseCursor::NormalCursor); }
    if (waveformBounds.contains (e.getPosition())) {
      if (mouseCursorX != e.x || mouseCursorY != e.y) {
        mouseCursorX = e.x;
        mouseCursorY = e.y;
        repaint(); }}
	  else if (mouseCursorX != -1) {
	    mouseCursorX = -1;
	    mouseCursorY = -1;
	    repaint(); }}



void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) {
  audioPlayer->getNextAudioBlock (bufferToFill); }

void MainComponent::timerCallback() {
  glowAlpha = 0.5f + 0.5f * std::sin(juce::Time::getMillisecondCounter() * Config::pulseSpeedFactor);

  if (!audioPlayer->isPlaying() && audioPlayer->getTransportSource().hasStreamFinished() && shouldLoop) {
    audioPlayer->getTransportSource().setPosition (0.0);
    audioPlayer->getTransportSource().start(); }
  if (shouldLoop && loopOutPosition > loopInPosition && audioPlayer->getTransportSource().getCurrentPosition() >= loopOutPosition) {
    audioPlayer->getTransportSource().setPosition (loopInPosition); }
  // No longer call updateButtonText() directly. Play/Stop button text is updated by changeListenerCallback.
  if (showStats) {
    juce::String debugInfo;
    debugInfo << "Samples Loaded: " << audioPlayer->getThumbnail().getNumSamplesFinished() << "\n";
    debugInfo << "Approx Peak: " << audioPlayer->getThumbnail().getApproximatePeak() << "\n";
    float minV, maxV;
    audioPlayer->getThumbnail().getApproximateMinMax(0.0, audioPlayer->getThumbnail().getTotalLength(), 0, minV, maxV);
    debugInfo << "Min/Max: " << minV << " / " << maxV;
    statsDisplay.setText (debugInfo, false); }
  if (audioPlayer->isPlaying() || showStats || audioPlayer->getThumbnail().getTotalLength() > 0.0 || shouldAutoCutIn || shouldAutoCutOut)
    repaint(); }

    void MainComponent::paint (juce::Graphics& g)
    {
      g.fillAll (Config::mainBackgroundColor);

      // Waveform drawing
      if (audioPlayer->getThumbnail().getNumChannels() > 0)
      {
        int pixelsPerSample = 1;
        if (currentQuality == ThumbnailQuality::Low)
          pixelsPerSample = 4;
        else if (currentQuality == ThumbnailQuality::Medium)
          pixelsPerSample = 2;

        if (currentChannelViewMode == ChannelViewMode::Mono || audioPlayer->getThumbnail().getNumChannels() == 1)
        {
          g.setColour (Config::waveformColor);
          if (pixelsPerSample > 1)
            drawReducedQualityWaveform(g, 0, pixelsPerSample);
          else
            audioPlayer->getThumbnail().drawChannel (g, waveformBounds, 0.0, audioPlayer->getThumbnail().getTotalLength(), 0, 1.0f);
        }
        else // Stereo
        {
          g.setColour (Config::waveformColor);
          if (pixelsPerSample > 1)
          {
            for (int ch = 0; ch < audioPlayer->getThumbnail().getNumChannels(); ++ch)
              drawReducedQualityWaveform(g, ch, pixelsPerSample);
          }
          else
          {
            audioPlayer->getThumbnail().drawChannels (g, waveformBounds, 0.0, audioPlayer->getThumbnail().getTotalLength(), 1.0f);
          }
        }
      } // End of waveform drawing

      auto audioLength = (float)audioPlayer->getThumbnail().getTotalLength();

      if (audioLength > 0.0) // Main block for drawing elements that depend on audio length
      {
        // Threshold visualization
        if (isCutModeActive) // Only draw threshold visualization if Cut mode is active
        {
          // Helper lambda to draw threshold lines at a given position
          auto drawThresholdVisualisation = [&](juce::Graphics& g_ref, double loopPos, float threshold, bool isActive)
          {
            if (audioLength <= 0.0) return; // Only draw if audio is loaded

            float normalisedThreshold = threshold; // threshold is 0.0-1.0
            float centerY = (float)waveformBounds.getCentreY();
            float halfHeight = (float)waveformBounds.getHeight() / 2.0f;

            // Calculate Y positions for the threshold lines
            float topThresholdY = centerY - (normalisedThreshold * halfHeight);
            float bottomThresholdY = centerY + (normalisedThreshold * halfHeight);

            // Ensure lines are within bounds
            topThresholdY = juce::jlimit((float)waveformBounds.getY(), (float)waveformBounds.getBottom(), topThresholdY);
            bottomThresholdY = juce::jlimit((float)waveformBounds.getY(), (float)waveformBounds.getBottom(), bottomThresholdY);

            // Calculate X position for the loop point
            float xPos = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (loopPos / audioLength);

            // Calculate start and end X for the Config::thresholdLineWidth wide line
            float halfThresholdLineWidth = Config::thresholdLineWidth / 2.0f;
            float lineStartX = xPos - halfThresholdLineWidth;
            float lineEndX = xPos + halfThresholdLineWidth;

            // Ensure lines are within waveformBounds horizontally
            lineStartX = juce::jmax(lineStartX, (float)waveformBounds.getX());
            lineEndX = juce::jmin(lineEndX, (float)waveformBounds.getRight());
            float currentLineWidth = lineEndX - lineStartX;

            // No dimming factors - always full color if active
            juce::Colour lineColor = Config::thresholdLineColor;
            juce::Colour regionColor = Config::thresholdRegionColor;

            // Draw the filled region (Config::thresholdLineWidth wide)
            g_ref.setColour(regionColor);
            g_ref.fillRect(lineStartX, topThresholdY, currentLineWidth, bottomThresholdY - topThresholdY);

            // --- GLOW EFFECT FOR THRESHOLD ---
            if (isActive) {
              juce::Colour glowColor = lineColor.withAlpha(lineColor.getFloatAlpha() * glowAlpha);
              g_ref.setColour(glowColor);
              // Draw a thicker line underneath the main line to create a glow
              g_ref.fillRect(lineStartX, topThresholdY - (Config::thresholdGlowThickness / 2.0f - 0.5f), currentLineWidth, Config::thresholdGlowThickness);
              g_ref.fillRect(lineStartX, bottomThresholdY - (Config::thresholdGlowThickness / 2.0f - 0.5f), currentLineWidth, Config::thresholdGlowThickness);
            }
            // --- END GLOW EFFECT FOR THRESHOLD ---

            // Draw the main threshold lines (Config::thresholdLineWidth wide) on top of the glow
            g_ref.setColour(lineColor);
            g_ref.drawHorizontalLine((int)topThresholdY, lineStartX, lineEndX);
            g_ref.drawHorizontalLine((int)bottomThresholdY, lineStartX, lineEndX);
          };

          // Draw In-Threshold Visualization
          drawThresholdVisualisation(g, loopInPosition, currentInSilenceThreshold, shouldAutoCutIn);

          // Draw Out-Threshold Visualization
          drawThresholdVisualisation(g, loopOutPosition, currentOutSilenceThreshold, shouldAutoCutOut);
        } // End of if (isCutModeActive) for threshold visualization

        // Loop Region drawing
        if (isCutModeActive) // Only draw loop region if Cut mode is active
        {
          auto actualIn = juce::jmin(loopInPosition, loopOutPosition);
          auto actualOut = juce::jmax(loopInPosition, loopOutPosition);
          auto inX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (actualIn / audioLength);
          auto outX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (actualOut / audioLength);
          g.setColour(Config::loopRegionColor); // No dimming factor
          g.fillRect(juce::Rectangle<float>(inX, (float)waveformBounds.getY(), outX - inX, (float)waveformBounds.getHeight()));
        }

        // Vertical loop lines
        if (isCutModeActive) // Only draw if Cut mode is active
        {
          auto inX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (loopInPosition / audioLength);
          auto outX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (loopOutPosition / audioLength);

          // Draw pulsing glow for vertical lines (always active if cut mode is active)
          juce::Colour glowColor = Config::loopLineColor.withAlpha(Config::loopLineColor.getFloatAlpha() * (1.0f - glowAlpha));
          g.setColour(glowColor);
          // Draw a thicker rectangle for the glow, centered on the line
          g.fillRect(inX - (Config::loopLineGlowThickness / 2.0f - 0.5f), (float)waveformBounds.getY(), Config::loopLineGlowThickness, (float)waveformBounds.getHeight());
          g.fillRect(outX - (Config::loopLineGlowThickness / 2.0f - 0.5f), (float)waveformBounds.getY(), Config::loopLineGlowThickness, (float)waveformBounds.getHeight());
        }

        // Playback Cursor
        auto drawPosition = (float)audioPlayer->getTransportSource().getCurrentPosition();
        auto x = (drawPosition / audioLength) * (float)waveformBounds.getWidth() + (float)waveformBounds.getX();
        if (audioPlayer->isPlaying())
        {
          juce::ColourGradient gradient (
            Config::playbackCursorGlowColorStart,
            (float)x - 10.0f, (float)waveformBounds.getCentreY(),
                                         Config::playbackCursorGlowColorEnd,
                                         (float)x, (float)waveformBounds.getCentreY(),
                                         false );
          g.setGradientFill (gradient);
          g.fillRect (juce::Rectangle<float>((int)x - 10, (float)waveformBounds.getY(), 10, (float)waveformBounds.getHeight()));
        }
        else
        {
          juce::ColourGradient glowGradient;
          glowGradient.addColour (0.0, Config::playbackCursorGlowColorStart);
          glowGradient.addColour (0.5, Config::playbackCursorGlowColorEnd);
          glowGradient.addColour (1.0, Config::playbackCursorColor.withAlpha(0.0f));
          glowGradient.point1 = { (float)x - 5.0f, (float)waveformBounds.getCentreY() };
          glowGradient.point2 = { (float)x + 5.0f, (float)waveformBounds.getCentreY() };
          g.setGradientFill (glowGradient);
          g.fillRect (juce::Rectangle<float>((int)x - 5, (float)waveformBounds.getY(), 10, (float)waveformBounds.getHeight()));
        }
        g.setColour (Config::playbackCursorColor);
        g.drawVerticalLine ((int)x, (float)waveformBounds.getY(), (float)waveformBounds.getBottom());
      } // End of main if (audioLength > 0.0) block for controls drawing

      // Mouse Cursor
      if (mouseCursorX != -1)
      {
        g.setColour (Config::mouseCursorHighlightColor);
        g.fillRect (mouseCursorX - 2, waveformBounds.getY(), 5, waveformBounds.getHeight());
        g.fillRect (waveformBounds.getX(), mouseCursorY - 2, waveformBounds.getWidth(), 5);

        // --- Amplitude lines at mouse cursor ---
        if (audioLength > 0.0) // Check audioLength for amplitude lines if drawing depends on it (it does for timeAtMouse calculation)
        {
          float minVal, maxVal;
          double timeAtMouse = (double)(mouseCursorX - waveformBounds.getX()) / (double)waveformBounds.getWidth() * audioLength;
          audioPlayer->getThumbnail().getApproximateMinMax(timeAtMouse, timeAtMouse + (audioLength / waveformBounds.getWidth()), 0, minVal, maxVal);

          float centerY = (float)waveformBounds.getCentreY();
          float halfHeight = (float)waveformBounds.getHeight() / 2.0f;

          float topAmplitudeY = centerY - (maxVal * halfHeight);
          float bottomAmplitudeY = centerY - (minVal * halfHeight); // Corrected calculation

          // Clamp to waveform bounds
          topAmplitudeY = juce::jlimit((float)waveformBounds.getY(), (float)waveformBounds.getBottom(), topAmplitudeY);
          bottomAmplitudeY = juce::jlimit((float)waveformBounds.getY(), (float)waveformBounds.getBottom(), bottomAmplitudeY);

          float lineLength = Config::mouseAmplitudeLineLength;
          float halfLineLength = lineLength / 2.0f;
          float lineStartX = mouseCursorX - halfLineLength;
          float lineEndX = mouseCursorX + halfLineLength;

          // Clamp to waveformBounds horizontally
          lineStartX = juce::jmax(lineStartX, (float)waveformBounds.getX());
          lineEndX = juce::jmin(lineEndX, (float)waveformBounds.getRight());
          float currentLineWidth = lineEndX - lineStartX;

          // Draw glow
          g.setColour(Config::mouseAmplitudeGlowColor);
          g.fillRect(lineStartX, topAmplitudeY - (Config::mouseAmplitudeGlowThickness / 2.0f - 0.5f),
                     currentLineWidth, Config::mouseAmplitudeGlowThickness);
          g.fillRect(lineStartX, bottomAmplitudeY - (Config::mouseAmplitudeGlowThickness / 2.0f - 0.5f),
                     currentLineWidth, Config::mouseAmplitudeGlowThickness);

          // Draw main lines
          g.setColour(Config::mouseAmplitudeLineColor);
          g.fillRect(lineStartX, topAmplitudeY - (Config::mouseAmplitudeLineThickness / 2.0f - 0.5f),
                     currentLineWidth, Config::mouseAmplitudeLineThickness);
          g.fillRect(lineStartX, bottomAmplitudeY - (Config::mouseAmplitudeLineThickness / 2.0f - 0.5f),
                     currentLineWidth, Config::mouseAmplitudeLineThickness);

          // --- Display Time at Mouse Cursor ---
          juce::String timeString = formatTime(timeAtMouse);
          g.setColour(Config::playbackTextColor);
          g.setFont(Config::mouseCursorTextSize);
          // Position slightly above the mouse's horizontal line
          g.drawText(timeString,
                     mouseCursorX + 5, // A little to the right of the vertical line
                     mouseCursorY - Config::mouseCursorTextSize - 5, // Above the horizontal line
                     200, // Sufficient width
                     Config::mouseCursorTextSize,
                     juce::Justification::left,
                     false);

          // --- Display Amplitude at Mouse Cursor (Percentage) ---
          juce::String maxAmpString = juce::String::formatted("+%.2f%%", maxVal * 100.0f);
          juce::String minAmpString = juce::String::formatted("%.2f%%", minVal * 100.0f); // minVal is already negative

          g.setColour(Config::mouseAmplitudeLineColor); // Use the amplitude line color for text
          g.setFont(Config::mouseCursorTextSize);

          // Draw max amplitude percentage near topAmplitudeY
          g.drawText(maxAmpString,
                     lineEndX + 5, // A little to the right of the amplitude line
                     topAmplitudeY - (Config::mouseCursorTextSize / 2.0f), // Vertically centered on the line
                     100, // Sufficient width
                     Config::mouseCursorTextSize,
                     juce::Justification::left,
                     false);

          // Draw min amplitude percentage near bottomAmplitudeY
          g.drawText(minAmpString,
                     lineEndX + 5, // A little to the right of the amplitude line
                     bottomAmplitudeY - (Config::mouseCursorTextSize / 2.0f), // Vertically centered on the line
                     100, // Sufficient width
                     Config::mouseCursorTextSize,
                     juce::Justification::left,
                     false);
        } // End of if (audioLength > 0.0) for mouse cursor amplitude drawing

        g.setColour (Config::mouseCursorLineColor);
        g.drawVerticalLine (mouseCursorX, (float)waveformBounds.getY(), (float)waveformBounds.getBottom());
        g.drawHorizontalLine (mouseCursorY, (float)waveformBounds.getX(), (float)waveformBounds.getRight());
      } // End of if (mouseCursorX != -1)

      // Playback time display (bottom row)
      if (audioLength > 0.0)
      {
        double currentTime = audioPlayer->getTransportSource().getCurrentPosition();
        double totalTime = audioPlayer->getThumbnail().getTotalLength(); // totalTime is calculated but totalTimeStaticStr is used for display
        double remainingTime = totalTime - currentTime;

        juce::String currentTimeStr = formatTime(currentTime);
        juce::String remainingTimeStr = formatTime(remainingTime);

        int textY = bottomRowTopY - Config::playbackTimeTextOffsetY;

        g.setColour(Config::playbackTextColor);
        g.setFont(Config::playbackTextSize);

        // Draw Current Time (Left)
        g.drawText(currentTimeStr, playbackLeftTextX, textY, Config::playbackTextWidth, Config::playbackTextHeight, juce::Justification::left, false);

        // Draw Total Time (Center)
        g.drawText(totalTimeStaticStr, playbackCenterTextX, textY, Config::playbackTextWidth, 20, juce::Justification::centred, false);

        // Draw Remaining Time (Right)
        g.drawText(remainingTimeStr, playbackRightTextX, textY, Config::playbackTextWidth, 20, juce::Justification::right, false);
      }
    } // Final closing brace for paint.

// juce::TextEditor::Listener callbacks
void MainComponent::textEditorTextChanged (juce::TextEditor& editor) {
    DBG("Text Editor Text Changed.");
    if (&editor == &inSilenceThresholdEditor) {
        DBG("  In Silence Threshold Editor: " << editor.getText());
        int newPercentage = editor.getText().getIntValue();
        if (newPercentage >= 1 && newPercentage <= 99) {
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Valid
        } else {
            editor.setColour(juce::TextEditor::textColourId, Config::textEditorOutOfRangeColour); // Out of range
        }
    } else if (&editor == &outSilenceThresholdEditor) {
        DBG("  Out Silence Threshold Editor: " << editor.getText());
        int newPercentage = editor.getText().getIntValue();
        if (newPercentage >= 1 && newPercentage <= 99) {
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Valid
        } else {
            editor.setColour(juce::TextEditor::textColourId, Config::textEditorOutOfRangeColour); // Out of range
        }
    } else {
        DBG("  Loop Editor Text Changed: " << editor.getText());
        double totalLength = audioPlayer->getThumbnail().getTotalLength();
        double newPosition = parseTime(editor.getText());

        if (newPosition >= 0.0 && newPosition <= totalLength) {
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Valid and in range
        } else if (newPosition == -1.0) {
            editor.setColour(juce::TextEditor::textColourId, Config::textEditorErrorColor); // Completely invalid format
        } else {
            editor.setColour(juce::TextEditor::textColourId, Config::textEditorWarningColor); // Valid format but out of range
        }
    }
}

void MainComponent::textEditorReturnKeyPressed (juce::TextEditor& editor) {
    DBG("Text Editor Return Key Pressed.");
    if (&editor == &loopInEditor) {
        DBG("  Loop In Editor: Return Key Pressed");
        if (shouldAutoCutIn) {
            DBG("  Loop In Editor: Return Key Pressed ignored: Auto Cut In is active.");
            editor.setText(formatTime(loopInPosition), juce::dontSendNotification); // Revert text
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
            editor.giveAwayKeyboardFocus();
            return;
        }
        double newPosition = parseTime(editor.getText());
        if (newPosition >= 0.0 && newPosition <= audioPlayer->getThumbnail().getTotalLength()) {
            // Validate against loopOutPosition if it's set
            if (loopOutPosition > -1.0 && newPosition > loopOutPosition) {
                // Invalid: new loopIn is after loopOut, revert to current loopIn
                            editor.setText(formatTime(loopInPosition), juce::dontSendNotification);
                            editor.setColour(juce::TextEditor::textColourId, Config::textEditorWarningColor); // Indicate warning
                        } else {
                            loopInPosition = newPosition;
                            DBG("    Loop In position set to: " << loopInPosition);
                            updateLoopButtonColors();
                            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
                            repaint();
                        }
                    } else {
                        // Revert to last valid position if input is invalid (out of bounds)
                        editor.setText(formatTime(loopInPosition), juce::dontSendNotification);
                        editor.setColour(juce::TextEditor::textColourId, Config::textEditorErrorColor); // Indicate error
                    }
                } else if (&editor == &loopOutEditor) {
                    DBG("  Loop Out Editor: Return Key Pressed");
                    if (shouldAutoCutOut) {
                        DBG("  Loop Out Editor: Return Key Pressed ignored: Auto Cut Out is active.");
                        editor.setText(formatTime(loopOutPosition), juce::dontSendNotification); // Revert text
                        editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
                        editor.giveAwayKeyboardFocus();
                        return;
                    }
                    double newPosition = parseTime(editor.getText());
                    if (newPosition >= 0.0 && newPosition <= audioPlayer->getThumbnail().getTotalLength()) {
                        if (shouldLoop && audioPlayer->getTransportSource().getCurrentPosition() >= loopOutPosition)
                        {
                            audioPlayer->getTransportSource().setPosition(loopInPosition);
                        }
                        // Validate against loopInPosition if it's set
                        if (loopInPosition > -1.0 && newPosition < loopInPosition) { // Uncommented and properly nested
                            // Invalid: new loopOut is before loopIn, revert to current loopOut
                            editor.setText(formatTime(loopOutPosition), juce::dontSendNotification);
                            editor.setColour(juce::TextEditor::textColourId, Config::textEditorWarningColor); // Indicate warning
                        } else { // This else belongs to the inner 'if'
                            loopOutPosition = newPosition;
                            DBG("    Loop Out position set to: " << loopOutPosition);
                            updateLoopButtonColors();
                            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
                            repaint();
                        }
                    } else { // This 'else' correctly handles out-of-bounds input for loopOutEditor
                        // Revert to last valid position if input is invalid (out of bounds)
                        editor.setText(formatTime(loopOutPosition), juce::dontSendNotification);
                        editor.setColour(juce::TextEditor::textColourId, Config::textEditorErrorColor); // Indicate error
                    }    } else if (&editor == &inSilenceThresholdEditor)
    {
        DBG("  In Silence Threshold Editor: Return Key Pressed");
        int newPercentage = editor.getText().getIntValue();
        if (newPercentage >= 1 && newPercentage <= 99)
        {
            currentInSilenceThreshold = static_cast<float>(newPercentage) / 100.0f;
            DBG("    In Silence threshold set to: " << currentInSilenceThreshold);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
            detectInSilence(); // Trigger detection if mode is active (always after pressing enter on threshold)
        }
        else
        {
            editor.setText(juce::String(static_cast<int>(currentInSilenceThreshold * 100.0f)), juce::dontSendNotification);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
        }
    } else if (&editor == &outSilenceThresholdEditor)
    {
        DBG("  Out Silence Threshold Editor: Return Key Pressed");
        int newPercentage = editor.getText().getIntValue();
        if (newPercentage >= 1 && newPercentage <= 99)
        {
            currentOutSilenceThreshold = static_cast<float>(newPercentage) / 100.0f;
            DBG("    Out Silence threshold set to: " << currentOutSilenceThreshold);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
            detectOutSilence(); // Trigger detection
        }
        else
        {
            editor.setText(juce::String(static_cast<int>(currentOutSilenceThreshold * 100.0f)), juce::dontSendNotification);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
        }
    }
    editor.giveAwayKeyboardFocus(); // Lose focus after pressing enter
} // Add this closing brace


void MainComponent::textEditorEscapeKeyPressed (juce::TextEditor& editor) {
    DBG("Text Editor Escape Key Pressed.");
    // Revert text to current value on escape
    if (&editor == &loopInEditor) {
        DBG("  Loop In Editor: Escape Key Pressed");
        editor.setText(formatTime(loopInPosition), juce::dontSendNotification);
    } else if (&editor == &loopOutEditor) {
        DBG("  Loop Out Editor: Escape Key Pressed");
        editor.setText(formatTime(loopOutPosition), juce::dontSendNotification);
    } else if (&editor == &inSilenceThresholdEditor) {
        DBG("  In Silence Threshold Editor: Escape Key Pressed");
        editor.setText(juce::String(static_cast<int>(currentInSilenceThreshold * 100.0f)), juce::dontSendNotification);
    } else if (&editor == &outSilenceThresholdEditor) {
        DBG("  Out Silence Threshold Editor: Escape Key Pressed");
        editor.setText(juce::String(static_cast<int>(currentOutSilenceThreshold * 100.0f)), juce::dontSendNotification);
    }
    editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
    editor.giveAwayKeyboardFocus(); // Lose focus
}

void MainComponent::textEditorFocusLost (juce::TextEditor& editor) {
    DBG("Text Editor Focus Lost.");
    // Similar logic to return key pressed, but always revert if invalid on focus lost
    if (&editor == &loopInEditor) {
        DBG("  Loop In Editor: Focus Lost");
        if (shouldAutoCutIn) {
            DBG("  Loop In Editor: Focus Lost ignored: Auto Cut In is active.");
            editor.setText(formatTime(loopInPosition), juce::dontSendNotification); // Revert text
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
            return;
        }
        double newPosition = parseTime(editor.getText());
        if (newPosition >= 0.0 && newPosition <= audioPlayer->getThumbnail().getTotalLength()) {
            // Validate against loopOutPosition if it's set
            if (loopOutPosition > -1.0 && newPosition > loopOutPosition) {
                // Invalid: new loopIn is after loopOut, revert to current loopIn
                            editor.setText(formatTime(loopInPosition), juce::dontSendNotification);
                            editor.setColour(juce::TextEditor::textColourId, Config::textEditorWarningColor); // Indicate warning
                        } else {
                            loopInPosition = newPosition;
                            DBG("    Loop In position set to: " << loopInPosition);
                            updateLoopButtonColors();
                            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
                            repaint();
                        }
                    } else {
                        // Revert to last valid position if input is invalid (out of bounds)
                        editor.setText(formatTime(loopInPosition), juce::dontSendNotification);
                        editor.setColour(juce::TextEditor::textColourId, Config::textEditorErrorColor); // Indicate error
                        repaint();
                    }
                } else if (&editor == &loopOutEditor) {
                    DBG("  Loop Out Editor: Focus Lost");
                    if (shouldAutoCutOut) {
                        DBG("  Loop Out Editor: Focus Lost ignored: Auto Cut Out is active.");
                        editor.setText(formatTime(loopOutPosition), juce::dontSendNotification); // Revert text
                        editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
                        return;
                    }
                    double newPosition = parseTime(editor.getText());
                    if (newPosition >= 0.0 && newPosition <= audioPlayer->getThumbnail().getTotalLength()) {
                        // Validate against loopInPosition if it's set
                        if (loopInPosition > -1.0 && newPosition < loopInPosition) {
                            // Invalid: new loopOut is before loopIn, revert to current loopOut
                            editor.setText(formatTime(loopOutPosition), juce::dontSendNotification);
                            editor.setColour(juce::TextEditor::textColourId, Config::textEditorWarningColor); // Indicate warning
                        } else {
                            loopOutPosition = newPosition;
                            DBG("    Loop Out position set to: " << loopOutPosition);
                            updateLoopButtonColors();
                            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
                            repaint();
                        }
                    } else { // This 'else' correctly handles out-of-bounds input for loopOutEditor
                        // Revert to last valid position if input is invalid (out of bounds)
                        editor.setText(formatTime(loopOutPosition), juce::dontSendNotification);
                        editor.setColour(juce::TextEditor::textColourId, Config::textEditorErrorColor); // Indicate error
                    }    } else if (&editor == &inSilenceThresholdEditor) {
        DBG("  In Silence Threshold Editor: Focus Lost");
        int newPercentage = editor.getText().getIntValue();
        if (newPercentage >= 1 && newPercentage <= 99)
        {
            currentInSilenceThreshold = static_cast<float>(newPercentage) / 100.0f;
            DBG("    In Silence threshold set to: " << currentInSilenceThreshold);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
            detectInSilence(); // Trigger detection
        }
        else
        {
            editor.setText(juce::String(static_cast<int>(currentInSilenceThreshold * 100.0f)), juce::dontSendNotification);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
        }
    } else if (&editor == &outSilenceThresholdEditor) {
        DBG("  Out Silence Threshold Editor: Focus Lost");
        int newPercentage = editor.getText().getIntValue();
        if (newPercentage >= 1 && newPercentage <= 99)
        {
            currentOutSilenceThreshold = static_cast<float>(newPercentage) / 100.0f;
            DBG("    Out Silence threshold set to: " << currentOutSilenceThreshold);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
            detectOutSilence(); // Trigger detection
        }
        else
        {
            editor.setText(juce::String(static_cast<int>(currentOutSilenceThreshold * 100.0f)), juce::dontSendNotification);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
        }
    }
}

void MainComponent::updateLoopLabels() {
  loopInEditor.setText(formatTime(loopInPosition), juce::dontSendNotification);
  
  loopOutEditor.setText(formatTime(loopOutPosition), juce::dontSendNotification);
}

void MainComponent::detectInSilence()
{
    DBG("detectInSilence() called.");
    bool wasPlaying = audioPlayer->isPlaying();
    if (wasPlaying) {
        audioPlayer->getTransportSource().stop();
        DBG("  Playback stopped for silence detection.");
    }

    juce::AudioFormatReader* reader = audioPlayer->getAudioFormatReader();

    if (!reader) {
        DBG("  AudioFormatReader is null. Returning.");
        if (wasPlaying) { // If we stopped playback, start it again before returning
            audioPlayer->getTransportSource().start();
        }
        return;
    }
    
    int numChannels = reader->numChannels;
    juce::int64 lengthInSamples = reader->lengthInSamples;
    DBG("  Reader: numChannels = " << numChannels << ", lengthInSamples = " << lengthInSamples);

    if (lengthInSamples <= 0) {
        DBG("  Audio length in samples is non-positive. Returning.");
        if (wasPlaying) { // If we stopped playback, start it again before returning
            audioPlayer->getTransportSource().start();
        }
        return;
    }

    auto buffer = std::make_unique<juce::AudioBuffer<float>>(numChannels, (int)lengthInSamples);
    DBG("  AudioBuffer allocated with numChannels = " << buffer->getNumChannels() << ", numSamples = " << buffer->getNumSamples());
    reader->read(buffer.get(), 0, (int)lengthInSamples, 0, true, true);
    DBG("  Audio data read into buffer.");

    float threshold = currentInSilenceThreshold; // Use the member variable
    DBG("  Silence detection threshold (in): " << threshold);

    int firstSample = -1;

    for (int i = 0; i < buffer->getNumSamples(); ++i)
    {
        bool isSilent = true;
        for (int j = 0; j < buffer->getNumChannels(); ++j)
        {
            if (std::abs(buffer->getSample(j, i)) > threshold)
            {
                isSilent = false;
                break;
            }
        }

        if (!isSilent)
        {
            firstSample = i;
            break; // Found the first non-silent sample
        }
    }
    DBG("  First non-silent sample: " << firstSample);


    if (firstSample != -1)
    {
        // Find previous zero crossing for loop in
        int loopInSample = 0;
        for (int i = firstSample; i > 0; --i)
        {
            // Ensure valid index before access
            if (i >= 1 && i < buffer->getNumSamples()) {
                bool sign1 = buffer->getSample(0, i) >= 0;
                bool sign2 = buffer->getSample(0, i - 1) >= 0;
                if (sign1 != sign2)
                {
                    loopInSample = i;
                    break;
                }
            } else {
                DBG("  ERROR: Invalid index for zero crossing (loop in) at i=" << i);
            }
        }
        DBG("  Calculated loopInSample (zero crossing): " << loopInSample);

        loopInPosition = (double)loopInSample / reader->sampleRate;
        DBG("  Setting loopInPosition: " << loopInPosition);

        // Jump playhead to loopInPosition
        audioPlayer->getTransportSource().setPosition(loopInPosition);
        DBG("  Transport source position set to: " << loopInPosition);

        updateLoopButtonColors();
        updateLoopLabels();
        repaint();
    } else {
        DBG("  No non-silent samples found for loop in. loopInPosition unchanged.");
    }

    if (wasPlaying) {
        audioPlayer->getTransportSource().start();
        DBG("  Playback restarted.");
    }
    DBG("detectInSilence() finished.");
}

void MainComponent::detectOutSilence()
{
    DBG("detectOutSilence() called.");
    bool wasPlaying = audioPlayer->isPlaying();
    if (wasPlaying) {
        audioPlayer->getTransportSource().stop();
        DBG("  Playback stopped for silence detection.");
    }

    juce::AudioFormatReader* reader = audioPlayer->getAudioFormatReader();

    if (!reader) {
        DBG("  AudioFormatReader is null. Returning.");
        if (wasPlaying) {
            audioPlayer->getTransportSource().start();
        }
        return;
    }
    
    int numChannels = reader->numChannels;
    juce::int64 lengthInSamples = reader->lengthInSamples;
    DBG("  Reader: numChannels = " << numChannels << ", lengthInSamples = " << lengthInSamples);

    if (lengthInSamples <= 0) {
        DBG("  Audio length in samples is non-positive. Returning.");
        if (wasPlaying) {
            audioPlayer->getTransportSource().start();
        }
        return;
    }

    auto buffer = std::make_unique<juce::AudioBuffer<float>>(numChannels, (int)lengthInSamples);
    DBG("  AudioBuffer allocated with numChannels = " << buffer->getNumChannels() << ", numSamples = " << buffer->getNumSamples());
    reader->read(buffer.get(), 0, (int)lengthInSamples, 0, true, true);
    DBG("  Audio data read into buffer.");

    float threshold = currentOutSilenceThreshold; // Use the OUT threshold
    DBG("  Silence detection threshold (out): " << threshold);

    int lastSample = -1; // We're interested in the *last* non-silent sample

    for (int i = buffer->getNumSamples() - 1; i >= 0; --i) // Iterate backwards
    {
        bool isSilent = true;
        for (int j = 0; j < buffer->getNumChannels(); ++j)
        {
            if (std::abs(buffer->getSample(j, i)) > threshold)
            {
                isSilent = false;
                break;
            }
        }

        if (!isSilent)
        {
            lastSample = i;
            break; // Found the last non-silent sample
        }
    }
    DBG("  Last non-silent sample: " << lastSample);


    if (lastSample != -1)
    {
        // Find next zero crossing for loop out (after last non-silent sample)
        int loopOutSample = buffer->getNumSamples() - 1; // Default to end of file if no zero crossing found
        for (int i = lastSample; i < buffer->getNumSamples() - 1; ++i)
        {
            if (i >= 0 && i < buffer->getNumSamples() - 1) {
                bool sign1 = buffer->getSample(0, i) >= 0;
                bool sign2 = buffer->getSample(0, i + 1) >= 0;
                if (sign1 != sign2)
                {
                    loopOutSample = i;
                    break;
                }
            } else {
                DBG("  ERROR: Invalid index for zero crossing (loop out) at i=" << i);
            }
        }
        DBG("  Calculated loopOutSample (zero crossing): " << loopOutSample);

        loopOutPosition = (double)loopOutSample / reader->sampleRate;
        DBG("  Setting loopOutPosition: " << loopOutPosition);

        audioPlayer->getTransportSource().setPosition(loopOutPosition);
        DBG("  Transport source position set to: " << loopOutPosition);

        updateLoopButtonColors();
        updateLoopLabels();
        repaint();
    } else {
        DBG("  No non-silent samples found for loop out. loopOutPosition unchanged.");
    }

    if (wasPlaying) {
        audioPlayer->getTransportSource().start();
        DBG("  Playback restarted.");
    }
    DBG("detectOutSilence() finished.");
}

void MainComponent::ensureLoopOrder()
{
    if (loopInPosition > loopOutPosition)
    {
        std::swap(loopInPosition, loopOutPosition);
    }
}

void MainComponent::resized() {

  auto bounds = getLocalBounds();

  int rowHeight = Config::buttonHeight + Config::windowBorderMargins * 2; // Calculate rowHeight here



    layoutTopRowButtons(bounds, rowHeight); // Pass rowHeight to the function



  



    layoutLoopAndCutControls(bounds, rowHeight);

  layoutBottomRowAndTextDisplay(bounds, rowHeight);

  layoutWaveformAndStats(bounds);
}





void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate) {
  audioPlayer->prepareToPlay (samplesPerBlockExpected, sampleRate); }

void MainComponent::changeListenerCallback (juce::ChangeBroadcaster* source) {
  if (source == audioPlayer.get())
  {
    if (audioPlayer->isPlaying())
      playStopButton.setButtonText (Config::stopButtonText);
    else
      playStopButton.setButtonText (Config::playButtonText);
    repaint();
  }
}

void MainComponent::releaseResources() { audioPlayer->releaseResources(); }

void MainComponent::updateComponentStates() {
  const bool enabled = audioPlayer->getThumbnail().getTotalLength() > 0.0; // Base enable state depends on file loaded
  const bool cutControlsActive = isCutModeActive && enabled; // Cut controls enabled only if Cut mode is active AND file is loaded

  updateGeneralButtonStates(enabled); // Call the new function

  updateCutModeControlStates(isCutModeActive, enabled, shouldAutoCutIn, shouldAutoCutOut);
}

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
void MainComponent::updateCutModeControlStates(bool isCutModeActive, bool enabled, bool shouldAutoCutIn, bool shouldAutoCutOut)
{
  // Controls related to "Cut" mode: their enabled and visible state depends on isCutModeActive
  // Manual loop point controls are enabled only if Cut mode is active AND their auto-cut is NOT active
  loopInButton.setEnabled(isCutModeActive && !shouldAutoCutIn);
  loopOutButton.setEnabled(isCutModeActive && !shouldAutoCutOut);
  loopInEditor.setEnabled(isCutModeActive && !shouldAutoCutIn);
  loopOutEditor.setEnabled(isCutModeActive && !shouldAutoCutOut);
  clearLoopInButton.setEnabled(isCutModeActive && !shouldAutoCutIn);
  clearLoopOutButton.setEnabled(isCutModeActive && !shouldAutoCutOut);

  // Threshold editors are always enabled. Their visibility is still tied to Cut mode.
  inSilenceThresholdEditor.setEnabled(true);
  outSilenceThresholdEditor.setEnabled(true);

  // Auto-cut buttons are enabled only if Cut mode is active
  autoCutInButton.setEnabled(isCutModeActive);
  autoCutOutButton.setEnabled(isCutModeActive);

  // Set visibility for "Cut" mode related controls
  loopInButton.setVisible(isCutModeActive);
  loopOutButton.setVisible(isCutModeActive);
  loopInEditor.setVisible(isCutModeActive);
  loopOutEditor.setVisible(isCutModeActive);
  clearLoopInButton.setVisible(isCutModeActive);
  clearLoopOutButton.setVisible(isCutModeActive);


  inSilenceThresholdEditor.setVisible(isCutModeActive);



  outSilenceThresholdEditor.setVisible(isCutModeActive);


  autoCutInButton.setVisible(isCutModeActive);
  autoCutOutButton.setVisible(isCutModeActive);
}

/**
 * @brief Updates the enabled and visible states of general UI buttons.
 *
 * This function sets the enabled and visible states for buttons that are
 * always active or depend only on whether a file is currently loaded,
 * regardless of the cut mode.
 *
 * @param enabled A boolean indicating whether a file is loaded (true) or not (false).
 */
void MainComponent::updateGeneralButtonStates(bool enabled)
{
  // Buttons that should always be enabled (and visible)
  openButton.setEnabled(true);
  exitButton.setEnabled(true);
  loopButton.setEnabled(true);
  autoplayButton.setEnabled(true);
  cutButton.setEnabled(true);

  // Buttons that depend on a file being loaded (but not necessarily isCutModeActive)
  playStopButton.setEnabled(enabled);
  modeButton.setEnabled(enabled);
  statsButton.setEnabled(enabled);
  channelViewButton.setEnabled(enabled);
  qualityButton.setEnabled(enabled);
  statsDisplay.setEnabled(enabled);
}



void MainComponent::focusGained (juce::Component::FocusChangeType cause) {
  ignoreUnused (cause);
  grabKeyboardFocus();
}
