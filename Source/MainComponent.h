#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "DrunkDrawnLookAndFeel.h"

class MainComponent  : public juce::AudioAppComponent,
                       public juce::ChangeListener,
                       public juce::Timer
{
public:
    enum class ViewMode { Classic, Overlay };

    MainComponent() : thumbnailCache (5),
                      thumbnail (512, formatManager, thumbnailCache)
    {
        // 1. Audio Formats
        formatManager.registerFormat (new juce::WavAudioFormat(), false);
        formatManager.registerFormat (new juce::AiffAudioFormat(), false);
        formatManager.registerFormat (new juce::FlacAudioFormat(), false);
        formatManager.registerFormat (new juce::OggVorbisAudioFormat(), false);
        formatManager.registerFormat (new juce::MP3AudioFormat(), false);

        thumbnail.addChangeListener (this);

        // 2. Look and Feel - Set up the "Sorta" Brand Colors
        setLookAndFeel (&drunkLF);
        drunkLF.setButtonOnColorRange({ juce::Colour(0xffff1493), 0.08f, 0.12f, 0.1f }); // Hot Pink
        drunkLF.setButtonOutlineColorRange({ juce::Colour(0xff00ffff), 0.1f, 0.15f, 0.15f }); // Cyan
        drunkLF.setTextColorRange({ juce::Colour(0xff00ffff), 0.0f, 0.0f, 0.1f }); // High Contrast Cyan

        // 3. UI Components with [Shortcuts] in labels
        addAndMakeVisible (openButton);
        openButton.setButtonText ("[O]pen");
        openButton.setName ("openBtn"); // Name is used as a random seed for wobble
        openButton.onClick = [this] { openButtonClicked(); };

        addAndMakeVisible (playStopButton);
        updateButtonText();
        playStopButton.setName ("playBtn");
        playStopButton.onClick = [this] { playStopButtonClicked(); };
        playStopButton.setEnabled (false);

        addAndMakeVisible (modeButton);
        modeButton.setButtonText ("[V]iew");
        modeButton.setName ("viewBtn");
        modeButton.onClick = [this] {
            currentMode = (currentMode == ViewMode::Classic) ? ViewMode::Overlay : ViewMode::Classic;
            resized();
            repaint();
        };

        addAndMakeVisible (exitButton);
        exitButton.setButtonText ("[Q]uit");
        exitButton.setName ("exitBtn");
        exitButton.setColour (juce::TextButton::buttonColourId, juce::Colours::darkred);
        exitButton.onClick = [] { juce::JUCEApplication::getInstance()->systemRequestedQuit(); };

        addAndMakeVisible (statsButton);
        statsButton.setButtonText ("[S]tats");
        statsButton.setName ("statsBtn");
        statsButton.setClickingTogglesState (true);
        statsButton.onClick = [this] {
            showStats = statsButton.getToggleState();
            resized();
        };

        addAndMakeVisible (statsDisplay);
        statsDisplay.setReadOnly (true);
        statsDisplay.setMultiLine (true);
        statsDisplay.setWantsKeyboardFocus (false); // Prevents debug window from eating shortcuts
        statsDisplay.setColour (juce::TextEditor::backgroundColourId, juce::Colours::black.withAlpha(0.5f));
        statsDisplay.setVisible (false);

        setSize (800, 400);
        setAudioChannels (0, 2);
        startTimerHz (60);
        setWantsKeyboardFocus (true); // Needed for key listeners
    }

    ~MainComponent() override
    {
        setLookAndFeel (nullptr); // Crucial: avoids using deleted LF during shutdown
        shutdownAudio();
    }

    // --- KEYBOARD SHORTCUTS ---
    bool keyPressed (const juce::KeyPress& key) override
    {
        auto keyCode = key.getTextCharacter();

        if (key == juce::KeyPress::spaceKey) { playStopButtonClicked(); return true; }
        if (keyCode == 'q' || keyCode == 'Q') { juce::JUCEApplication::getInstance()->systemRequestedQuit(); return true; }
        if (keyCode == 'o' || keyCode == 'O') { openButtonClicked(); return true; }
        if (keyCode == 's' || keyCode == 'S') { statsButton.triggerClick(); return true; }
        if (keyCode == 'v' || keyCode == 'V') { modeButton.triggerClick(); return true; }

        return false;
    }

    void openButtonClicked()
    {
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
                    playStopButton.setEnabled (true);
                    readerSource.reset (newSource.release());
                    updateButtonText();
                }
            }
        });
    }

    void playStopButtonClicked()
    {
        if (transportSource.isPlaying()) transportSource.stop();
        else {
            if (transportSource.hasStreamFinished()) transportSource.setPosition (0.0);
            transportSource.start();
        }
        updateButtonText();
    }

    void updateButtonText()
    {
        if (transportSource.isPlaying())
            playStopButton.setButtonText (juce::CharPointer_UTF8 ("\xe2\x8f\xb8")); // Pause Icon
        else
            playStopButton.setButtonText (juce::CharPointer_UTF8 ("\xe2\x96\xb6")); // Play Icon
    }

    // --- AUDIO & TIMER ---
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override { transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate); }
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override {
        if (readerSource.get() == nullptr) bufferToFill.clearActiveBufferRegion();
        else transportSource.getNextAudioBlock (bufferToFill);
    }
    void releaseResources() override { transportSource.releaseResources(); }

    void timerCallback() override
    {
        if (!transportSource.isPlaying() && playStopButton.getButtonText().contains(juce::CharPointer_UTF8 ("\xe2\x8f\xb8")))
            updateButtonText();

        if (showStats)
        {
            juce::String debugInfo;
            debugInfo << "Samples Loaded: " << thumbnail.getNumSamplesFinished() << "\n";
            debugInfo << "Approx Peak: " << thumbnail.getApproximatePeak() << "\n";
            float minV, maxV;
            thumbnail.getApproximateMinMax(0.0, thumbnail.getTotalLength(), 0, minV, maxV);
            debugInfo << "Min/Max: " << minV << " / " << maxV << "\n";
            debugInfo << "Position: " << juce::String(transportSource.getCurrentPosition(), 2) << "s";
            statsDisplay.setText (debugInfo, false);
        }
        repaint();
    }

    void changeListenerCallback (juce::ChangeBroadcaster*) override { repaint(); }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::black);
        if (thumbnail.getNumChannels() > 0)
        {
            g.setColour (juce::Colours::deeppink);
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
        auto bounds = getLocalBounds();

        auto bottomArea = bounds.removeFromBottom (40).reduced (5);
        modeButton.setBounds (bottomArea.removeFromRight (80));
        bottomArea.removeFromRight (5);
        statsButton.setBounds (bottomArea.removeFromRight (80));

        auto topRow = bounds.removeFromTop (50).reduced (5);
        exitButton.setBounds (topRow.removeFromRight (80));
        openButton.setBounds (topRow.removeFromLeft (80));
        topRow.removeFromLeft (5);
        playStopButton.setBounds (topRow.removeFromLeft (80));

        if (showStats)
        {
            statsDisplay.setVisible (true);
            statsBounds = bounds.removeFromBottom (100).reduced (10, 5);
            statsDisplay.setBounds (statsBounds);
        }
        else { statsDisplay.setVisible (false); }

        if (currentMode == ViewMode::Classic)
        {
            waveformBounds = bounds.reduced (10, 0);
            drunkLF.setBaseAlpha(1.0f); // Solid buttons
        }
        else
        {
            waveformBounds = getLocalBounds();
            drunkLF.setBaseAlpha(0.6f); // Semi-transparent button backgrounds only
        }
    }

private:
    juce::AudioFormatManager formatManager;
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;

    DrunkDrawnLookAndFeel drunkLF; //
    juce::TextButton openButton, playStopButton, modeButton, exitButton, statsButton;
    juce::TextEditor statsDisplay;
    std::unique_ptr<juce::FileChooser> chooser; // <--- The fix for your error!

    juce::Rectangle<int> waveformBounds, statsBounds;
    ViewMode currentMode = ViewMode::Classic;
    bool showStats = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};