#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "DrunkDrawnLookAndFeel.h"
#include "LoopButton.h" // Include the custom LoopButton header

class MainComponent  : public juce::AudioAppComponent,
                       public juce::ChangeListener,
                       public juce::Timer
{
public:
    enum class ViewMode { Classic, Overlay };
    enum class PlacementMode { None, LoopIn, LoopOut };

    MainComponent() : thumbnailCache (5),
                      thumbnail (512, formatManager, thumbnailCache),
                      loopInButton("Loop In Button"), // Initialize LoopButton
                      loopOutButton("Loop Out Button")  // Initialize LoopButton
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
        openButton.setButtonText ("[D]ir");
        openButton.setName ("dirBtn"); // Name is used as a random seed for wobble
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

        addAndMakeVisible (loopButton);
        loopButton.setButtonText ("[L]oop");
        loopButton.setName ("loopBtn");
        loopButton.setClickingTogglesState (true);
        loopButton.onClick = [this] { shouldLoop = loopButton.getToggleState(); };

        addAndMakeVisible (loopInButton);
        loopInButton.setButtonText ("[I]n");
        loopInButton.setName ("loopInBtn");
        loopInButton.onLeftClick = [this] { loopInPosition = transportSource.getCurrentPosition(); repaint(); };
        loopInButton.onRightClick = [this] { currentPlacementMode = PlacementMode::LoopIn; repaint(); };

        addAndMakeVisible (loopOutButton);
        loopOutButton.setButtonText ("[O]ut");
        loopOutButton.setName ("loopOutBtn");
        loopOutButton.onLeftClick = [this] { loopOutPosition = transportSource.getCurrentPosition(); repaint(); };
        loopOutButton.onRightClick = [this] { currentPlacementMode = PlacementMode::LoopOut; repaint(); };

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
        shutdownAudio();
        thumbnail.removeChangeListener (this);
        stopTimer();
        chooser.reset();
        setLookAndFeel (nullptr); // Crucial: avoids using deleted LF during shutdown
    }

    // --- MOUSE & KEYBOARD INTERACTION ---

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (e.mods.isRightButtonDown()) return;
        // Only seek if not in a placement mode
        if (currentPlacementMode == PlacementMode::None && waveformBounds.contains (e.getMouseDownPosition()))
            seekToPosition (e.x);
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (e.mods.isRightButtonDown()) return;
        if (waveformBounds.contains (e.getPosition()))
            seekToPosition (e.x);
    }

    void mouseMove (const juce::MouseEvent& e) override
    {
        if (currentPlacementMode != PlacementMode::None && waveformBounds.contains (e.getPosition()))
        {
            juce::MouseCursor cursor = juce::MouseCursor::PointingHandCursor; // Or a custom crosshair
            setMouseCursor (cursor);
        }
        else
        {
            setMouseCursor (juce::MouseCursor::NormalCursor);
        }

        if (waveformBounds.contains (e.getPosition()))
        {
            if (mouseCursorX != e.x) // Only repaint if X coordinate changed
            {
                mouseCursorX = e.x;
                repaint();
            }
        }
        else if (mouseCursorX != -1) // If mouse moved out of waveformBounds
        {
            mouseCursorX = -1;
            repaint();
        }
    }

    // MainComponent's own mouseUp, for clicks directly on MainComponent
    void mouseUp (const juce::MouseEvent& e) override
    {
        // Only proceed with waveform interaction if in placement mode
        if (currentPlacementMode != PlacementMode::None && waveformBounds.contains (e.getPosition()))
        {
            auto relativeX = (double)(e.x - waveformBounds.getX());
            auto proportion = relativeX / (double)waveformBounds.getWidth();
            auto newPosition = juce::jlimit (0.0, 1.0, proportion) * thumbnail.getTotalLength();

            if (currentPlacementMode == PlacementMode::LoopIn)
            {
                loopInPosition = newPosition;
                DBG("Loop In set by mouse click on waveform");
            }
            else if (currentPlacementMode == PlacementMode::LoopOut)
            {
                loopOutPosition = newPosition;
                DBG("Loop Out set by mouse click on waveform");
            }
            currentPlacementMode = PlacementMode::None; // Exit placement mode
            repaint(); // Repaint to update loop markers
        }
    }

    void seekToPosition (int x)
    {
        if (thumbnail.getTotalLength() > 0.0)
        {
            auto relativeX = (double)(x - waveformBounds.getX());
            auto proportion = relativeX / (double)waveformBounds.getWidth();
            auto newPosition = juce::jlimit (0.0, 1.0, proportion) * thumbnail.getTotalLength();
            transportSource.setPosition (newPosition);
        }
    }


    bool keyPressed (const juce::KeyPress& key) override
    {
        if (thumbnail.getTotalLength() > 0.0)
        {
            constexpr double skipAmountSeconds = 5.0;
            if (key == juce::KeyPress::leftKey)
            {
                auto newPos = juce::jmax (0.0, transportSource.getCurrentPosition() - skipAmountSeconds);
                transportSource.setPosition (newPos);
                return true;
            }

            if (key == juce::KeyPress::rightKey)
            {
                auto newPos = juce::jmin (thumbnail.getTotalLength(), transportSource.getCurrentPosition() + skipAmountSeconds);
                transportSource.setPosition (newPos);
                return true;
            }
        }

        auto keyCode = key.getTextCharacter();
        if (key == juce::KeyPress::spaceKey) { playStopButtonClicked(); return true; }
        if (keyCode == 'q' || keyCode == 'Q') { juce::JUCEApplication::getInstance()->systemRequestedQuit(); return true; }
        if (keyCode == 'd' || keyCode == 'D') { openButtonClicked(); return true; } // Changed from 'o' to 'd'
        if (keyCode == 's' || keyCode == 'S') { statsButton.triggerClick(); return true; }
        if (keyCode == 'v' || keyCode == 'V') { modeButton.triggerClick(); return true; }
        if (keyCode == 'l' || keyCode == 'L') { loopButton.triggerClick(); return true; }
        if (keyCode == 'i' || keyCode == 'I') { loopInButton.triggerClick(); return true; }
        if (keyCode == 'o' || keyCode == 'O') { loopOutButton.triggerClick(); return true; } // 'O' for loopOut


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
        if (transportSource.isPlaying())
        {
            transportSource.stop();
        }
        else
        {
            if (transportSource.hasStreamFinished())
            {
                if (shouldLoop)
                    transportSource.setPosition (0.0);
                else
                {
                    // If not looping and stream finished, just stop.
                    updateButtonText(); // Update button to "Play" icon
                    return;
                }
            }
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
        // Continuous looping logic (looping the entire track)
        if (!transportSource.isPlaying() && transportSource.hasStreamFinished() && shouldLoop)
        {
            transportSource.setPosition (0.0);
            transportSource.start();
        }

        // Loop point logic
        if (shouldLoop && loopOutPosition > loopInPosition && transportSource.getCurrentPosition() >= loopOutPosition)
        {
            transportSource.setPosition (loopInPosition);
        }

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

            // Draw loop lines and shade
            if (audioLength > 0.0)
            {
                bool inIsSet = loopInPosition > -1.0;
                bool outIsSet = loopOutPosition > -1.0;

                // Draw shade if both are set
                if (inIsSet && outIsSet)
                {
                    auto actualIn = juce::jmin(loopInPosition, loopOutPosition);
                    auto actualOut = juce::jmax(loopInPosition, loopOutPosition);
                    auto inX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (actualIn / audioLength);
                    auto outX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (actualOut / audioLength);

                    g.setColour(juce::Colours::white.withAlpha(0.3f));
                    g.fillRect(juce::Rectangle<float>(inX, (float)waveformBounds.getY(), outX - inX, (float)waveformBounds.getHeight()));
                }

                // Draw individual lines
                g.setColour(juce::Colours::cyan);
                if (inIsSet)
                {
                    auto inX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (loopInPosition / audioLength);
                    g.drawVerticalLine((int)inX, (float)waveformBounds.getY(), (float)waveformBounds.getBottom());
                }
                if (outIsSet)
                {
                    auto outX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (loopOutPosition / audioLength);
                    g.drawVerticalLine((int)outX, (float)waveformBounds.getY(), (float)waveformBounds.getBottom());
                }

                // Draw playhead
                auto drawPosition = (float)transportSource.getCurrentPosition();
                auto x = (drawPosition / audioLength) * (float)waveformBounds.getWidth() + (float)waveformBounds.getX();

                g.setColour (juce::Colours::lime);
                g.drawVerticalLine ((int)x, (float)waveformBounds.getY(), (float)waveformBounds.getBottom());
            }

            // Draw mouse cursor line if active
            if (mouseCursorX != -1)
            {
                // Wider, translucent shadow line
                g.setColour (juce::Colours::darkgrey.withAlpha(0.2f)); // Wider shadow effect
                g.fillRect (mouseCursorX - 2, waveformBounds.getY(), 5, waveformBounds.getHeight()); // 5 pixels wide

                // Thin mouse cursor line
                g.setColour (juce::Colours::yellow); // Distinct color and transparency
                g.drawVerticalLine (mouseCursorX, (float)waveformBounds.getY(), (float)waveformBounds.getBottom());
            }
        }
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        auto bottomArea = bounds.removeFromBottom (50).reduced (5);
        modeButton.setBounds (bottomArea.removeFromRight (80));
        bottomArea.removeFromRight (5);
        loopButton.setBounds (bottomArea.removeFromRight (80)); // Added loopButton
        bottomArea.removeFromRight (5); // Separator
        statsButton.setBounds (bottomArea.removeFromRight (80));

        auto topRow = bounds.removeFromTop (50).reduced (5);
        exitButton.setBounds (topRow.removeFromRight (80));
        openButton.setBounds (topRow.removeFromLeft (80));
        topRow.removeFromLeft (5);
        playStopButton.setBounds (topRow.removeFromLeft (80));
        topRow.removeFromLeft (5); // Separator
        loopButton.setBounds (topRow.removeFromLeft (80)); // Placed after playStopButton
        topRow.removeFromLeft (5); // Separator
        loopInButton.setBounds (topRow.removeFromLeft (80));
        topRow.removeFromLeft (5); // Separator
        loopOutButton.setBounds (topRow.removeFromLeft (80));

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
            drunkLF.setBaseAlpha(1.0f); // Make semi-transparent button backgrounds solid
        }
    }

private:
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;

    DrunkDrawnLookAndFeel drunkLF; //
    juce::TextButton openButton, playStopButton, modeButton, exitButton, statsButton, loopButton;
    LoopButton loopInButton, loopOutButton;
    juce::TextEditor statsDisplay;
    std::unique_ptr<juce::FileChooser> chooser; // <--- The fix for your error!

    juce::Rectangle<int> waveformBounds, statsBounds;
    ViewMode currentMode = ViewMode::Classic;
    bool showStats = false;
    bool shouldLoop = false;
    double loopInPosition = -1.0;
    double loopOutPosition = -1.0;
    PlacementMode currentPlacementMode = PlacementMode::None;
    int mouseCursorX = -1; // -1 indicates no active hover

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};