#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include "LoopButton.h"
#include "ModernLookAndFeel.h"

class MainComponent  : public juce::AudioAppComponent,
                       public juce::ChangeListener,
                       public juce::Timer {
                       
public:
    enum class ViewMode { Classic, Overlay };
    enum class PlacementMode { None, LoopIn, LoopOut };
    enum class ChannelViewMode { Mono, Stereo };
    enum class ThumbnailQuality { Low, Medium, High };

	MainComponent();

    ~MainComponent() override;

    void mouseDown (const juce::MouseEvent& e) override;

    void mouseDrag (const juce::MouseEvent& e) override;

    void mouseMove (const juce::MouseEvent& e) override;

    void mouseUp (const juce::MouseEvent& e) override;

    void seekToPosition (int x);

    bool keyPressed (const juce::KeyPress& key) override;

    void openButtonClicked();

    void playStopButtonClicked();

    void updateButtonText();

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill);

    void releaseResources() override;

    void changeListenerCallback (juce::ChangeBroadcaster*) override;

    void timerCallback();

    void paint (juce::Graphics& g);

    void resized(); 

private:
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;

    ModernLookAndFeel modernLF;

    juce::TextButton openButton, playStopButton, modeButton, exitButton, statsButton, loopButton, channelViewButton, qualityButton, fullscreenButton;
    LoopButton loopInButton, loopOutButton;
    juce::TextEditor statsDisplay;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::Rectangle<int> waveformBounds, statsBounds;
    ViewMode currentMode = ViewMode::Classic;
    bool showStats = false;
    bool shouldLoop = false;
    double loopInPosition = -1.0;
    double loopOutPosition = -1.0;
    PlacementMode currentPlacementMode = PlacementMode::None;
    int mouseCursorX = -1, mouseCursorY = -1;
    ChannelViewMode currentChannelViewMode = ChannelViewMode::Mono;
    ThumbnailQuality currentQuality = ThumbnailQuality::Low;

    void updateQualityButtonText();
    void drawReducedQualityWaveform(juce::Graphics& g, int channel, int pixelsPerSample);
    void updateLoopButtonColors();

    juce::String formatTime(double seconds);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
