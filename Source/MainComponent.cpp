#include "MainComponent.h"
#include "Config.h"

MainComponent::MainComponent() : thumbnailCache (5), thumbnail (512, formatManager, thumbnailCache),
								 loopInButton("Loop In Button"), loopOutButton("Loop Out Button") {
  formatManager.registerFormat (new juce::WavAudioFormat(), false);
  formatManager.registerFormat (new juce::AiffAudioFormat(), false);
  formatManager.registerFormat (new juce::FlacAudioFormat(), false);
  formatManager.registerFormat (new juce::OggVorbisAudioFormat(), false);
  formatManager.registerFormat (new juce::MP3AudioFormat(), false);

  thumbnail.addChangeListener (this);

  setLookAndFeel (&modernLF);
  modernLF.setBaseOffColor(Config::buttonBaseColour);
  modernLF.setBaseOnColor(Config::buttonOnColour);
  modernLF.setTextColor(Config::buttonTextColour);

  addAndMakeVisible (openButton);
  openButton.setButtonText ("[D]ir");
  openButton.onClick = [this] { openButtonClicked(); };

  addAndMakeVisible (playStopButton);
  updateButtonText();
  playStopButton.onClick = [this] { playStopButtonClicked(); };
  playStopButton.setEnabled (false);

  addAndMakeVisible (modeButton);
  modeButton.setButtonText ("[V]iew");
  modeButton.setClickingTogglesState (true);
  modeButton.onClick = [this] {
    DBG("Button Clicked: Mode, new state: " << (modeButton.getToggleState() ? "Overlay" : "Classic"));
    currentMode = modeButton.getToggleState() ? ViewMode::Overlay : ViewMode::Classic;
    modeButton.setButtonText (currentMode == ViewMode::Classic ? "[V]iew01" : "[V]iew02");
    resized();
    repaint(); };

  addAndMakeVisible (channelViewButton);
  channelViewButton.setButtonText ("[C]han");
  channelViewButton.setClickingTogglesState (true);
  channelViewButton.onClick = [this] {
    DBG("Button Clicked: Channel View, new state: " << (channelViewButton.getToggleState() ? "Stereo" : "Mono"));
    currentChannelViewMode = channelViewButton.getToggleState() ? ChannelViewMode::Stereo : ChannelViewMode::Mono;
    channelViewButton.setButtonText (currentChannelViewMode == ChannelViewMode::Mono ? "[C]han 1" : "[C]han 2");
    repaint(); };

  addAndMakeVisible (qualityButton);
  qualityButton.setButtonText ("[Q]ual");
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

  addAndMakeVisible (exitButton);
  exitButton.setButtonText ("[E]xit");
  exitButton.setColour (juce::TextButton::buttonColourId, Config::exitButtonColor);
  exitButton.onClick = [] {
    DBG("Button Clicked: Exit - System Quit Requested");
    juce::JUCEApplication::getInstance()->systemRequestedQuit(); };

  addAndMakeVisible (statsButton);
  statsButton.setButtonText ("[S]tats");
  statsButton.setClickingTogglesState (true);
  statsButton.onClick = [this] {
    DBG("Button Clicked: Stats, new state: " << (statsButton.getToggleState() ? "Visible" : "Hidden"));
    showStats = statsButton.getToggleState();
    resized(); };

  addAndMakeVisible (loopButton);
  loopButton.setButtonText ("[L]oop");
  loopButton.setClickingTogglesState (true);
  loopButton.onClick = [this] {
    DBG("Button Clicked: Loop, new state: " << (loopButton.getToggleState() ? "On" : "Off"));
    shouldLoop = loopButton.getToggleState(); };

  addAndMakeVisible (autoplayButton);
  autoplayButton.setButtonText (Config::autoplayButtonText);
  autoplayButton.setClickingTogglesState (true);
  autoplayButton.setToggleState (shouldAutoplay, juce::dontSendNotification);
  autoplayButton.onClick = [this] {
    DBG("Button Clicked: Autoplay, new state: " << (autoplayButton.getToggleState() ? "On" : "Off"));
    shouldAutoplay = autoplayButton.getToggleState();
    if (shouldAutoplay && isFileLoaded && !transportSource.isPlaying()) {
        playStopButtonClicked();
    }
  };

  addAndMakeVisible (autoCutInButton);
  autoCutInButton.setButtonText (Config::autoCutInButtonText);
  autoCutInButton.setClickingTogglesState (true);
  autoCutInButton.setToggleState (shouldAutoCutIn, juce::dontSendNotification);
  autoCutInButton.onClick = [this] {
    DBG("Button Clicked: Auto Cut In, new state: " << (autoCutInButton.getToggleState() ? "On" : "Off"));
    shouldAutoCutIn = autoCutInButton.getToggleState();
  };

  addAndMakeVisible (autoCutOutButton);
  autoCutOutButton.setButtonText (Config::autoCutOutButtonText);
  autoCutOutButton.setClickingTogglesState (true);
  autoCutOutButton.setToggleState (shouldAutoCutOut, juce::dontSendNotification);
  autoCutOutButton.onClick = [this] {
    DBG("Button Clicked: Auto Cut Out, new state: " << (autoCutOutButton.getToggleState() ? "On" : "Off"));
    shouldAutoCutOut = autoCutOutButton.getToggleState();
  };

  addAndMakeVisible (cutButton);
  cutButton.setButtonText (Config::cutButtonText);
  cutButton.setClickingTogglesState (true);
  cutButton.setToggleState (isCutModeActive, juce::dontSendNotification);
  cutButton.onClick = [this] {
    DBG("Button Clicked: Cut Mode, new state: " << (cutButton.getToggleState() ? "Active" : "Inactive"));
    isCutModeActive = cutButton.getToggleState();
    updateComponentStates(); // Update visibility/enabled state of cut-related controls
  };

  addAndMakeVisible (loopInButton);
  loopInButton.setButtonText ("[I]n");
    loopInButton.onLeftClick = [this] {
      loopInPosition = transportSource.getCurrentPosition();
      DBG("Loop In Button Left Clicked. Position: " << loopInPosition);    ensureLoopOrder(); // Call helper
    updateLoopButtonColors();
    repaint(); };
  loopInButton.onRightClick = [this] {
    DBG("Loop In Button Right Clicked. Setting placement mode to LoopIn.");
    currentPlacementMode = PlacementMode::LoopIn;
    updateLoopButtonColors();
    repaint(); };

  addAndMakeVisible (loopOutButton);
  loopOutButton.setButtonText ("[O]ut");
  loopOutButton.onLeftClick = [this] {
    loopOutPosition = transportSource.getCurrentPosition();
    DBG("Loop Out Button Left Clicked. Position: " << loopOutPosition);
    ensureLoopOrder(); // Call helper
    updateLoopButtonColors();
    repaint(); };
  loopOutButton.onRightClick = [this] {
    DBG("Loop Out Button Right Clicked. Setting placement mode to LoopOut.");
    currentPlacementMode = PlacementMode::LoopOut;
    updateLoopButtonColors();
    repaint(); };

  addAndMakeVisible (statsDisplay);
  statsDisplay.setReadOnly (true);
  statsDisplay.setMultiLine (true);
  statsDisplay.setWantsKeyboardFocus (false);
  statsDisplay.setColour (juce::TextEditor::backgroundColourId, Config::statsDisplayBackgroundColour);
  statsDisplay.setColour (juce::TextEditor::textColourId, Config::statsDisplayTextColour);
  statsDisplay.setVisible (false);

  addAndMakeVisible (loopInEditor);
  loopInEditor.setReadOnly (false);
  loopInEditor.setJustification(juce::Justification::centred);
  loopInEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colours::grey.withAlpha(Config::playbackTextBackgroundAlpha));
  loopInEditor.setColour (juce::TextEditor::textColourId, Config::playbackTextColor);
  loopInEditor.setFont (juce::Font (juce::FontOptions (Config::playbackTextSize)));
  loopInEditor.setMultiLine (false);
  loopInEditor.setReturnKeyStartsNewLine (false);
  loopInEditor.addListener (this);
  loopInEditor.setWantsKeyboardFocus (true); // Allow to take focus for editing

  addAndMakeVisible (loopOutEditor);
  loopOutEditor.setReadOnly (false);
  loopOutEditor.setJustification(juce::Justification::centred);
  loopOutEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colours::grey.withAlpha(Config::playbackTextBackgroundAlpha));
  loopOutEditor.setColour (juce::TextEditor::textColourId, Config::playbackTextColor);
  loopOutEditor.setFont (juce::Font (juce::FontOptions (Config::playbackTextSize)));
  loopOutEditor.setMultiLine (false);
  loopOutEditor.setReturnKeyStartsNewLine (false);
  loopOutEditor.addListener (this);
  loopOutEditor.setWantsKeyboardFocus (true); // Allow to take focus for editing

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
    loopOutPosition = thumbnail.getTotalLength();
    ensureLoopOrder();
    updateLoopButtonColors();
    updateLoopLabels();
    repaint();
  };


  addAndMakeVisible (detectInSilenceButton);
  detectInSilenceButton.setButtonText (Config::detectInButtonText);
  detectInSilenceButton.onClick = [this] {
    DBG("Button Clicked: Detect In Silence");
    detectInSilence(); // Call detectInSilence
  };

  addAndMakeVisible (inSilenceThresholdLabel);
  inSilenceThresholdLabel.setText ("In Threshold:", juce::dontSendNotification);
  inSilenceThresholdLabel.setJustificationType (juce::Justification::right);

  addAndMakeVisible (inSilenceThresholdEditor);
  inSilenceThresholdEditor.setText (juce::String (static_cast<int>(Config::silenceThreshold * 100.0f)));
  inSilenceThresholdEditor.setInputRestrictions (0, "0123456789");
  inSilenceThresholdEditor.setJustification(juce::Justification::centred);
  inSilenceThresholdEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colours::grey.withAlpha(Config::playbackTextBackgroundAlpha));
  inSilenceThresholdEditor.setColour (juce::TextEditor::textColourId, Config::playbackTextColor);
  inSilenceThresholdEditor.setFont (juce::Font (juce::FontOptions (Config::playbackTextSize)));
  inSilenceThresholdEditor.applyFontToAllText(inSilenceThresholdEditor.getFont());
  inSilenceThresholdEditor.setMultiLine (false);
  inSilenceThresholdEditor.setReturnKeyStartsNewLine (false);
  inSilenceThresholdEditor.addListener (this);
  inSilenceThresholdEditor.setWantsKeyboardFocus (true);


  addAndMakeVisible (detectOutSilenceButton);
  detectOutSilenceButton.setButtonText (Config::detectOutButtonText);
  detectOutSilenceButton.onClick = [this] {
    DBG("Button Clicked: Detect Out Silence");
    detectOutSilence(); // Call detectOutSilence
  };

  addAndMakeVisible (outSilenceThresholdLabel);
  outSilenceThresholdLabel.setText ("Out Threshold:", juce::dontSendNotification);
  outSilenceThresholdLabel.setJustificationType (juce::Justification::right);

  addAndMakeVisible (outSilenceThresholdEditor);
  outSilenceThresholdEditor.setText (juce::String (static_cast<int>(Config::outSilenceThreshold * 100.0f)));
  outSilenceThresholdEditor.setInputRestrictions (0, "0123456789");
  outSilenceThresholdEditor.setJustification(juce::Justification::centred);
  outSilenceThresholdEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colours::grey.withAlpha(Config::playbackTextBackgroundAlpha));
  outSilenceThresholdEditor.setColour (juce::TextEditor::textColourId, Config::playbackTextColor);
  outSilenceThresholdEditor.setFont (juce::Font (juce::FontOptions (Config::playbackTextSize)));
  outSilenceThresholdEditor.applyFontToAllText(outSilenceThresholdEditor.getFont());
  outSilenceThresholdEditor.setMultiLine (false);
  outSilenceThresholdEditor.setReturnKeyStartsNewLine (false);
  outSilenceThresholdEditor.addListener (this);
  outSilenceThresholdEditor.setWantsKeyboardFocus (true);


  updateLoopLabels();
  setSize(Config::initialWindowWidth, Config::initialWindowHeight);

  setAudioChannels (0, 2);
  startTimerHz (60);
  setWantsKeyboardFocus (true);
  grabKeyboardFocus(); // Request keyboard focus for MainComponent

  updateComponentStates(); // Call to set initial button states
}

