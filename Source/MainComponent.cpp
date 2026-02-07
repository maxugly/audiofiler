#include "MainComponent.h"
#include "ControlPanel.h"
#include "Config.h"

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
            controlPanel->setStatsDisplayText("File loaded: " + audioFile.getFileName());
            controlPanel->setTotalTimeStaticString(formatTime(audioPlayer->getThumbnail().getTotalLength()));
            controlPanel->setLoopInPosition(0.0);
            controlPanel->setLoopOutPosition(audioPlayer->getThumbnail().getTotalLength());
            controlPanel->updateComponentStates();
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
    if (controlPanel->getShouldLoop() && controlPanel->getLoopOutPosition() > controlPanel->getLoopInPosition() && audioPlayer->getTransportSource().getCurrentPosition() >= controlPanel->getLoopOutPosition())
    {
        audioPlayer->getTransportSource().setPosition(controlPanel->getLoopInPosition());
    }
    
    // We repaint continuously for animations and playback cursor
    controlPanel->repaint();
}

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
                controlPanel->setStatsDisplayText("File loaded: " + file.getFileName());
                controlPanel->setTotalTimeStaticString(formatTime(audioPlayer->getThumbnail().getTotalLength()));
                controlPanel->setLoopInPosition(0.0);
                controlPanel->setLoopOutPosition(audioPlayer->getThumbnail().getTotalLength());
                controlPanel->updateLoopLabels();
                controlPanel->updateComponentStates();

                // if (controlPanel->shouldAutoplay())
                //    audioPlayer->togglePlayStop();
            }
            else
            {
                controlPanel->setStatsDisplayText(result.getErrorMessage(), juce::Colours::red);
            }
        }
        grabKeyboardFocus();
    });
}

void MainComponent::mouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown()) return;

    if (controlPanel->getWaveformBounds().contains(e.getMouseDownPosition()))
    {
        auto newPosition = (double)(e.x - controlPanel->getWaveformBounds().getX()) / (double)controlPanel->getWaveformBounds().getWidth() * audioPlayer->getThumbnail().getTotalLength();
        if (newPosition < 0.0) newPosition = 0.0;
        if (newPosition > audioPlayer->getThumbnail().getTotalLength()) newPosition = audioPlayer->getThumbnail().getTotalLength();

        if (controlPanel->getPlacementMode() == AppEnums::PlacementMode::LoopIn) {
            controlPanel->setLoopInPosition(newPosition);
            controlPanel->setPlacementMode(AppEnums::PlacementMode::None);
            controlPanel->ensureLoopOrder();
            controlPanel->updateLoopButtonColors();
            controlPanel->updateLoopLabels();
            controlPanel->repaint();
        } else if (controlPanel->getPlacementMode() == AppEnums::PlacementMode::LoopOut) {
            controlPanel->setLoopOutPosition(newPosition);
            controlPanel->setPlacementMode(AppEnums::PlacementMode::None);
            controlPanel->ensureLoopOrder();
            controlPanel->updateLoopButtonColors();
            controlPanel->updateLoopLabels();
            controlPanel->repaint();
        } else {
            seekToPosition(e.x);
        }
    }
}

void MainComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown()) return;
    if (controlPanel->getPlacementMode() == PlacementMode::None && controlPanel->getWaveformBounds().contains(e.getPosition()))
    {
        seekToPosition(e.x);
    }
}

void MainComponent::mouseMove(const juce::MouseEvent& e)
{
    if (controlPanel->getWaveformBounds().contains(e.getPosition()))
    {
        mouseCursorX = e.x;
        mouseCursorY = e.y;
        // The actual drawing of the cursor is now in ControlPanel, but we might need to pass the coordinates
        // For simplicity, we trigger a repaint and let ControlPanel handle it.
        controlPanel->repaint();
    }
    else if (mouseCursorX != -1)
    {
        mouseCursorX = -1;
        mouseCursorY = -1;
        controlPanel->repaint();
    }
}

void MainComponent::mouseUp(const juce::MouseEvent& e)
{
    // Potentially handle something here if needed
}

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
    // Simplified, assuming controlPanel handles the logic of whether to ignore
    if (k == 'i' || k == 'I') {
        controlPanel->setLoopInPosition(audioPlayer->getTransportSource().getCurrentPosition());
        controlPanel->repaint();
        return true;
    }
    if (k == 'o' || k == 'O') {
        controlPanel->setLoopOutPosition(audioPlayer->getTransportSource().getCurrentPosition());
        controlPanel->repaint();
        return true;
    }
    if (k == 'u' || k == 'U') { controlPanel->clearLoopIn(); return true; }
    if (k == 'p' || k == 'P') { controlPanel->clearLoopOut(); return true; }
    return false;
}

juce::String MainComponent::formatTime(double seconds) {
  if (seconds < 0) seconds = 0;
  int hours = (int)(seconds / 3600.0);
  int minutes = ((int)(seconds / 60.0)) % 60;
  int secs = ((int)seconds) % 60;
  int milliseconds = (int)((seconds - (int)seconds) * 1000.0);
  return juce::String::formatted("%02d:%02d:%02d:%03d", hours, minutes, secs, milliseconds);
}