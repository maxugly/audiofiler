

#include "MainComponent.h"
#include "TimeUtils.h"
#include "ControlPanel.h"
#include "Config.h"
#include "KeybindHandler.h"
#include "PlaybackRepeatController.h"

MainComponent::MainComponent()
{

    audioPlayer = std::make_unique<AudioPlayer>(sessionState);
    audioPlayer->addChangeListener(this);

    controlPanel = std::make_unique<ControlPanel>(*this, sessionState);
    addAndMakeVisible(controlPanel.get());

    keybindHandler = std::make_unique<KeybindHandler>(*this, *audioPlayer, *controlPanel);
    playbackRepeatController = std::make_unique<PlaybackRepeatController>(*audioPlayer, *controlPanel);

    setAudioChannels(0, 2);

    setSize(Config::Layout::Window::width, Config::Layout::Window::height);

    setWantsKeyboardFocus(true);
    openGLContext.attachTo(*this);

}

MainComponent::~MainComponent()
{
    openGLContext.detach();
    audioPlayer->removeChangeListener(this);

    shutdownAudio();

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

    g.fillAll(Config::Colors::Window::background);
}

void MainComponent::resized()
{

    if (controlPanel != nullptr)
        controlPanel->setBounds(getLocalBounds());
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == audioPlayer.get())
    {
        controlPanel->updatePlayButtonText(audioPlayer->isPlaying());

        repaint(); 
    }
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
                controlPanel->setTotalTimeStaticString(TimeUtils::formatTime(audioPlayer->getThumbnail().getTotalLength()));

                controlPanel->updateCutLabels();
                controlPanel->updateComponentStates();
                controlPanel->updateStatsFromAudio();

                if (controlPanel->shouldAutoplay())
                   audioPlayer->togglePlayStop();
            }
            else
            {
                controlPanel->setStatsDisplayText(result.getErrorMessage(), Config::Colors::statsErrorText);
            }
        }

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

        audioPlayer->setPlayheadPosition(newPosition);
    }
}

bool MainComponent::keyPressed(const juce::KeyPress& key)
{
    if (keybindHandler != nullptr)
        return keybindHandler->handleKeyPress(key);
    return false;
}