void MainComponent::updateQualityButtonText() {
  if (currentQuality == MainComponent::ThumbnailQuality::High) qualityButton.setButtonText("[Q]ual H");
  else if (currentQuality == MainComponent::ThumbnailQuality::Medium) qualityButton.setButtonText("[Q]ual M");
  else qualityButton.setButtonText("[Q]ual L"); }

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
  shutdownAudio();
  thumbnail.removeChangeListener(this);
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
  auto audioLength = thumbnail.getTotalLength();
  if (audioLength <= 0.0)
  return;
  auto width = waveformBounds.getWidth();
  auto height = waveformBounds.getHeight();
  auto centerY = waveformBounds.getCentreY();
  for (int x = 0; x < width; x += pixelsPerSample) {
    auto proportion = (double)x / (double)width;
    auto time = proportion * audioLength;
    float minVal, maxVal;
    thumbnail.getApproximateMinMax(time, time + (audioLength / width) * pixelsPerSample, channel, minVal, maxVal);
    auto topY = centerY - (maxVal * height * 0.5f);
    auto bottomY = centerY - (minVal * height * 0.5f);
    g.drawVerticalLine(waveformBounds.getX() + x, topY, bottomY); }}

void MainComponent::openButtonClicked() {
  DBG("Button Clicked: Open");
  isFileLoaded = false; // Assume no file loaded initially
  updateComponentStates(); // Disable buttons until a file is successfully loaded

  auto filter = formatManager.getWildcardForAllFormats();
  chooser = std::make_unique<juce::FileChooser> ("Select Audio...",
    juce::File::getSpecialLocation (juce::File::userHomeDirectory), filter);
  auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
  chooser->launchAsync (flags, [this] (const juce::FileChooser& fc) {
    auto file = fc.getResult();
    if (file.exists()) {
      auto* reader = formatManager.createReaderFor (file);
      if (reader != nullptr) {
        transportSource.stop();
        transportSource.setSource (nullptr);
        auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);
        transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);
        thumbnail.setSource (new juce::FileInputSource (file));
        DBG("AudioThumbnail: Num Channels = " << thumbnail.getNumChannels() << ", Total Length = " << thumbnail.getTotalLength());
                totalTimeStaticStr = formatTime(thumbnail.getTotalLength()); // Set total time static string
                isFileLoaded = true; // Set flag to true after successful load
                loopInPosition  = 0.0;
                loopOutPosition = thumbnail.getTotalLength();
                readerSource.reset (newSource.release());
                updateButtonText();
                updateComponentStates(); // Update component states after loading file
                grabKeyboardFocus(); // Re-grab focus after file chooser closes
                if (shouldAutoplay && isFileLoaded) {
                    if (shouldAutoCutIn) {
                        detectInSilence();
                    }
                    if (shouldAutoCutOut) {
                        detectOutSilence();
                    }
                    playStopButtonClicked(); // Simulate click to start playback
                }

              }}});}

