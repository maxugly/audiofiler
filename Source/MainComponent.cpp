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
    currentMode = modeButton.getToggleState() ? ViewMode::Overlay : ViewMode::Classic;
    modeButton.setButtonText (currentMode == ViewMode::Classic ? "[V]iew01" : "[V]iew02");
    resized();
    repaint(); };

  addAndMakeVisible (channelViewButton);
  channelViewButton.setButtonText ("[C]han");
  channelViewButton.setClickingTogglesState (true);
  channelViewButton.onClick = [this] {
    currentChannelViewMode = channelViewButton.getToggleState() ? ChannelViewMode::Stereo : ChannelViewMode::Mono;
    channelViewButton.setButtonText (currentChannelViewMode == ChannelViewMode::Mono ? "[C]han 1" : "[C]han 2");
    repaint(); };

  addAndMakeVisible (qualityButton);
  qualityButton.setButtonText ("[Q]ual");
  qualityButton.onClick = [this] {
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
  exitButton.onClick = [] { juce::JUCEApplication::getInstance()->systemRequestedQuit(); };

  addAndMakeVisible (statsButton);
  statsButton.setButtonText ("[S]tats");
  statsButton.setClickingTogglesState (true);
  statsButton.onClick = [this] {
    showStats = statsButton.getToggleState();
    resized(); };

  addAndMakeVisible (loopButton);
  loopButton.setButtonText ("[L]oop");
  loopButton.setClickingTogglesState (true);
  loopButton.onClick = [this] { shouldLoop = loopButton.getToggleState(); };

  addAndMakeVisible (loopInButton);
  loopInButton.setButtonText ("[I]n");
  loopInButton.onLeftClick = [this] { 
    loopInPosition = transportSource.getCurrentPosition();
    updateLoopButtonColors();
    repaint(); };
  loopInButton.onRightClick = [this] {
    currentPlacementMode = PlacementMode::LoopIn;
    updateLoopButtonColors();
    repaint(); };

  addAndMakeVisible (loopOutButton);
  loopOutButton.setButtonText ("[O]ut");
  loopOutButton.onLeftClick = [this] {
    loopOutPosition = transportSource.getCurrentPosition();
    updateLoopButtonColors();
    repaint(); };
  loopOutButton.onRightClick = [this] {
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

  addAndMakeVisible (loopOutEditor);
  loopOutEditor.setReadOnly (false);
  loopOutEditor.setJustification(juce::Justification::centred);
  loopOutEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colours::grey.withAlpha(Config::playbackTextBackgroundAlpha));
  loopOutEditor.setColour (juce::TextEditor::textColourId, Config::playbackTextColor);
  loopOutEditor.setFont (juce::Font (juce::FontOptions (Config::playbackTextSize)));
  loopOutEditor.setMultiLine (false);
  loopOutEditor.setReturnKeyStartsNewLine (false);
  loopOutEditor.addListener (this);

  addAndMakeVisible (clearLoopInButton);
  clearLoopInButton.setButtonText ("X");
  clearLoopInButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
  clearLoopInButton.onClick = [this] {
      loopInPosition = -1.0;
      updateLoopButtonColors();
      updateLoopLabels();
      repaint();
  };

  addAndMakeVisible (clearLoopOutButton);
  clearLoopOutButton.setButtonText ("X");
  clearLoopOutButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
  clearLoopOutButton.onClick = [this] {
      loopOutPosition = -1.0;
      updateLoopButtonColors();
      updateLoopLabels();
      repaint();
  };


  updateLoopLabels();
  setSize(Config::initialWindowWidth, Config::initialWindowHeight);

  setAudioChannels (0, 2);
  startTimerHz (60);
  setWantsKeyboardFocus (true); }

void MainComponent::updateQualityButtonText() {
  if (currentQuality == MainComponent::ThumbnailQuality::High) qualityButton.setButtonText("[Q]ual H");
  else if (currentQuality == MainComponent::ThumbnailQuality::Medium) qualityButton.setButtonText("[Q]ual M");
  else qualityButton.setButtonText("[Q]ual L"); }

void MainComponent::updateLoopButtonColors() {
  if (currentPlacementMode == PlacementMode::LoopIn) {loopInButton.setColour(juce::TextButton::buttonColourId, Config::loopButtonPlacementModeColor); }
  else if (loopInPosition > -1.0) {loopInButton.setColour(juce::TextButton::buttonColourId, Config::loopButtonActiveColor); }
  else {loopInButton.setColour(juce::TextButton::buttonColourId, Config::buttonBaseColour); }
  if (currentPlacementMode == PlacementMode::LoopOut) {loopOutButton.setColour(juce::TextButton::buttonColourId, Config::loopButtonPlacementModeColor);}
  else if (loopOutPosition > -1.0) {loopOutButton.setColour(juce::TextButton::buttonColourId, Config::loopButtonActiveColor); }
  else {loopOutButton.setColour(juce::TextButton::buttonColourId, Config::buttonBaseColour); }
  updateLoopLabels();}

