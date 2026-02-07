#pragma once

#include <JuceHeader.h>
#include "AudioPlayer.h"
#include "ControlPanel.h" 
#include "AppEnums.h"

class MainComponent  : public juce::AudioAppComponent,
                       public juce::ChangeListener,
                       public juce::Timer
{
public:
  using PlacementMode = AppEnums::PlacementMode;

  MainComponent();
  ~MainComponent() override;

  void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
  void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
  void releaseResources() override;
  
  void paint(juce::Graphics& g) override;
  void resized() override;
  
  void changeListenerCallback(juce::ChangeBroadcaster* source) override;
  void timerCallback() override;

  void mouseDown (const juce::MouseEvent& e) override;
  void mouseDrag (const juce::MouseEvent& e) override;
  void mouseMove (const juce::MouseEvent& e) override;
  void mouseUp (const juce::MouseEvent& e) override;
  bool keyPressed (const juce::KeyPress& key) override;
  
  void openButtonClicked();
  void seekToPosition (int x);
  juce::String formatTime(double seconds);
  
  AudioPlayer* getAudioPlayer() const { return audioPlayer.get(); }

private:
    std::unique_ptr<AudioPlayer> audioPlayer;
    std::unique_ptr<juce::FileChooser> chooser;
    std::unique_ptr<ControlPanel> controlPanel;

    int mouseCursorX = -1;
    int mouseCursorY = -1;
    
    // Keypress handlers
    bool handleGlobalKeybinds(const juce::KeyPress& key);
    bool handlePlaybackKeybinds(const juce::KeyPress& key);
    bool handleUIToggleKeybinds(const juce::KeyPress& key);
    bool handleLoopKeybinds(const juce::KeyPress& key);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};