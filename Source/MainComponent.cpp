#include "MainComponent.h"
#include "ControlPanel.h"
#include "Config.h"
#include <cmath> // For std::abs

MainComponent::MainComponent()
{
    audioPlayer = std::make_unique<AudioPlayer>();
    audioPlayer->addChangeListener(this);
    
    controlPanel = std::make_unique<ControlPanel>(*this);
    addAndMakeVisible(controlPanel.get());

    setSize(Config::initialWindowWidth, Config::initialWindowHeight);
    setAudioChannels(0, 2);
    startTimerHz(60);
    setWantsKeyboardFocus(true);
    grabKeyboardFocus();

#if TESTING_MODE
    juce::File audioFile(TEST_FILE_PATH);
    if (audioFile.existsAsFile())
    {
        auto result = audioPlayer->loadFile(audioFile);
        if (result.wasOk())
        {
            controlPanel->setTotalTimeStaticString(formatTime(audioPlayer->getThumbnail().getTotalLength()));
            controlPanel->setLoopInPosition(0.0);
            controlPanel->setLoopOutPosition(audioPlayer->getThumbnail().getTotalLength());
            controlPanel->updateComponentStates();
            
            // Calculate and display dynamic statistics
            juce::String stats = buildStatsString();
            controlPanel->updateStatsDisplay(stats);
            // Further setup for testing can be done via controlPanel
        }
        else
        {
            controlPanel->setStatsDisplayText(result.getErrorMessage(), juce::Colours::red);
        }
    }
#endif
}

MainComponent::~MainComponent()
{
    audioPlayer->removeChangeListener(this);
    shutdownAudio();
    stopTimer();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    audioPlayer->prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    audioPlayer->getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    audioPlayer->releaseResources();
}

void MainComponent::paint(juce::Graphics& g)
{
    // The ControlPanel now handles all painting of controls and waveforms.
    // MainComponent's paint is now only for things drawn outside of ControlPanel.
    // We can leave this empty if ControlPanel covers the whole area.
}

void MainComponent::resized()
{
    controlPanel->setBounds(getLocalBounds());
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == audioPlayer.get())
    {
        controlPanel->updatePlayButtonText(audioPlayer->isPlaying());
        repaint(); // Repaint to update playback cursor etc.
    }
}

void MainComponent::timerCallback()
{
    // Why: Enforce playback within loop points if Cut Mode is active.
    // If playback position exceeds loopOutPosition:
    //   - If looping is also enabled, jump back to loopInPosition.
    //   - If looping is not enabled, stop playback.
    if (controlPanel->isCutModeActive() && controlPanel->getLoopOutPosition() > controlPanel->getLoopInPosition() && audioPlayer->getTransportSource().getCurrentPosition() >= controlPanel->getLoopOutPosition())
    {
        if (controlPanel->getShouldLoop())
        {
            audioPlayer->getTransportSource().setPosition(controlPanel->getLoopInPosition());
        }
        else
        {
            audioPlayer->getTransportSource().stop();
        }
    }
    // Original loop logic, now only active if Cut Mode is OFF
    else if (controlPanel->getShouldLoop() && controlPanel->getLoopOutPosition() > controlPanel->getLoopInPosition() && audioPlayer->getTransportSource().getCurrentPosition() >= controlPanel->getLoopOutPosition())
    {
        audioPlayer->getTransportSource().setPosition(controlPanel->getLoopInPosition());
    }
    
    // We repaint continuously for animations and playback cursor
    controlPanel->repaint();
}

/**
 * @brief Handles the click event for the open file button.
 *        Opens a file chooser dialog and loads the selected audio file.
 */
void MainComponent::openButtonClicked()
{
    chooser = std::make_unique<juce::FileChooser>("Select Audio...",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory),
        audioPlayer->getFormatManager().getWildcardForAllFormats());
    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file.exists())
        {
            auto result = audioPlayer->loadFile(file);
            if (result.wasOk())
            {
                controlPanel->setTotalTimeStaticString(formatTime(audioPlayer->getThumbnail().getTotalLength()));
                controlPanel->setLoopInPosition(0.0);
                controlPanel->setLoopOutPosition(audioPlayer->getThumbnail().getTotalLength());
                controlPanel->updateLoopLabels();
                controlPanel->updateComponentStates();

                // Calculate and display dynamic statistics
                juce::String stats = buildStatsString();
                controlPanel->updateStatsDisplay(stats);

                if (controlPanel->shouldAutoplay())
                   audioPlayer->togglePlayStop();
            }
            else
            {
                controlPanel->setStatsDisplayText(result.getErrorMessage(), juce::Colours::red);
            }
        }
        grabKeyboardFocus();
    });
}

/**
 * @brief Seeks the audio playback position based on an X-coordinate in the waveform.
 * @param x The X-coordinate in pixels relative to the MainComponent.
 */
void MainComponent::seekToPosition(int x)
{
    if (audioPlayer->getThumbnail().getTotalLength() > 0.0)
    {
        auto relativeX = (double)(x - controlPanel->getWaveformBounds().getX());
        auto proportion = relativeX / (double)controlPanel->getWaveformBounds().getWidth();
        auto newPosition = juce::jlimit(0.0, 1.0, proportion) * audioPlayer->getThumbnail().getTotalLength();
        audioPlayer->getTransportSource().setPosition(newPosition);
    }
}