void MainComponent::playStopButtonClicked() {
  DBG("Button Clicked: Play/Stop");
  if (transportSource.isPlaying()) {
    transportSource.stop(); }
  else {
    if (transportSource.hasStreamFinished()) {
      if (shouldLoop)
        transportSource.setPosition (0.0);
          else {
            updateButtonText();
              return; }}
    transportSource.start();}
  updateButtonText(); }

bool MainComponent::keyPressed (const juce::KeyPress& key) {
    auto keyCode = key.getTextCharacter();
    DBG("Key Pressed: " << key.getTextDescription());

    // Handle 'e' and 'd' keys regardless of file loaded status
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

    // All other actions require a file to be loaded
    if (isFileLoaded) {
        if (thumbnail.getTotalLength() > 0.0) {
            constexpr double skipAmountSeconds = 5.0;
            if (key == juce::KeyPress::leftKey) {
                auto newPos = juce::jmax (0.0, transportSource.getCurrentPosition() - skipAmountSeconds);
                transportSource.setPosition (newPos);
                DBG("  Left arrow key pressed. Seeking to " << newPos);
                return true;
            }
            if (key == juce::KeyPress::rightKey) {
                auto newPos = juce::jmin (thumbnail.getTotalLength(), transportSource.getCurrentPosition() + skipAmountSeconds);
                transportSource.setPosition (newPos);
                DBG("  Right arrow key pressed. Seeking to " << newPos);
                return true;
            }
        }

        if (key == juce::KeyPress::spaceKey) {
            DBG("  Space key pressed. Toggling play/stop.");
            playStopButtonClicked();
            return true;
        }
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
        if (keyCode == 'i' || keyCode == 'I') {
            loopInPosition = transportSource.getCurrentPosition();
            DBG("  'i' key pressed. Setting loop in position to " << loopInPosition);
            ensureLoopOrder(); // Call helper
            updateLoopButtonColors();
            updateLoopLabels();
            repaint();
            return true;
        }
        if (keyCode == 'o' || keyCode == 'O') {
            loopOutPosition = transportSource.getCurrentPosition();
            DBG("  'o' key pressed. Setting loop out position to " << loopOutPosition);
            ensureLoopOrder(); // Call helper
            updateLoopButtonColors();
            updateLoopLabels();
            repaint();
            return true;
        }
        if (keyCode == 'u' || keyCode == 'U') {
            DBG("  'u' key pressed. Clearing loop in.");
            clearLoopInButton.triggerClick();
            return true;
        }
        if (keyCode == 'p' || keyCode == 'P') {
            DBG("  'p' key pressed. Clearing loop out.");
            clearLoopOutButton.triggerClick();
            return true;
        }
    }

    // If none of the above conditions were met, return false
    return false;
}