MainComponent::~MainComponent() {
  shutdownAudio();
  thumbnail.removeChangeListener(this);
  stopTimer();
  setLookAndFeel(nullptr); }

juce::String MainComponent::formatTime(double seconds) {
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
        playStopButton.setEnabled (true);
        readerSource.reset (newSource.release());
        updateButtonText(); }}});}

void MainComponent::playStopButtonClicked() {
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

    // Handle 'e' and 'd' keys regardless of file loaded status
    if (keyCode == 'e' || keyCode == 'E') {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
        return true;
    }
    if (keyCode == 'd' || keyCode == 'D') {
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
                return true;
            }
            if (key == juce::KeyPress::rightKey) {
                auto newPos = juce::jmin (thumbnail.getTotalLength(), transportSource.getCurrentPosition() + skipAmountSeconds);
                transportSource.setPosition (newPos);
                return true;
            }
        }

        if (key == juce::KeyPress::spaceKey) { playStopButtonClicked(); return true; }
        if (keyCode == 's' || keyCode == 'S') { statsButton.triggerClick(); return true; }
        if (keyCode == 'v' || keyCode == 'V') { modeButton.triggerClick(); return true; }
        if (keyCode == 'c' || keyCode == 'C') { channelViewButton.triggerClick(); return true; }
        if (keyCode == 'q' || keyCode == 'Q') { qualityButton.triggerClick(); return true; }
        if (keyCode == 'l' || keyCode == 'L') { loopButton.triggerClick(); return true; }
        if (keyCode == 'i' || keyCode == 'I') {
            loopInPosition = transportSource.getCurrentPosition();
            updateLoopButtonColors();
            updateLoopLabels();
            repaint();
            return true;
        }
        if (keyCode == 'o' || keyCode == 'O') {
            loopOutPosition = transportSource.getCurrentPosition();
            updateLoopButtonColors();
            updateLoopLabels();
            repaint();
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
    transportSource.setPosition (newPosition); }}

void MainComponent::mouseUp (const juce::MouseEvent& e) {
  if (currentPlacementMode != PlacementMode::None && waveformBounds.contains (e.getPosition())) {
    auto relativeX = (double)(e.x - waveformBounds.getX());
    auto proportion = relativeX / (double)waveformBounds.getWidth();
    auto newPosition = juce::jlimit (0.0, 1.0, proportion) * thumbnail.getTotalLength();
    if (currentPlacementMode == PlacementMode::LoopIn) {
      loopInPosition = newPosition;
      DBG("Loop In set by mouse click on waveform"); }
        else if (currentPlacementMode == PlacementMode::LoopOut) {
        loopOutPosition = newPosition;
        DBG("Loop Out set by mouse click on waveform"); }
          currentPlacementMode = PlacementMode::None;
          updateLoopButtonColors();
          repaint(); }}

void MainComponent::mouseDown (const juce::MouseEvent& e) {
  if (e.mods.isRightButtonDown()) return;
  if (currentPlacementMode == PlacementMode::None && waveformBounds.contains (e.getMouseDownPosition()))
        seekToPosition (e.x); }

