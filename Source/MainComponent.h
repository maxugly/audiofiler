#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

class MainComponent  : public juce::AudioAppComponent,
                       public juce::ChangeListener,
                       public juce::Timer
{
public:
    MainComponent() : thumbnailCache (5),
                      thumbnail (512, formatManager, thumbnailCache)
    {

        formatManager.registerFormat (new juce::WavAudioFormat(), false);
        formatManager.registerFormat (new juce::AiffAudioFormat(), false);
        formatManager.registerFormat (new juce::FlacAudioFormat(), false);
        formatManager.registerFormat (new juce::OggVorbisAudioFormat(), false);
        formatManager.registerFormat (new juce::MP3AudioFormat(), false);

        thumbnail.addChangeListener (this);

        addAndMakeVisible (openButton);
        openButton.setButtonText ("Open...");
        openButton.onClick = [this] { openButtonClicked(); };

        addAndMakeVisible (playButton);
        playButton.setButtonText ("Play");
        playButton.onClick = [this] { playButtonClicked(); };
        playButton.setEnabled (false);

        addAndMakeVisible (stopButton);
        stopButton.setButtonText ("Stop");
        stopButton.onClick = [this] { stopButtonClicked(); };
        stopButton.setEnabled (false);

        setSize (800, 400);
        setAudioChannels (0, 2);
        startTimerHz (60);
        setWantsKeyboardFocus (true);
    }

    ~MainComponent() override { shutdownAudio(); }

    bool keyPressed (const juce::KeyPress& key) override
    {
        if (key == juce::KeyPress::spaceKey)
        {
            if (transportSource.isPlaying()) stopButtonClicked();
            else                           playButtonClicked();
            return true;
        }
        return false;
    }

    void openButtonClicked()
    {
        auto filter = formatManager.getWildcardForAllFormats();

        chooser = std::make_unique<juce::FileChooser> ("Select Audio...",
            juce::File::getSpecialLocation (juce::File::userHomeDirectory),
            filter);

        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

        chooser->launchAsync (flags, [this] (const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file.exists())
            {
                auto* reader = formatManager.createReaderFor (file);
                if (reader != nullptr)
                {
                    auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);
                    transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);
                    thumbnail.setSource (new juce::FileInputSource (file));

                    playButton.setEnabled (true);
                    readerSource.reset (newSource.release());
                }
            }
        });
    }

    void playButtonClicked()
    {
        transportSource.start();
        playButton.setEnabled (false);
        stopButton.setEnabled (true);
    }

    void stopButtonClicked()
    {
        transportSource.stop();
        transportSource.setPosition (0.0);
        stopButton.setEnabled (false);
        playButton.setEnabled (true);
    }

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        if (readerSource.get() == nullptr)
            bufferToFill.clearActiveBufferRegion();
        else
            transportSource.getNextAudioBlock (bufferToFill);
    }

    void releaseResources() override { transportSource.releaseResources(); }

    void timerCallback() override { repaint(); }
    void changeListenerCallback (juce::ChangeBroadcaster*) override { repaint(); }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::black);
        g.setColour (juce::Colours::deeppink);

        auto waveformBounds = getLocalBounds().reduced (10, 50);

        if (thumbnail.getNumChannels() > 0)
        {
            thumbnail.drawChannels (g, waveformBounds, 0.0, thumbnail.getTotalLength(), 1.0f);

            auto audioLength = (float)thumbnail.getTotalLength();
            auto drawPosition = (float)transportSource.getCurrentPosition();
            auto x = (drawPosition / audioLength) * (float)waveformBounds.getWidth() + (float)waveformBounds.getX();

            g.setColour (juce::Colours::white);
            g.drawVerticalLine ((int)x, (float)waveformBounds.getY(), (float)waveformBounds.getBottom());
        }
    }

    void resized() override
    {
        auto area = getLocalBounds().removeFromTop (40);
        openButton.setBounds (area.removeFromLeft (100).reduced (2));
        playButton.setBounds (area.removeFromLeft (100).reduced (2));
        stopButton.setBounds (area.removeFromLeft (100).reduced (2));
    }

private:
    juce::AudioFormatManager formatManager;
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;

    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;

    juce::TextButton openButton, playButton, stopButton;
    std::unique_ptr<juce::FileChooser> chooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};