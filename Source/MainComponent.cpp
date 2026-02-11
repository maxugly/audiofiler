#include "MainComponent.h"
#include "ControlPanel.h"
#include "Config.h"
#include "KeybindHandler.h"
#include "PlaybackLoopController.h"

MainComponent::MainComponent()
{
    audioPlayer = std::make_unique<AudioPlayer>();
    audioPlayer->addChangeListener(this);
    
    controlPanel = std::make_unique<ControlPanel>(*this);
    addAndMakeVisible(controlPanel.get());
    keybindHandler = std::make_unique<KeybindHandler>(*this, *audioPlayer, *controlPanel);
    playbackLoopController = std::make_unique<PlaybackLoopController>(*audioPlayer, *controlPanel);

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
            controlPanel->updateStatsFromAudio();
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
    if (playbackLoopController != nullptr)
        playbackLoopController->tick();
    
    // Handle momentary 'z' zoom key
    const bool isZDown = juce::KeyPress::isKeyCurrentlyDown('z') || juce::KeyPress::isKeyCurrentlyDown('Z');
    controlPanel->setZKeyDown(isZDown);

    // Keep editors in sync
    controlPanel->updateLoopLabels();

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
                controlPanel->updateStatsFromAudio();

                // Run autocut detection if enabled
                auto& sd = controlPanel->getSilenceDetector();
                if (sd.getIsAutoCutInActive()) sd.detectInSilence();
                if (sd.getIsAutoCutOutActive()) sd.detectOutSilence();

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
        audioPlayer->setPositionConstrained(newPosition,
                                           controlPanel->getLoopInPosition(),
                                           controlPanel->getLoopOutPosition());
    }
}

bool MainComponent::keyPressed(const juce::KeyPress& key)
{
    if (keybindHandler != nullptr)
        return keybindHandler->handleKeyPress(key);
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