bool MainComponent::keyPressed(const juce::KeyPress& key)
{
    if (handleGlobalKeybinds(key)) return true;
    if (audioPlayer->getThumbnail().getTotalLength() > 0.0)
    {
        if (handlePlaybackKeybinds(key)) return true;
        if (handleUIToggleKeybinds(key)) return true;
        if (handleLoopKeybinds(key)) return true;
    }
    return false;
}

bool MainComponent::handleGlobalKeybinds(const juce::KeyPress& key)
{
    if (key.getTextCharacter() == 'e' || key.getTextCharacter() == 'E') {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
        return true;
    }
    if (key.getTextCharacter() == 'd' || key.getTextCharacter() == 'D') {
        openButtonClicked();
        return true;
    }
    return false;
}

bool MainComponent::handlePlaybackKeybinds(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::spaceKey) {
        audioPlayer->togglePlayStop();
        return true;
    }
    // Arrow key logic...
    return false;
}

bool MainComponent::handleUIToggleKeybinds(const juce::KeyPress& key)
{
    auto k = key.getTextCharacter();
    if (k == 's' || k == 'S') { controlPanel->toggleStats(); return true; }
    if (k == 'v' || k == 'V') { controlPanel->triggerModeButton(); return true; }
    if (k == 'c' || k == 'C') { controlPanel->triggerChannelViewButton(); return true; }
    if (k == 'q' || k == 'Q') { controlPanel->triggerQualityButton(); return true; }
    if (k == 'l' || k == 'L') { controlPanel->triggerLoopButton(); return true; }
    return false;
}

bool MainComponent::handleLoopKeybinds(const juce::KeyPress& key)
{
    auto k = key.getTextCharacter();
    // Why: Prevent manual loop point setting via keybinds when auto-cut is active
    // or when in a waveform placement mode, to avoid conflicts and unexpected behavior.
    if (controlPanel->getPlacementMode() == AppEnums::PlacementMode::None) // Only allow keybinds if not in waveform placement mode
    {
        if (k == 'i' || k == 'I') {
            // Why: If auto-cut in is active, the 'i' keybind should be ignored
            // as the loop-in position is managed automatically.

            controlPanel->setLoopInPosition(audioPlayer->getTransportSource().getCurrentPosition());
            controlPanel->repaint();
            return true;
        }
        if (k == 'o' || k == 'O') {
            // Why: If auto-cut out is active, the 'o' keybind should be ignored
            // as the loop-out position is managed automatically.

            controlPanel->setLoopOutPosition(audioPlayer->getTransportSource().getCurrentPosition());
            controlPanel->repaint();
            return true;
        }
    }
    if (k == 'u' || k == 'U') { controlPanel->clearLoopIn(); return true; }
    if (k == 'p' || k == 'P') { controlPanel->clearLoopOut(); return true; }
    return false;
}

/**
 * @brief Formats a time in seconds into a human-readable string (HH:MM:SS:mmm).
 * @param seconds The time in seconds.
 * @return A formatted juce::String.
 */
juce::String MainComponent::formatTime(double seconds) {
  if (seconds < 0) seconds = 0;
  int hours = (int)(seconds / 3600.0);
  int minutes = ((int)(seconds / 60.0)) % 60;
  int secs = ((int)seconds) % 60;
  int milliseconds = (int)((seconds - (int)seconds) * 1000.0);
  return juce::String::formatted("%02d:%02d:%02d:%03d", hours, minutes, secs, milliseconds);
}

/**
 * @brief Builds a string containing various audio statistics.
 * @return A juce::String with formatted audio statistics.
 */
juce::String MainComponent::buildStatsString()
{
    juce::String stats;
    auto* thumbnail = &audioPlayer->getThumbnail();
    auto* reader = audioPlayer->getAudioFormatReader();

    if (thumbnail && thumbnail->getTotalLength() > 0.0 && reader)
    {
        stats << "File: " << audioPlayer->getLoadedFile().getFileName() << "\n";
        stats << "Samples Loaded: " << reader->lengthInSamples << "\n";
        stats << "Sample Rate: " << reader->sampleRate << " Hz\n";
        stats << "Channels: " << thumbnail->getNumChannels() << "\n";
        stats << "Length: " << formatTime(thumbnail->getTotalLength()) << "\n";

        float minVal = 0.0f, maxVal = 0.0f;
        thumbnail->getApproximateMinMax(0.0, thumbnail->getTotalLength(), 0, minVal, maxVal); // For channel 0
        stats << "Approx Peak (Ch 0): " << juce::jmax(std::abs(minVal), std::abs(maxVal)) << "\n";
        stats << "Min: " << minVal << ", Max: " << maxVal << "\n";

        // If stereo, get stats for channel 1 as well
        if (thumbnail->getNumChannels() > 1)
        {
            thumbnail->getApproximateMinMax(0.0, thumbnail->getTotalLength(), 1, minVal, maxVal); // For channel 1
            stats << "Approx Peak (Ch 1): " << juce::jmax(std::abs(minVal), std::abs(maxVal)) << "\n";
            stats << "Min: " << minVal << ", Max: " << maxVal << "\n";
        }
    }
    else
    {
        stats << "No file loaded or error reading audio.";
    }
    return stats;
}