void MainComponent::mouseDrag (const juce::MouseEvent& e) {
  if (e.mods.isRightButtonDown()) return;
  if (currentPlacementMode == PlacementMode::None && waveformBounds.contains (e.getPosition()))
    seekToPosition (e.x); }

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
  if (transportSource.isPlaying() || showStats || thumbnail.getTotalLength() > 0.0)
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
      bool inIsSet = loopInPosition > -1.0;
      bool outIsSet = loopOutPosition > -1.0;
      if (inIsSet && outIsSet) {
        auto actualIn = juce::jmin(loopInPosition, loopOutPosition);
        auto actualOut = juce::jmax(loopInPosition, loopOutPosition);
        auto inX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (actualIn / audioLength);
        auto outX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (actualOut / audioLength);
        g.setColour(Config::loopRegionColor);
        g.fillRect(juce::Rectangle<float>(inX, (float)waveformBounds.getY(), outX - inX, (float)waveformBounds.getHeight())); }
      g.setColour(Config::loopLineColor);
      if (inIsSet) {
        auto inX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (loopInPosition / audioLength);
        g.drawVerticalLine((int)inX, (float)waveformBounds.getY(), (float)waveformBounds.getBottom()); }
      if (outIsSet) {
        auto outX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (loopOutPosition / audioLength);
        g.drawVerticalLine((int)outX, (float)waveformBounds.getY(), (float)waveformBounds.getBottom()); }
      auto drawPosition = (float)transportSource.getCurrentPosition();
      auto x = (drawPosition / audioLength) * (float)waveformBounds.getWidth() + (float)waveformBounds.getX();
      if (transportSource.isPlaying()) {
        juce::ColourGradient gradient (
        juce::Colours::lime.withAlpha(0.0f),
        (float)x - 10.0f, (float)waveformBounds.getCentreY(),
        juce::Colours::lime.withAlpha(0.5f),
        (float)x, (float)waveformBounds.getCentreY(),
        false );
          g.setGradientFill (gradient);
          g.fillRect (juce::Rectangle<float>((int)x - 10, (float)waveformBounds.getY(), 10, (float)waveformBounds.getHeight())); }
      else {
        juce::ColourGradient glowGradient;
        glowGradient.addColour (0.0, juce::Colours::lime.withAlpha(0.0f));
        glowGradient.addColour (0.5, juce::Colours::lime.withAlpha(0.5f));
        glowGradient.addColour (1.0, juce::Colours::lime.withAlpha(0.0f));
        glowGradient.point1 = { (float)x - 5.0f, (float)waveformBounds.getCentreY() };
        glowGradient.point2 = { (float)x + 5.0f, (float)waveformBounds.getCentreY() };
        g.setGradientFill (glowGradient);
        g.fillRect (juce::Rectangle<float>((int)x - 5, (float)waveformBounds.getY(), 10, (float)waveformBounds.getHeight())); }
      g.setColour (juce::Colours::lime);
      g.drawVerticalLine ((int)x, (float)waveformBounds.getY(), (float)waveformBounds.getBottom()); }
  if (mouseCursorX != -1) {
    g.setColour (juce::Colours::darkorange.withAlpha(0.4f));
    g.fillRect (mouseCursorX - 2, waveformBounds.getY(), 5, waveformBounds.getHeight());
    g.fillRect (waveformBounds.getX(), mouseCursorY - 2, waveformBounds.getWidth(), 5);
    g.setColour (juce::Colours::yellow);
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
    g.drawText(currentTimeStr, playbackLeftTextX, textY, Config::playbackTextWidth, 20, juce::Justification::left, false);

    // Draw Total Time (Center)
    g.drawText(totalTimeStaticStr, playbackCenterTextX, textY, Config::playbackTextWidth, 20, juce::Justification::centred, false);

    // Draw Remaining Time (Right)
    g.drawText(remainingTimeStr, playbackRightTextX, textY, Config::playbackTextWidth, 20, juce::Justification::right, false); }}}

