#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include "LoopButton.h" // Include the custom LoopButton header
#include "ModernLookAndFeel.h"

class MainComponent  : public juce::AudioAppComponent,
                       public juce::ChangeListener,
                       public juce::Timer
{
public:
    enum class ViewMode { Classic, Overlay };
    enum class PlacementMode { None, LoopIn, LoopOut };
    enum class ChannelViewMode { Mono, Stereo };

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
        setLookAndFeel (&modernLF);
        modernLF.setBaseOffColor(juce::Colour(0xff3a3a3a)); // Dark Grey
        modernLF.setBaseOnColor(juce::Colour(0xff00bfff));  // Deep Sky Blue
        modernLF.setTextColor(juce::Colours::white);

        // 3. UI Components with [Shortcuts] in labels
        addAndMakeVisible (openButton);
        openButton.setButtonText ("[D]ir");

        openButton.onClick = [this] { openButtonClicked(); };

        addAndMakeVisible (playStopButton);
        updateButtonText();

        playStopButton.onClick = [this] { playStopButtonClicked(); };
        playStopButton.setEnabled (false);

        addAndMakeVisible (modeButton);
        modeButton.setButtonText ("[V]iew");
        modeButton.setClickingTogglesState (true); // Make it a toggle button

        modeButton.onClick = [this] {
            // Update currentMode based on the button's toggle state
            currentMode = modeButton.getToggleState() ? ViewMode::Overlay : ViewMode::Classic;
            
            // Update button text to reflect current mode for better feedback
            modeButton.setButtonText (currentMode == ViewMode::Classic ? "[V]iew 1" : "[V]iew 2");
            
            resized();
            repaint();
        };

        // Channel View Button
        addAndMakeVisible (channelViewButton);
        channelViewButton.setButtonText ("[C]han");
        channelViewButton.setClickingTogglesState (true);

        channelViewButton.onClick = [this] {
            currentChannelViewMode = channelViewButton.getToggleState() ? ChannelViewMode::Stereo : ChannelViewMode::Mono;
            channelViewButton.setButtonText (currentChannelViewMode == ChannelViewMode::Mono ? "[C]han 1" : "[C]han 2");
            repaint();
        };

        addAndMakeVisible (exitButton);
        exitButton.setButtonText ("[Q]uit");

        exitButton.setColour (juce::TextButton::buttonColourId, juce::Colours::darkred);
        exitButton.onClick = [] { juce::JUCEApplication::getInstance()->systemRequestedQuit(); };

        addAndMakeVisible (statsButton);
        statsButton.setButtonText ("[S]tats");

        statsButton.setClickingTogglesState (true);
        statsButton.onClick = [this] {
            showStats = statsButton.getToggleState();
            resized();
        };

        addAndMakeVisible (loopButton);
        loopButton.setButtonText ("[L]oop");

        loopButton.setClickingTogglesState (true);
        loopButton.onClick = [this] { shouldLoop = loopButton.getToggleState(); };

        addAndMakeVisible (loopInButton);
        loopInButton.setButtonText ("[I]n");

        loopInButton.onLeftClick = [this] { loopInPosition = transportSource.getCurrentPosition(); repaint(); };
        loopInButton.onRightClick = [this] { currentPlacementMode = PlacementMode::LoopIn; repaint(); };

        addAndMakeVisible (loopOutButton);
        loopOutButton.setButtonText ("[O]ut");

        loopOutButton.onLeftClick = [this] { loopOutPosition = transportSource.getCurrentPosition(); repaint(); };
        loopOutButton.onRightClick = [this] { currentPlacementMode = PlacementMode::LoopOut; repaint(); };

        addAndMakeVisible (statsDisplay);
        statsDisplay.setReadOnly (true);
        statsDisplay.setMultiLine (true);
        statsDisplay.setWantsKeyboardFocus (false); // Prevents debug window from eating shortcuts
        statsDisplay.setColour (juce::TextEditor::backgroundColourId, juce::Colours::black.withAlpha(0.5f));
        statsDisplay.setColour (juce::TextEditor::textColourId, juce::Colours::white); // Ensure text is 100% opaque white
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
        // Only seek if not in a placement mode
        if (currentPlacementMode == PlacementMode::None && waveformBounds.contains (e.getPosition()))
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
            if (mouseCursorX != e.x || mouseCursorY != e.y) // Only repaint if X or Y coordinate changed
            {
                mouseCursorX = e.x;
                mouseCursorY = e.y;
                repaint();
            }
        }
        else if (mouseCursorX != -1) // If mouse moved out of waveformBounds
        {
            mouseCursorX = -1;
            mouseCursorY = -1;
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
        if (keyCode == 'c' || keyCode == 'C') { channelViewButton.triggerClick(); return true; }
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
                    DBG("AudioThumbnail: Num Channels = " << thumbnail.getNumChannels() << ", Total Length = " << thumbnail.getTotalLength());
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
        DBG("Paint: thumbnail.getNumChannels() = " << thumbnail.getNumChannels());

        if (thumbnail.getNumChannels() > 0)
        {
            // Draw waveform based on channel view mode
            if (currentChannelViewMode == ChannelViewMode::Mono || thumbnail.getNumChannels() == 1)
            {
                // Draw only the first channel (mono view)
                g.setColour (juce::Colours::deeppink); // Set color here, as drawChannel doesn't do it
                DBG("Paint: Drawing Mono channel (0)");
                thumbnail.drawChannel (g, waveformBounds, 0.0, thumbnail.getTotalLength(), 0, 1.0f);
            }
            else // Stereo view
            {
                // Draw all available channels
                g.setColour (juce::Colours::deeppink); // Set color here, as drawChannels doesn't do it
                DBG("Paint: Drawing Stereo channels");
                thumbnail.drawChannels (g, waveformBounds, 0.0, thumbnail.getTotalLength(), 1.0f);
            }

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
                g.setColour(juce::Colours::blue);
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

                // Calculate playhead position
                auto drawPosition = (float)transportSource.getCurrentPosition();
                auto x = (drawPosition / audioLength) * (float)waveformBounds.getWidth() + (float)waveformBounds.getX();

                if (transportSource.isPlaying())
                {
                    // Draw fading green shadow behind playhead (when playing)
                    juce::ColourGradient gradient (
                        juce::Colours::lime.withAlpha(0.0f),         // Start colour (transparent lime)
                        (float)x - 10.0f, (float)waveformBounds.getCentreY(), // Start point (left of tail)
                        juce::Colours::lime.withAlpha(0.5f),         // End colour (semi-transparent lime)
                        (float)x, (float)waveformBounds.getCentreY(), // End point (at playhead)
                        false // Don't repeat
                    );
                    g.setGradientFill (gradient);
                    g.fillRect (juce::Rectangle<float>((int)x - 10, (float)waveformBounds.getY(), 10, (float)waveformBounds.getHeight()));
                }
                else // When stopped: draw a shorter, centered "glow"
                {
                    juce::ColourGradient glowGradient;
                    glowGradient.addColour (0.0, juce::Colours::lime.withAlpha(0.0f)); // Transparent at left edge of glow
                    glowGradient.addColour (0.5, juce::Colours::lime.withAlpha(0.5f)); // Semi-transparent in middle (at playhead)
                    glowGradient.addColour (1.0, juce::Colours::lime.withAlpha(0.0f)); // Transparent at right edge of glow
                    
                    glowGradient.point1 = { (float)x - 5.0f, (float)waveformBounds.getCentreY() };
                    glowGradient.point2 = { (float)x + 5.0f, (float)waveformBounds.getCentreY() };

                    g.setGradientFill (glowGradient);
                    // Draw a 10-pixel wide rectangle centered around x
                    g.fillRect (juce::Rectangle<float>((int)x - 5, (float)waveformBounds.getY(), 10, (float)waveformBounds.getHeight()));
                }

                // Draw playhead
                g.setColour (juce::Colours::lime);
                g.drawVerticalLine ((int)x, (float)waveformBounds.getY(), (float)waveformBounds.getBottom());
            }

            // Draw mouse cursor line if active
            if (mouseCursorX != -1)
            {
                // Wider, translucent shadow for vertical line
                g.setColour (juce::Colours::darkorange.withAlpha(0.4f));
                g.fillRect (mouseCursorX - 2, waveformBounds.getY(), 5, waveformBounds.getHeight());

                // Wider, translucent shadow for horizontal line
                g.fillRect (waveformBounds.getX(), mouseCursorY - 2, waveformBounds.getWidth(), 5);

                // Thin vertical mouse cursor line
                g.setColour (juce::Colours::yellow);
                g.drawVerticalLine (mouseCursorX, (float)waveformBounds.getY(), (float)waveformBounds.getBottom());

                // Thin horizontal mouse cursor line
                g.drawHorizontalLine (mouseCursorY, (float)waveformBounds.getX(), (float)waveformBounds.getRight());
            }
        }
    }

    void resized() override
    {
        auto totalBounds = getLocalBounds();
        auto currentWorkingBounds = totalBounds; // This will be consumed by buttons to define central area

        // --- Calculate Button Layout first, consuming space ---

        // Top Row Buttons area
        auto topButtonsArea = currentWorkingBounds.removeFromTop (50).reduced (5);
        // Store the final bottom edge of top buttons for later reference
        int topButtonsBottomEdge = topButtonsArea.getBottom(); 

        // Bottom Row Buttons area
        auto bottomButtonsArea = currentWorkingBounds.removeFromBottom (50).reduced (5);
        // Store the final top edge of bottom buttons for later reference
        int bottomButtonsTopEdge = bottomButtonsArea.getY(); 

        // --- Position ALL Buttons ---
        // Top row
        exitButton.setBounds (topButtonsArea.removeFromRight (80));
        openButton.setBounds (topButtonsArea.removeFromLeft (80));
        topButtonsArea.removeFromLeft (5); // Spacer
        playStopButton.setBounds (topButtonsArea.removeFromLeft (80));
        topButtonsArea.removeFromLeft (5); // Spacer
        loopButton.setBounds (topButtonsArea.removeFromLeft (80));
        topButtonsArea.removeFromLeft (5); // Spacer
        loopInButton.setBounds (topButtonsArea.removeFromLeft (80));
        topButtonsArea.removeFromLeft (5); // Spacer
        loopOutButton.setBounds (topButtonsArea.removeFromLeft (80));

        // Bottom row
        modeButton.setBounds (bottomButtonsArea.removeFromRight (80));
        bottomButtonsArea.removeFromRight (5); // Spacer
        statsButton.setBounds (bottomButtonsArea.removeFromRight (80));
        bottomButtonsArea.removeFromRight (5); // Spacer
        channelViewButton.setBounds (bottomButtonsArea.removeFromRight (80));

        // --- Determine Waveform Bounds based on Mode ---
        if (currentMode == ViewMode::Classic)
        {
            // In Classic mode, waveform occupies the remaining 'currentWorkingBounds'
            waveformBounds = currentWorkingBounds.reduced (10, 0); 
        }
        else // ViewMode::Overlay
        {
            // In Overlay mode, waveform takes the full window space
            waveformBounds = totalBounds; 
        }

        // --- Position Stats Display (always floating over waveform) ---
        if (showStats)
        {
            statsDisplay.setVisible (true);
            int statsHeight = 100;
            int padding = 10;      // Consistent padding variable

            statsBounds = juce::Rectangle<int> (
                waveformBounds.getX() + padding, // X position: left aligned with waveformBounds, plus padding
                bottomButtonsTopEdge - statsHeight - padding, // Y position: above bottom buttons
                waveformBounds.getWidth() - (2 * padding), // Width: full width of waveformBounds, minus padding on both sides
                statsHeight
            );
            statsDisplay.setBounds (statsBounds);
            statsDisplay.toFront(true);
        }
        else
        {
            statsDisplay.setVisible (false);
        }

        // --- Ensure all buttons are visible ---
        openButton.setVisible(true); playStopButton.setVisible(true); modeButton.setVisible(true);
        exitButton.setVisible(true); statsButton.setVisible(true); loopButton.setVisible(true);
        loopInButton.setVisible(true); loopOutButton.setVisible(true);
    }

private:
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;

    ModernLookAndFeel modernLF;


    juce::TextButton openButton, playStopButton, modeButton, exitButton, statsButton, loopButton, channelViewButton;
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
    int mouseCursorX = -1, mouseCursorY = -1; // -1 indicates no active hover
    ChannelViewMode currentChannelViewMode = ChannelViewMode::Mono; // Default to mono view

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};