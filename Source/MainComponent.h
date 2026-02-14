#ifndef AUDIOFILER_MAINCOMPONENT_H
#define AUDIOFILER_MAINCOMPONENT_H

#include <JuceHeader.h>
#include "AudioPlayer.h"
#include "ControlPanel.h" 
#include "AppEnums.h"

class KeybindHandler;
class PlaybackLoopController;

/**
 * @file MainComponent.h
 * @brief Declares the MainComponent class, the central UI component of the audiofiler application.
 */

/**
 * @class MainComponent
 * @brief The main graphical user interface component for the audiofiler application.
 *
 * This class serves as the primary container for the application's UI,
 * managing the layout and interaction of various sub-components such as the
 * `AudioPlayer` and `ControlPanel`. It inherits from `juce::AudioAppComponent`
 * to handle audio input/output, `juce::ChangeListener` to respond to state
 * changes from its child components (like `AudioPlayer`), and `juce::Timer`
 * for periodic UI updates (e.g., refreshing playback time).
 *
 * It is responsible for:
 * - Setting up the audio I/O.
 * - Drawing the main application background and delegating drawing of other components.
 * - Handling global keyboard shortcuts.
 * - Coordinating interactions between the `AudioPlayer` and `ControlPanel`.
 */
class MainComponent  : public juce::AudioAppComponent,
                       public juce::ChangeListener,
                       public juce::Timer
{
public:
  using PlacementMode = AppEnums::PlacementMode; ///< Alias for `AppEnums::PlacementMode` for convenience.

  /**
   * @brief Constructs the MainComponent.
   *
   * Initializes the audio device manager, creates and attaches the `AudioPlayer`
   * and `ControlPanel`, sets up the UI, and starts the timer for periodic updates.
   */
  MainComponent();

  /**
   * @brief Destructor for MainComponent.
   *
   * Cleans up resources and ensures that the audio device is properly shut down.
   */
  ~MainComponent() override;

  //==============================================================================
  /** @name juce::AudioAppComponent Overrides
   *  Methods for handling audio setup and processing.
   *  @{
   */

  /**
   * @brief Prepares the audio source for playback.
   *
   * This is called by the audio device to prepare for audio playback, setting
   * up the `AudioPlayer` with the correct sample rate and buffer size.
   * @param samplesPerBlockExpected The expected number of samples in each audio block.
   * @param sampleRate The sample rate of the audio device.
   */
  void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;

  /**
   * @brief Fills the audio buffer with the next block of audio data.
   *
   * This is the main audio callback where audio data is requested from the
   * `AudioPlayer` and sent to the audio output device.
   * @param bufferToFill The buffer structure to be filled with audio data.
   */
  void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;

  /**
   * @brief Releases any audio resources held by the component.
   *
   * Called when playback stops or when the audio device is shut down.
   */
  void releaseResources() override;
  
  /** @} */
  //==============================================================================

  //==============================================================================
  /** @name juce::Component Overrides
   *  Methods for handling GUI drawing and layout.
   *  @{
   */

  /**
   * @brief Draws the component's content.
   *
   * This method is responsible for painting the background and delegating drawing
   * to its child components (like `ControlPanel` and `AudioPlayer`'s waveform).
   * @param g The graphics context to draw into.
   */
  void paint(juce::Graphics& g) override;

  /**
   * @brief Called when the component's size changes.
   *
   * This method is responsible for laying out and resizing all child components
   * to ensure they adapt correctly to the new window dimensions.
   */
  void resized() override;
  
  /** @} */
  //==============================================================================

  //==============================================================================
  /** @name juce::ChangeListener Overrides
   *  Method for receiving notifications from observed `juce::ChangeBroadcaster` objects.
   *  @{
   */

  /**
   * @brief Callback method triggered when an observed `juce::ChangeBroadcaster` changes state.
   *
   * This `MainComponent` listens to the `AudioPlayer` for playback state changes
   * (e.g., stopped due to end of file), and updates its UI accordingly.
   * @param source A pointer to the `juce::ChangeBroadcaster` that initiated the change.
   */
  void changeListenerCallback(juce::ChangeBroadcaster* source) override;

  /** @} */
  //==============================================================================

  //==============================================================================
  /** @name juce::Timer Overrides
   *  Method for periodic updates.
   *  @{
   */

  /**
   * @brief Callback method triggered periodically by the timer.
   *
   * Used for routine UI updates, such as refreshing the playback time display,
   * updating component positions, and animating visual elements.
   */
  void timerCallback() override;

  /** @} */
  //==============================================================================

  //==============================================================================
  /** @name Event Handlers
   *  Methods for handling user input and specific actions.
   *  @{
   */

  /**
   * @brief Handles global key press events.
   * @param key The `juce::KeyPress` object representing the key that was pressed.
   * @return True if the key event was handled by this component, false otherwise.
   */
  bool keyPressed (const juce::KeyPress& key) override;
  
  /**
   * @brief Initiates the file open dialog to allow the user to select an audio file.
   *
   * When a file is selected, it's loaded by the `AudioPlayer`.
   */
  void openButtonClicked();

  /**
   * @brief Seeks the audio playback position based on an X-coordinate in the waveform.
   *
   * This method is typically called by the `ControlPanel`'s mouse event handlers
   * to allow the user to click on the waveform to scrub through the audio.
   * @param x The X-coordinate in pixels relative to the `MainComponent`'s bounds.
   */
  void seekToPosition (int x);

  /** @} */
  //==============================================================================

  //==============================================================================
  /** @name Utility Methods
   *  @{
   */

  /**
   * @brief Formats a time in seconds into a human-readable string (HH:MM:SS:mmm).
   * @param seconds The time value in seconds to format.
   * @return A `juce::String` containing the formatted time.
   */
  juce::String formatTime(double seconds);
  
  /**
   * @brief Gets a pointer to the `AudioPlayer` instance.
   * @return A raw pointer to the `audioPlayer` managed by this component.
   */
  AudioPlayer* getAudioPlayer() const { return audioPlayer.get(); }

  /** @} */
  //==============================================================================

private:
    //==============================================================================
    /** @name Private Member Variables
     *  Internal components and state of the MainComponent.
     *  @{
     */
    std::unique_ptr<AudioPlayer> audioPlayer;       ///< The audio playback manager.
    std::unique_ptr<juce::FileChooser> chooser;     ///< Used for opening audio files.
    std::unique_ptr<ControlPanel> controlPanel;     ///< The main control panel containing buttons and displays.
    std::unique_ptr<KeybindHandler> keybindHandler; ///< Delegates keyboard shortcut logic.
    std::unique_ptr<PlaybackLoopController> playbackLoopController; ///< Enforces loop bounds during playback.
    juce::OpenGLContext openGLContext; ///< Context for OpenGL rendering.
    
    /** @} */
    //==============================================================================

    //==============================================================================
    //==============================================================================
    /** @name Private Utility Methods
     *  @{
     */

    /** @} */
    //==============================================================================

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

#endif // AUDIOFILER_MAINCOMPONENT_H
