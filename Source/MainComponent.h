#ifndef AUDIOFILER_MAINCOMPONENT_H
#define AUDIOFILER_MAINCOMPONENT_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_extra/juce_gui_extra.h>
    #include <juce_opengl/juce_opengl.h>
#else
    #include <JuceHeader.h>
#endif

#include "Core/AudioPlayer.h"
#include "UI/ControlPanel.h" 
#include "Core/AppEnums.h"
#include "Core/SessionState.h"

class KeybindHandler;

/**
 * @defgroup UI UI Components
 * @brief User interface classes, including custom components, views, and presenters.
 *
 * @defgroup AudioEngine Audio Engine
 * @brief Core audio processing, file handling, and playback logic.
 *
 * @defgroup State State Management
 * @brief Data models, configuration, and application state.
 *
 * @defgroup Threading Threading/Workers
 * @brief Background threads for analysis and file reading.
 *
 * @defgroup Helpers Helpers
 * @brief Utility functions and static helper classes.
 */

/**
 * @ingroup UI
 * @class MainComponent
 * @brief The root component of the application.
 * @details This class is the main entry point for the UI and audio processing.
 *          It owns the high-level `AudioPlayer` (audio engine) and `ControlPanel` (UI),
 *          and manages the `SessionState` (data model).
 *
 *          It implements `juce::AudioAppComponent` to handle audio callbacks and
 *          `juce::ChangeListener` to respond to transport changes.
 *
 * @see AudioPlayer
 * @see ControlPanel
 * @see SessionState
 */
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
    juce::OpenGLContext openGLContext; 

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

#endif 