// juce::TextEditor::Listener callbacks
void MainComponent::textEditorTextChanged (juce::TextEditor& editor) {
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

void MainComponent::textEditorReturnKeyPressed (juce::TextEditor& editor) {
    if (&editor == &loopInEditor) {
        double newPosition = parseTime(editor.getText());
        if (newPosition >= 0.0 && newPosition <= thumbnail.getTotalLength()) {
            loopInPosition = newPosition;
            updateLoopButtonColors();
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
            repaint();
        } else {
            editor.setColour(juce::TextEditor::textColourId, juce::Colours::red); // Indicate error
            if (loopInPosition >= 0.0)
                editor.setText(formatTime(loopInPosition), juce::dontSendNotification);
            else
                editor.setText("--:--:--:---", juce::dontSendNotification);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
        }
    } else if (&editor == &loopOutEditor) {
        double newPosition = parseTime(editor.getText());
        if (newPosition >= 0.0 && newPosition <= thumbnail.getTotalLength()) {
            loopOutPosition = newPosition;
            updateLoopButtonColors();
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
            repaint();
        } else {
            editor.setColour(juce::TextEditor::textColourId, juce::Colours::red); // Indicate error
            if (loopOutPosition >= 0.0)
                editor.setText(formatTime(loopOutPosition), juce::dontSendNotification);
            else
                editor.setText("--:--:--:---", juce::dontSendNotification);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
        } // Closing brace for the 'else' part of loopOutEditor validation
    } // Closing brace for the 'else if (&editor == &loopOutEditor)' block
    editor.giveAwayKeyboardFocus(); // Lose focus after pressing enter
}

void MainComponent::textEditorEscapeKeyPressed (juce::TextEditor& editor) {
    // Revert text to current value on escape
    if (&editor == &loopInEditor) {
        if (loopInPosition >= 0.0)
            editor.setText(formatTime(loopInPosition), juce::dontSendNotification);
        else
            editor.setText("--:--:--:---", juce::dontSendNotification);
    } else if (&editor == &loopOutEditor) {
        if (loopOutPosition >= 0.0)
            editor.setText(formatTime(loopOutPosition), juce::dontSendNotification);
        else
            editor.setText("--:--:--:---", juce::dontSendNotification);
    }
    editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
    editor.giveAwayKeyboardFocus(); // Lose focus
}

void MainComponent::textEditorFocusLost (juce::TextEditor& editor) {
    // Similar logic to return key pressed, but always revert if invalid on focus lost
    if (&editor == &loopInEditor) {
        double newPosition = parseTime(editor.getText());
        if (newPosition >= 0.0 && newPosition <= thumbnail.getTotalLength()) {
            loopInPosition = newPosition;
            updateLoopButtonColors();
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
            repaint();
        } else {
            // Revert to last valid position if input is invalid on focus lost
            if (loopInPosition >= 0.0)
                editor.setText(formatTime(loopInPosition), juce::dontSendNotification);
            else
                editor.setText("--:--:--:---", juce::dontSendNotification);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
        }
    } else if (&editor == &loopOutEditor) {
        double newPosition = parseTime(editor.getText());
        if (newPosition >= 0.0 && newPosition <= thumbnail.getTotalLength()) {
            loopOutPosition = newPosition;
            updateLoopButtonColors();
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
            repaint();
        } else {
            // Revert to last valid position if input is invalid on focus lost
            if (loopOutPosition >= 0.0)
                editor.setText(formatTime(loopOutPosition), juce::dontSendNotification);
            else
                editor.setText("--:--:--:---", juce::dontSendNotification);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
        }
    }
}

void MainComponent::updateLoopLabels() {
  if (loopInPosition >= 0.0)
    loopInEditor.setText(formatTime(loopInPosition), juce::dontSendNotification);
  else
    loopInEditor.setText("--:--:--:---", juce::dontSendNotification);
  
  if (loopOutPosition >= 0.0)
    loopOutEditor.setText(formatTime(loopOutPosition), juce::dontSendNotification);
  else
    loopOutEditor.setText("--:--:--:---", juce::dontSendNotification);
}

void MainComponent::resized() {
  auto bounds = getLocalBounds();

  int rowHeight = Config::buttonHeight + Config::windowBorderMargins * 2; // Make row height configurable based on button height
  auto topRow = bounds.removeFromTop(rowHeight).reduced(Config::windowBorderMargins);
  openButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
  playStopButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
  modeButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
  statsButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
  loopButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins); // Moved here
  exitButton.setBounds(topRow.removeFromRight(Config::buttonWidth)); topRow.removeFromRight(Config::windowBorderMargins);

  auto loopRow = bounds.removeFromTop(rowHeight).reduced(Config::windowBorderMargins);
  loopInButton.setBounds(loopRow.removeFromLeft(Config::buttonWidth)); loopRow.removeFromLeft(Config::windowBorderMargins);

  // Position loopInEditor
  loopInTextX = loopRow.getX(); // Left edge of the space for loopIn
  loopTextY = loopRow.getY() + (loopRow.getHeight() / 2) - 10; // Vertically center 20px high text
  loopInEditor.setBounds(loopInTextX, loopTextY, Config::loopTextWidth, 20); // Set bounds for loopInEditor
  loopRow.removeFromLeft(Config::loopTextWidth); // Space for loopInEditor
  loopRow.removeFromLeft(Config::windowBorderMargins / 2);

  clearLoopInButton.setBounds(loopRow.getX(), loopTextY, 25, 20);
  loopRow.removeFromLeft(25);

  loopRow.removeFromLeft(Config::windowBorderMargins * 2); // Doubled distance

  loopOutButton.setBounds(loopRow.removeFromLeft(Config::buttonWidth)); loopRow.removeFromLeft(Config::windowBorderMargins);

  // Position loopOutEditor
  loopOutTextX = loopRow.getX(); // Left edge of the space for loopOut
  // loopTextY is already set once, assuming numbers are on the same line
  loopOutEditor.setBounds(loopOutTextX, loopTextY, Config::loopTextWidth, 20); // Set bounds for loopOutEditor
  loopRow.removeFromLeft(Config::loopTextWidth); // Space for loopOutEditor
  loopRow.removeFromLeft(Config::windowBorderMargins / 2);

  clearLoopOutButton.setBounds(loopRow.getX(), loopTextY, 25, 20);
  loopRow.removeFromLeft(25);

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
  exitButton.setVisible(true);}

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