void MainComponent::seekToPosition (int x) {
  if (thumbnail.getTotalLength() > 0.0) {
    auto relativeX = (double)(x - waveformBounds.getX());
    auto proportion = relativeX / (double)waveformBounds.getWidth();
    auto newPosition = juce::jlimit (0.0, 1.0, proportion) * thumbnail.getTotalLength();
    DBG("Seeking to position " << newPosition << " (x: " << x << ")");
    transportSource.setPosition (newPosition);
  }
}

void MainComponent::mouseUp (const juce::MouseEvent& e) {
  DBG("Mouse Up event at (" << e.x << ", " << e.y << ")");


  if (currentPlacementMode != PlacementMode::None && waveformBounds.contains (e.getPosition())) {
    auto relativeX = (double)(e.x - waveformBounds.getX());
    auto proportion = relativeX / (double)waveformBounds.getWidth();
    auto newPosition = juce::jlimit (0.0, 1.0, proportion) * thumbnail.getTotalLength();
    if (currentPlacementMode == PlacementMode::LoopIn) {
      loopInPosition = newPosition;
      DBG("  Setting Loop In position via mouse: " << loopInPosition);
      ensureLoopOrder(); // Call helper
    } else if (currentPlacementMode == PlacementMode::LoopOut) {
      loopOutPosition = newPosition;
      DBG("  Setting Loop Out position via mouse: " << loopOutPosition);
      ensureLoopOrder(); // Call helper
    }
    currentPlacementMode = PlacementMode::None;
    updateLoopButtonColors();
    repaint();
  }
}

