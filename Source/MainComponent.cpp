#include "MainComponent.h"
#include "TimeUtils.h"
#include "ControlPanel.h"
#include "Config.h"
#include "KeybindHandler.h"
#include "PlaybackLoopController.h"

MainComponent::MainComponent()
{
    // 1. Initialize logic and engines
    audioPlayer = std::make_unique<AudioPlayer>();
    audioPlayer->addChangeListener(this);
    
    // 2. Initialize UI components
    controlPanel = std::make_unique<ControlPanel>(*this);
    addAndMakeVisible(controlPanel.get());
    
    // 3. Initialize controllers that bridge UI and Logic
    keybindHandler = std::make_unique<KeybindHandler>(*this, *audioPlayer, *controlPanel);
    playbackLoopController = std::make_unique<PlaybackLoopController>(*audioPlayer, *controlPanel);

    // 4. Set Audio Channels (Important to do before some UI sizing if they depend on audio state)
    setAudioChannels(0, 2);

    // 5. Set Window Size - This triggers resized() and lays out the ControlPanel
    setSize(Config::Layout::Window::width, Config::Layout::Window::height);
    
    // 6. Start the UI Refresh Timer
    startTimerHz(60);

    // 7. Focus Setup - We allow the component to receive focus, 
    // but we no longer "grab" it here to avoid Peer assertion crashes.
    setWantsKeyboardFocus(true);
    openGLContext.attachTo(*this);

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
        }
        else
        {
            controlPanel->setStatsDisplayText(result.getErrorMessage(), Config::Colors::statsErrorText);
        }
    }
#endif
}

MainComponent::~MainComponent()
{
    openGLContext.detach();
    audioPlayer->removeChangeListener(this);
    shutdownAudio();
    stopTimer();
}

//==============================================================================
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

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    // ControlPanel handles the bulk of the UI painting.
    // Fill the background to prevent "ghosting" artifacts.
    g.fillAll(Config::Colors::Window::background);
}

void MainComponent::resized()
{
    // Ensure the control panel fills the entire available space
    if (controlPanel != nullptr)
        controlPanel->setBounds(getLocalBounds());
}

//==============================================================================
void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == audioPlayer.get())
    {
        controlPanel->updatePlayButtonText(audioPlayer->isPlaying());
        repaint(); 
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

    // Redraw the control panel (includes waveform and cursor)
    controlPanel->repaint();
}

//==============================================================================
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
                controlPanel->updateStatsFromAudio();

                auto& sd = controlPanel->getSilenceDetector();
                if (sd.getIsAutoCutInActive()) sd.detectInSilence();
                if (sd.getIsAutoCutOutActive()) sd.detectOutSilence();

                if (controlPanel->shouldAutoplay())
                   audioPlayer->togglePlayStop();
            }
            else
            {
                controlPanel->setStatsDisplayText(result.getErrorMessage(), Config::Colors::statsErrorText);
            }
        }
        // Safely request focus after a UI interaction
        grabKeyboardFocus();
    });
}

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

juce::String MainComponent::formatTime(double seconds) {
  return TimeUtils::formatTime(seconds);
}
