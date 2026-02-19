#ifndef AUDIOFILER_MAINCOMPONENT_H
#define AUDIOFILER_MAINCOMPONENT_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_extra/juce_gui_extra.h>
    #include <juce_opengl/juce_opengl.h>
#else
    #include <JuceHeader.h>
#endif

#include "AudioPlayer.h"
#include "ControlPanel.h" 
#include "AppEnums.h"
#include "SessionState.h"

class KeybindHandler;

class PlaybackRepeatController;

class MainComponent  : public juce::AudioAppComponent,
                    public juce::ChangeListener
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

  bool keyPressed (const juce::KeyPress& key) override;

  void openButtonClicked();

  void seekToPosition (int x);

  AudioPlayer* getAudioPlayer() const { return audioPlayer.get(); }

private:

    SessionState sessionState;
    std::unique_ptr<AudioPlayer> audioPlayer;       
    std::unique_ptr<juce::FileChooser> chooser;     
    std::unique_ptr<ControlPanel> controlPanel;     
    std::unique_ptr<KeybindHandler> keybindHandler; 
    std::unique_ptr<PlaybackRepeatController> playbackRepeatController; 
    juce::OpenGLContext openGLContext; 

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

#endif 