void MainComponent::mouseDown (const juce::MouseEvent& e) {
  DBG("Mouse Down event at (" << e.x << ", " << e.y << ")");
  if (e.mods.isRightButtonDown()) {
    DBG("  (right-click, ignoring)");
    return;
  }
  if (currentPlacementMode == PlacementMode::None && waveformBounds.contains (e.getMouseDownPosition())) {
    DBG("  Seeking to position " << e.x);
    seekToPosition (e.x);
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

void MainComponent::updateButtonText() {
  if (transportSource.isPlaying())
    playStopButton.setButtonText (juce::CharPointer_UTF8 ("\xe2\x8f\xb8"));
  else
    playStopButton.setButtonText (juce::CharPointer_UTF8 ("\xe2\x96\xb6")); }

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) {
  if (readerSource.get() == nullptr) bufferToFill.clearActiveBufferRegion();
  else transportSource.getNextAudioBlock (bufferToFill); }

void MainComponent::timerCallback() {
  glowAlpha = 0.5f + 0.5f * std::sin(juce::Time::getMillisecondCounter() * Config::pulseSpeedFactor);

  if (!transportSource.isPlaying() && transportSource.hasStreamFinished() && shouldLoop) {
    transportSource.setPosition (0.0);
    transportSource.start(); }
  if (shouldLoop && loopOutPosition > loopInPosition && transportSource.getCurrentPosition() >= loopOutPosition) {
    transportSource.setPosition (loopInPosition); }
  if (!transportSource.isPlaying() && playStopButton.getButtonText().contains(juce::CharPointer_UTF8 ("\xe2\x8f\xb8")))
    updateButtonText();
  if (showStats) {
    juce::String debugInfo;
    debugInfo << "Samples Loaded: " << thumbnail.getNumSamplesFinished() << "\n";
    debugInfo << "Approx Peak: " << thumbnail.getApproximatePeak() << "\n";
    float minV, maxV;
    thumbnail.getApproximateMinMax(0.0, thumbnail.getTotalLength(), 0, minV, maxV);
    debugInfo << "Min/Max: " << minV << " / " << maxV;
    statsDisplay.setText (debugInfo, false); }
  if (transportSource.isPlaying() || showStats || thumbnail.getTotalLength() > 0.0 || shouldAutoCutIn || shouldAutoCutOut)
    repaint(); }

void MainComponent::paint (juce::Graphics& g) {
  g.fillAll (Config::mainBackgroundColor);
  if (thumbnail.getNumChannels() > 0) {
      int pixelsPerSample = 1;
      if (currentQuality == ThumbnailQuality::Low)
        pixelsPerSample = 4;
      else if (currentQuality == ThumbnailQuality::Medium)
        pixelsPerSample = 2;
      if (currentChannelViewMode == ChannelViewMode::Mono || thumbnail.getNumChannels() == 1) {
        g.setColour (Config::waveformColor);

        if (pixelsPerSample > 1) {
          drawReducedQualityWaveform(g, 0, pixelsPerSample); }
        else {
          thumbnail.drawChannel (g, waveformBounds, 0.0, thumbnail.getTotalLength(), 0, 1.0f); }}
  else {
    g.setColour (Config::waveformColor);
    if (pixelsPerSample > 1) {
    for (int ch = 0; ch < thumbnail.getNumChannels(); ++ch)
      drawReducedQualityWaveform(g, ch, pixelsPerSample); }
    else {
      thumbnail.drawChannels (g, waveformBounds, 0.0, thumbnail.getTotalLength(), 1.0f); }}
  auto audioLength = (float)thumbnail.getTotalLength();
  if (audioLength > 0.0) {
    if (isCutModeActive) { // Only draw threshold visualization if Cut mode is active
      // --- NEW CODE: Silence Threshold Visualization ---
      // Helper lambda to draw threshold lines at a given position
      auto drawThresholdVisualisation = [&](juce::Graphics& g_ref, double loopPos, float threshold, bool isActive) {
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


          juce::Colour lineColor = isActive ? Config::thresholdLineColor : Config::thresholdLineColor.withMultipliedAlpha(Config::thresholdLineInactiveDimFactor);
          juce::Colour regionColor = isActive ? Config::thresholdRegionColor : Config::thresholdRegionColor.withMultipliedAlpha(Config::thresholdRegionInactiveDimFactor);

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
      // --- END NEW CODE ---
    }

      // Loop points are initialized on file load or updated by user interaction, not reset on every paint.
      if (audioLength > 0.0) {
        auto actualIn = juce::jmin(loopInPosition, loopOutPosition);
        auto actualOut = juce::jmax(loopInPosition, loopOutPosition);
        auto inX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (actualIn / audioLength);
        auto outX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (actualOut / audioLength);
        g.setColour(shouldLoop ? Config::loopRegionColor : Config::loopRegionColor.withMultipliedAlpha(Config::loopRegionInactiveDimFactor));
        g.fillRect(juce::Rectangle<float>(inX, (float)waveformBounds.getY(), outX - inX, (float)waveformBounds.getHeight())); }
      if (audioLength > 0.0) {
        auto inX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (loopInPosition / audioLength);
        auto outX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (loopOutPosition / audioLength);

        // Draw base color for vertical lines (always visible)
        juce::Colour baseLineColor = Config::loopLineColor.withMultipliedAlpha(Config::loopLineInactiveDimFactor);
        g.setColour(baseLineColor);
        g.drawVerticalLine((int)inX, (float)waveformBounds.getY(), (float)waveformBounds.getBottom());
        g.drawVerticalLine((int)outX, (float)waveformBounds.getY(), (float)waveformBounds.getBottom());

        // Draw pulsing glow for vertical lines (if looping is active)
        if (shouldLoop) {
            juce::Colour glowColor = Config::loopLineColor.withAlpha(Config::loopLineColor.getFloatAlpha() * (1.0f - glowAlpha));
            g.setColour(glowColor);
            // Draw a thicker rectangle for the glow, centered on the line
            g.fillRect(inX - (Config::loopLineGlowThickness / 2.0f - 0.5f), (float)waveformBounds.getY(), Config::loopLineGlowThickness, (float)waveformBounds.getHeight());
            g.fillRect(outX - (Config::loopLineGlowThickness / 2.0f - 0.5f), (float)waveformBounds.getY(), Config::loopLineGlowThickness, (float)waveformBounds.getHeight());
        }
      }
      auto drawPosition = (float)transportSource.getCurrentPosition();
      auto x = (drawPosition / audioLength) * (float)waveformBounds.getWidth() + (float)waveformBounds.getX();
      if (transportSource.isPlaying()) {
        juce::ColourGradient gradient (
        Config::playbackCursorGlowColorStart,
        (float)x - 10.0f, (float)waveformBounds.getCentreY(),
        Config::playbackCursorGlowColorEnd,
        (float)x, (float)waveformBounds.getCentreY(),
        false );
          g.setGradientFill (gradient);
          g.fillRect (juce::Rectangle<float>((int)x - 10, (float)waveformBounds.getY(), 10, (float)waveformBounds.getHeight())); }
      else {
        juce::ColourGradient glowGradient;
        glowGradient.addColour (0.0, Config::playbackCursorGlowColorStart);
        glowGradient.addColour (0.5, Config::playbackCursorGlowColorEnd);
        glowGradient.addColour (1.0, juce::Colours::lime.withAlpha(0.0f));
        glowGradient.point1 = { (float)x - 5.0f, (float)waveformBounds.getCentreY() };
        glowGradient.point2 = { (float)x + 5.0f, (float)waveformBounds.getCentreY() };
        g.setGradientFill (glowGradient);
        g.fillRect (juce::Rectangle<float>((int)x - 5, (float)waveformBounds.getY(), 10, (float)waveformBounds.getHeight())); }
      g.setColour (Config::playbackCursorColor);
      g.drawVerticalLine ((int)x, (float)waveformBounds.getY(), (float)waveformBounds.getBottom()); }
  if (mouseCursorX != -1) {
    g.setColour (Config::mouseCursorHighlightColor);
    g.fillRect (mouseCursorX - 2, waveformBounds.getY(), 5, waveformBounds.getHeight());
    g.fillRect (waveformBounds.getX(), mouseCursorY - 2, waveformBounds.getWidth(), 5);
    g.setColour (Config::mouseCursorLineColor);
    g.drawVerticalLine (mouseCursorX, (float)waveformBounds.getY(), (float)waveformBounds.getBottom());
    g.drawHorizontalLine (mouseCursorY, (float)waveformBounds.getX(), (float)waveformBounds.getRight()); }
  if (audioLength > 0.0) {
    // Loop times are now handled by TextEditors, no direct drawing here.

    double currentTime = transportSource.getCurrentPosition();
    double totalTime = thumbnail.getTotalLength(); // totalTime is calculated but totalTimeStaticStr is used for display
    double remainingTime = totalTime - currentTime;

    juce::String currentTimeStr = formatTime(currentTime);
    juce::String remainingTimeStr = formatTime(remainingTime);

    int textY = bottomRowTopY - 25;

    g.setColour(Config::playbackTextColor);
    g.setFont(Config::playbackTextSize);

    // Draw Current Time (Left)
    g.drawText(currentTimeStr, playbackLeftTextX, textY, Config::playbackTextWidth, Config::playbackTextHeight, juce::Justification::left, false);

    // Draw Total Time (Center)
    g.drawText(totalTimeStaticStr, playbackCenterTextX, textY, Config::playbackTextWidth, 20, juce::Justification::centred, false);

    // Draw Remaining Time (Right)
    g.drawText(remainingTimeStr, playbackRightTextX, textY, Config::playbackTextWidth, 20, juce::Justification::right, false); }}}

// juce::TextEditor::Listener callbacks
void MainComponent::textEditorTextChanged (juce::TextEditor& editor) {
    DBG("Text Editor Text Changed.");
    if (&editor == &inSilenceThresholdEditor) {
        DBG("  In Silence Threshold Editor: " << editor.getText());
        int newPercentage = editor.getText().getIntValue();
        if (newPercentage >= 1 && newPercentage <= 99) {
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Valid
        } else {
            editor.setColour(juce::TextEditor::textColourId, juce::Colours::orange); // Out of range
        }
    } else if (&editor == &outSilenceThresholdEditor) {
        DBG("  Out Silence Threshold Editor: " << editor.getText());
        int newPercentage = editor.getText().getIntValue();
        if (newPercentage >= 1 && newPercentage <= 99) {
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Valid
        } else {
            editor.setColour(juce::TextEditor::textColourId, juce::Colours::orange); // Out of range
        }
    } else {
        DBG("  Loop Editor Text Changed: " << editor.getText());
        double totalLength = thumbnail.getTotalLength();
        double newPosition = parseTime(editor.getText());

        if (newPosition >= 0.0 && newPosition <= totalLength) {
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Valid and in range
        } else if (newPosition == -1.0) {
            editor.setColour(juce::TextEditor::textColourId, juce::Colours::red); // Completely invalid format
        } else {
            editor.setColour(juce::TextEditor::textColourId, juce::Colours::orange); // Valid format but out of range
        }
    }
}

void MainComponent::textEditorReturnKeyPressed (juce::TextEditor& editor) {
    DBG("Text Editor Return Key Pressed.");
    if (&editor == &loopInEditor) {
        DBG("  Loop In Editor: Return Key Pressed");
        double newPosition = parseTime(editor.getText());
        if (newPosition >= 0.0 && newPosition <= thumbnail.getTotalLength()) {
            // Validate against loopOutPosition if it's set
            if (loopOutPosition > -1.0 && newPosition > loopOutPosition) {
                // Invalid: new loopIn is after loopOut, revert to current loopIn
                editor.setText(formatTime(loopInPosition), juce::dontSendNotification);
                editor.setColour(juce::TextEditor::textColourId, juce::Colours::orange); // Indicate warning
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
            editor.setColour(juce::TextEditor::textColourId, juce::Colours::red); // Indicate error
        }
    } else if (&editor == &loopOutEditor) {
        DBG("  Loop Out Editor: Return Key Pressed");
        double newPosition = parseTime(editor.getText());
        if (newPosition >= 0.0 && newPosition <= thumbnail.getTotalLength()) {
            if (shouldLoop && transportSource.getCurrentPosition() >= loopOutPosition)
            {
                transportSource.setPosition(loopInPosition);
            }
            // Validate against loopInPosition if it's set
            if (loopInPosition > -1.0 && newPosition < loopInPosition) { // Uncommented and properly nested
                // Invalid: new loopOut is before loopIn, revert to current loopOut
                editor.setText(formatTime(loopOutPosition), juce::dontSendNotification);
                editor.setColour(juce::TextEditor::textColourId, juce::Colours::orange); // Indicate warning
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
            editor.setColour(juce::TextEditor::textColourId, juce::Colours::red); // Indicate error
        }
    } else if (&editor == &inSilenceThresholdEditor)
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
        double newPosition = parseTime(editor.getText());
        if (newPosition >= 0.0 && newPosition <= thumbnail.getTotalLength()) {
            // Validate against loopOutPosition if it's set
            if (loopOutPosition > -1.0 && newPosition > loopOutPosition) {
                // Invalid: new loopIn is after loopOut, revert to current loopIn
                editor.setText(formatTime(loopInPosition), juce::dontSendNotification);
                editor.setColour(juce::TextEditor::textColourId, juce::Colours::orange); // Indicate warning
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
            editor.setColour(juce::TextEditor::textColourId, juce::Colours::red); // Indicate error
            repaint();
        }
    } else if (&editor == &loopOutEditor) {
        DBG("  Loop Out Editor: Focus Lost");
        double newPosition = parseTime(editor.getText());
        if (newPosition >= 0.0 && newPosition <= thumbnail.getTotalLength()) {
            // Validate against loopInPosition if it's set
            if (loopInPosition > -1.0 && newPosition < loopInPosition) {
                // Invalid: new loopOut is before loopIn, revert to current loopOut
                editor.setText(formatTime(loopOutPosition), juce::dontSendNotification);
                editor.setColour(juce::TextEditor::textColourId, juce::Colours::orange); // Indicate warning
            } else {
                loopOutPosition = newPosition;
                DBG("    Loop Out position set to: " << loopOutPosition);
                updateLoopButtonColors();
                editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
                repaint();
            }
        } else { // This else belongs to `if (newPosition >= 0.0 && newPosition <= thumbnail.getTotalLength())`
            // Revert to last valid position if input is invalid (out of bounds)
            editor.setText(formatTime(loopOutPosition), juce::dontSendNotification);
            editor.setColour(juce::TextEditor::textColourId, juce::Colours::red); // Indicate error
        }
    } else if (&editor == &inSilenceThresholdEditor) {
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
    if (!readerSource) {
        DBG("  readerSource is null. Returning.");
        return;
    }

    bool wasPlaying = transportSource.isPlaying();
    if (wasPlaying) {
        transportSource.stop();
        DBG("  Playback stopped for silence detection.");
    }

    auto* reader = readerSource->getAudioFormatReader();
    if (!reader) {
        DBG("  AudioFormatReader is null. Returning.");
        if (wasPlaying) { // If we stopped playback, start it again before returning
            transportSource.start();
        }
        return;
    }
    
    int numChannels = reader->numChannels;
    juce::int64 lengthInSamples = reader->lengthInSamples;
    DBG("  Reader: numChannels = " << numChannels << ", lengthInSamples = " << lengthInSamples);

    if (lengthInSamples <= 0) {
        DBG("  Audio length in samples is non-positive. Returning.");
        if (wasPlaying) { // If we stopped playback, start it again before returning
            transportSource.start();
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
        transportSource.setPosition(loopInPosition);
        DBG("  Transport source position set to: " << loopInPosition);

        updateLoopButtonColors();
        updateLoopLabels();
        repaint();
    } else {
        DBG("  No non-silent samples found for loop in. loopInPosition unchanged.");
    }

    if (wasPlaying) {
        transportSource.start();
        DBG("  Playback restarted.");
    }
    DBG("detectInSilence() finished.");
}

void MainComponent::detectOutSilence()
{
    DBG("detectOutSilence() called.");
    if (!readerSource) {
        DBG("  readerSource is null. Returning.");
        return;
    }

    bool wasPlaying = transportSource.isPlaying();
    if (wasPlaying) {
        transportSource.stop();
        DBG("  Playback stopped for silence detection.");
    }

    auto* reader = readerSource->getAudioFormatReader();
    if (!reader) {
        DBG("  AudioFormatReader is null. Returning.");
        if (wasPlaying) {
            transportSource.start();
        }
        return;
    }
    
    int numChannels = reader->numChannels;
    juce::int64 lengthInSamples = reader->lengthInSamples;
    DBG("  Reader: numChannels = " << numChannels << ", lengthInSamples = " << lengthInSamples);

    if (lengthInSamples <= 0) {
        DBG("  Audio length in samples is non-positive. Returning.");
        if (wasPlaying) {
            transportSource.start();
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

        transportSource.setPosition(loopOutPosition);
        DBG("  Transport source position set to: " << loopOutPosition);

        updateLoopButtonColors();
        updateLoopLabels();
        repaint();
    } else {
        DBG("  No non-silent samples found for loop out. loopOutPosition unchanged.");
    }

    if (wasPlaying) {
        transportSource.start();
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

  int rowHeight = Config::buttonHeight + Config::windowBorderMargins * 2; // Make row height configurable based on button height
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

  auto loopRow = bounds.removeFromTop(rowHeight).reduced(Config::windowBorderMargins);
  loopInButton.setBounds(loopRow.removeFromLeft(Config::buttonWidth)); loopRow.removeFromLeft(Config::windowBorderMargins);

  // Position loopInEditor
  loopInTextX = loopRow.getX(); // Left edge of the space for loopIn
  loopTextY = loopRow.getY() + (loopRow.getHeight() / 2) - 10; // Vertically center 20px high text
  loopInEditor.setBounds(loopInTextX, loopTextY, Config::loopTextWidth, Config::playbackTextHeight); // Set bounds for loopInEditor
  loopRow.removeFromLeft(Config::loopTextWidth); // Space for loopInEditor
  loopRow.removeFromLeft(Config::windowBorderMargins / 2);

  clearLoopInButton.setBounds(loopRow.getX(), loopTextY, Config::clearButtonWidth, Config::playbackTextHeight);
  loopRow.removeFromLeft(25);

  loopRow.removeFromLeft(Config::windowBorderMargins * 2); // Doubled distance

  loopOutButton.setBounds(loopRow.removeFromLeft(Config::buttonWidth)); loopRow.removeFromLeft(Config::windowBorderMargins);

  // Position loopOutEditor
  loopOutTextX = loopRow.getX(); // Left edge of the space for loopOut
  // loopTextY is already set once, assuming numbers are on the same line
  loopOutEditor.setBounds(loopOutTextX, loopTextY, Config::loopTextWidth, Config::playbackTextHeight); // Set bounds for loopOutEditor
  loopRow.removeFromLeft(Config::loopTextWidth); // Space for loopOutEditor
  loopRow.removeFromLeft(Config::windowBorderMargins / 2);

  clearLoopOutButton.setBounds(loopRow.getX(), loopTextY, Config::clearButtonWidth, Config::playbackTextHeight);
  loopRow.removeFromLeft(25);

  loopRow.removeFromLeft(Config::windowBorderMargins * 2); 

  detectInSilenceButton.setBounds(loopRow.removeFromLeft(Config::buttonWidth)); 
  loopRow.removeFromLeft(Config::windowBorderMargins);

  inSilenceThresholdLabel.setBounds(loopRow.removeFromLeft(80));
  loopRow.removeFromLeft(Config::windowBorderMargins / 2);

  inSilenceThresholdEditor.setBounds(loopRow.getX(), loopTextY, 80, Config::playbackTextHeight);
  loopRow.removeFromLeft(80);

  loopRow.removeFromLeft(Config::windowBorderMargins * 2); 

  detectOutSilenceButton.setBounds(loopRow.removeFromLeft(Config::buttonWidth)); 
  loopRow.removeFromLeft(Config::windowBorderMargins);

  outSilenceThresholdLabel.setBounds(loopRow.removeFromLeft(80));
  loopRow.removeFromLeft(Config::windowBorderMargins / 2);

  outSilenceThresholdEditor.setBounds(loopRow.getX(), loopTextY, 80, Config::playbackTextHeight);
  loopRow.removeFromLeft(80);

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

  if (currentMode == ViewMode::Overlay) {waveformBounds = getLocalBounds(); }
  else {waveformBounds = bounds.reduced(Config::windowBorderMargins);}

  if (showStats) {
    statsBounds = contentAreaBounds.withHeight(100).reduced(10); // Use contentAreaBounds
    statsDisplay.setBounds(statsBounds);
    statsDisplay.setVisible(true);
    statsDisplay.toFront(true); }
  else {statsDisplay.setVisible(false); }

  openButton.setVisible(true);
  playStopButton.setVisible(true);
  modeButton.setVisible(true);
  statsButton.setVisible(true);
  loopButton.setVisible(true);
  loopInButton.setVisible(true);
  loopOutButton.setVisible(true);
  clearLoopInButton.setVisible(true);
  clearLoopOutButton.setVisible(true);
  channelViewButton.setVisible(true);
  qualityButton.setVisible(true);
  exitButton.setVisible(true);

  detectInSilenceButton.setVisible(true);
  inSilenceThresholdLabel.setVisible(true);
  inSilenceThresholdEditor.setVisible(true);

  detectOutSilenceButton.setVisible(true);
  outSilenceThresholdLabel.setVisible(true);
  outSilenceThresholdEditor.setVisible(true);}

juce::FlexBox MainComponent::getTopRowFlexBox(){
  juce::FlexBox row;
  row.flexDirection = juce::FlexBox::Direction::row;
  row.justifyContent = juce::FlexBox::JustifyContent::flexStart;
  row.alignItems = juce::FlexBox::AlignItems::center;

  auto addBtn = [&](juce::Button& btn, float width = 80.0f) {
    juce::FlexItem item(btn);
    item.flexBasis = width;
    item.flexGrow = 0.0f;
    item.flexShrink = 0.0f;
    item.margin = juce::FlexItem::Margin(0, 5, 0, 0);
    row.items.add(item); };

  addBtn(openButton);
  addBtn(playStopButton);
  addBtn(modeButton);
  addBtn(statsButton);
  addBtn(exitButton);
  return row; }

juce::FlexBox MainComponent::getLoopRowFlexBox() {
  juce::FlexBox row;
  row.flexDirection = juce::FlexBox::Direction::row;
  row.justifyContent = juce::FlexBox::JustifyContent::flexStart;
  row.alignItems = juce::FlexBox::AlignItems::center;

  auto addItem = [&](juce::Component& comp, float width) {
    juce::FlexItem item(comp);
    item.flexBasis = width;
    item.flexGrow = 0.0f;
    item.flexShrink = 0.0f;
    item.margin = juce::FlexItem::Margin(0, 5, 0, 0);
    row.items.add(item); };

  addItem(loopButton, Config::buttonWidth); // Use Config::buttonWidth
  addItem(loopInButton, Config::buttonWidth); // Use Config::buttonWidth
  addItem(loopInEditor, Config::loopTextWidth); // Use loopInEditor and Config::loopTextWidth
  addItem(loopOutButton, Config::buttonWidth); // Use Config::buttonWidth
  addItem(loopOutEditor, Config::loopTextWidth); // Use loopOutEditor and Config::loopTextWidth
  return row; }

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate) {
  transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate); }

void MainComponent::changeListenerCallback (juce::ChangeBroadcaster*) { repaint(); }

void MainComponent::releaseResources() { transportSource.releaseResources(); }

void MainComponent::updateComponentStates() {
  const bool enabled = isFileLoaded; // Base enable state depends on file loaded
  const bool cutControlsActive = isCutModeActive && enabled; // Cut controls enabled only if Cut mode is active AND file is loaded

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

  // Controls related to "Cut" mode: their enabled and visible state depends on isCutModeActive
  loopInButton.setEnabled(cutControlsActive);
  loopOutButton.setEnabled(cutControlsActive);
  loopInEditor.setEnabled(cutControlsActive);
  loopOutEditor.setEnabled(cutControlsActive);
  clearLoopInButton.setEnabled(cutControlsActive);
  clearLoopOutButton.setEnabled(cutControlsActive);

  detectInSilenceButton.setEnabled(cutControlsActive);
  inSilenceThresholdEditor.setEnabled(cutControlsActive);
  inSilenceThresholdLabel.setEnabled(cutControlsActive);

  detectOutSilenceButton.setEnabled(cutControlsActive);
  outSilenceThresholdEditor.setEnabled(cutControlsActive);
  outSilenceThresholdLabel.setEnabled(cutControlsActive);

  autoCutInButton.setEnabled(cutControlsActive);
  autoCutOutButton.setEnabled(cutControlsActive);

  // Set visibility for "Cut" mode related controls
  loopInButton.setVisible(isCutModeActive);
  loopOutButton.setVisible(isCutModeActive);
  loopInEditor.setVisible(isCutModeActive);
  loopOutEditor.setVisible(isCutModeActive);
  clearLoopInButton.setVisible(isCutModeActive);
  clearLoopOutButton.setVisible(isCutModeActive);

  detectInSilenceButton.setVisible(isCutModeActive);
  inSilenceThresholdEditor.setVisible(isCutModeActive);
  inSilenceThresholdLabel.setVisible(isCutModeActive);

  detectOutSilenceButton.setVisible(isCutModeActive);
  outSilenceThresholdEditor.setVisible(isCutModeActive);
  outSilenceThresholdLabel.setVisible(isCutModeActive);

  autoCutInButton.setVisible(isCutModeActive);
  autoCutOutButton.setVisible(isCutModeActive);
}

juce::FlexBox MainComponent::getBottomRowFlexBox() {
  juce::FlexBox row;
  row.flexDirection = juce::FlexBox::Direction::row;
  row.justifyContent = juce::FlexBox::JustifyContent::flexEnd;
  row.alignItems = juce::FlexBox::AlignItems::center;

  auto addBtn = [&](juce::Button& btn, float width = 80.0f) {
    juce::FlexItem item(btn);
    item.flexBasis = width;
    item.flexGrow = 0.0f;
    item.flexShrink = 0.0f;
    item.margin = juce::FlexItem::Margin(0, 5, 0, 0);
    row.items.add(item); };

  addBtn(qualityButton);
  addBtn(channelViewButton);
  addBtn(statsButton);
  addBtn(modeButton);
  return row; }

void MainComponent::focusGained (juce::Component::FocusChangeType cause) {
  ignoreUnused (cause);
  grabKeyboardFocus();
}
