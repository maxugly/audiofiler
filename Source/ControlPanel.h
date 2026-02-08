#pragma once

#include <JuceHeader.h>
#include "Config.h"
#include "ModernLookAndFeel.h"
#include "AppEnums.h"
#include "AudioPlayer.h" // Added for AudioPlayer type recognition

class MainComponent; // Forward declaration
#include <memory>    // Required for std::unique_ptr
#include "SilenceDetector.h" // Include the new SilenceDetector class

/**
 * @file ControlPanel.h
 * @brief Defines the ControlPanel class, which manages the application's UI controls, and the custom LoopButton.
 */

/**
 * @class LoopButton
 * @brief A custom button class designed to differentiate between left and right mouse clicks.
 *
 * This button is specifically used for the "Loop In" and "Loop Out" functionalities,
 * allowing a left-click to directly set a point and a right-click to enter a
 * placement mode for more precise interaction. It exposes `onLeftClick` and
 * `onRightClick` function objects for flexible callback assignment.
 */
class LoopButton : public juce::TextButton {
public:
    std::function<void()> onLeftClick;  ///< Function to call when the left mouse button is released over the button.
    std::function<void()> onRightClick; ///< Function to call when the right mouse button is released over the button.

    /**
     * @brief Constructs a LoopButton.
     * @param name The text to display on the button.
     */
    LoopButton (const juce::String& name = {}) : juce::TextButton (name) {}

private:
    /**
     * @brief Overrides `mouseUp` to detect left vs. right clicks and trigger custom callbacks.
     * @param event The mouse event details.
     */
    void mouseUp (const juce::MouseEvent& event) override {
        if (isEnabled()) {
            if (event.mods.isRightButtonDown()) {
                if (onRightClick) onRightClick();
            } else if (event.mods.isLeftButtonDown()) {
                if (onLeftClick) onLeftClick();
            }
        }
        juce::TextButton::mouseUp(event); // Call base class method
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoopButton)
};

/**
 * @class ControlPanel
 * @brief A component that manages all UI controls and interactions for the Sorta++ application.
 *
 * This class acts as the central hub for the application's graphical user interface,
 * encapsulating all buttons, text editors, and visual displays (excluding the waveform
 * rendering itself, which is handled by `MainComponent`). It manages their layout,
 * appearance, and state, delegating complex logic and audio operations back to the
 * `MainComponent` and `AudioPlayer`. It also integrates a `SilenceDetector` for
 * automatic loop point setting.
 */
class ControlPanel : public juce::Component,
                     public juce::TextEditor::Listener
{
public:
    //==============================================================================
    /** @name Constructors and Destructors
     *  @{
     */

    /**
     * @brief Constructs the ControlPanel.
     *
     * Initializes all UI components, sets up their properties, registers listeners,
     * and applies the custom `ModernLookAndFeel`.
     * @param owner A reference to the `MainComponent` that owns this panel. This reference
     *              is crucial for communicating user actions and updating global application state.
     */
    explicit ControlPanel(MainComponent& owner);

    /**
     * @brief Destructor.
     *
     * Ensures all child components are properly destroyed and listeners are unregistered.
     */
    ~ControlPanel() override;

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
     * This method handles drawing the background of the control panel itself and
     * ensures all child components are drawn. It also draws dynamic elements
     * such as the mouse cursor feedback and loop point visualization.
     * @param g The graphics context to draw into.
     */
    void paint(juce::Graphics& g) override;

    /**
     * @brief Called when the component's size changes.
     *
     * This method recalculates the layout of all child components and internal
     * drawing bounds (like `waveformBounds`) to adapt to new dimensions.
     */
    void resized() override;
    
    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name State Update Methods
     *  Methods for updating the UI based on application state changes.
     *  @{
     */

    /**
     * @brief Updates the text of the play/stop button.
     * @param isPlaying True if audio is currently playing, false if stopped.
     */
    void updatePlayButtonText(bool isPlaying);

    /**
     * @brief Updates the text in the `loopInEditor` and `loopOutEditor` based on current loop positions.
     */
    void updateLoopLabels();

    /**
     * @brief Updates the enabled/disabled and visible states of all interactive UI components.
     *
     * This ensures that buttons and editors are only interactive when appropriate
     * (e.g., play button disabled if no file loaded).
     */
    void updateComponentStates();

    /**
     * @brief Updates the colors of the `loopInButton` and `loopOutButton` based on the current `PlacementMode`.
     *
     * This provides visual feedback to the user about which loop point, if any, is
     * currently being set interactively.
     */
    void updateLoopButtonColors();

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name Playback & Looping State Accessors and Mutators
     *  @{
     */

    /** @brief Checks if looping is enabled.
     *  @return True if the loop feature is active, false otherwise.
     */
    bool getShouldLoop() const { return shouldLoop; }

    /** @brief Gets the current loop-in position.
     *  @return The loop-in position in seconds.
     */
    double getLoopInPosition() const { return loopInPosition; }

    /** @brief Gets the current loop-out position.
     *  @return The loop-out position in seconds.
     */
    double getLoopOutPosition() const { return loopOutPosition; }

    /**
     * @brief Sets the loop-in position.
     * @param pos The new loop-in position in seconds.
     */
    void setLoopInPosition(double pos) { loopInPosition = pos; }

    /**
     * @brief Sets the loop-out position.
     * @param pos The new loop-out position in seconds.
     */
    void setLoopOutPosition(double pos) { loopOutPosition = pos; }

    /**
     * @brief Ensures that `loopInPosition` is logically before or at `loopOutPosition`.
     *
     * If `loopInPosition` is greater than `loopOutPosition`, they are swapped.
     */
    void ensureLoopOrder();

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name UI Action Triggers
     *  Methods that trigger specific actions or toggle UI states.
     *  @{
     */

    /** @brief Toggles the visibility of the statistics display. */
    void toggleStats();

    /** @brief Triggers the action associated with the quality button, cycling through thumbnail quality modes. */
    void triggerQualityButton();

    /** @brief Triggers the action associated with the mode button, cycling through view modes (Classic/Overlay). */
    void triggerModeButton();
    
    /** @brief Triggers the action associated with the channel view button, cycling through channel display modes. */
    void triggerChannelViewButton();

    /** @brief Triggers the action associated with the main loop button, toggling global looping on/off. */
    void triggerLoopButton();
    
    /** @brief Programmatically "clicks" the clear loop in button to reset the loop-in point. */
    void clearLoopIn();

    /** @brief Programmatically "clicks" the clear loop out button to reset the loop-out point. */
    void clearLoopOut();

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name Stats Display Control
     *  Methods for managing the statistics display.
     *  @{
     */

    /**
     * @brief Sets the visibility of the statistics display.
     * @param shouldShowStats True to make the stats display visible, false to hide it.
     */
    void setShouldShowStats(bool shouldShowStats);

    /**
     * @brief Sets the static total time string in the stats display.
     * @param timeString The formatted string representing the total duration of the audio.
     */
    void setTotalTimeStaticString(const juce::String& timeString);

    /**
     * @brief Sets the text content of the stats display box with an optional custom color.
     * @param text The string to display in the stats panel.
     * @param color The color of the displayed text. Defaults to `Config::statsDisplayTextColour`.
     */
    void setStatsDisplayText(const juce::String& text, juce::Colour color = Config::statsDisplayTextColour);

    /**
     * @brief Updates the stats display with dynamically changing audio statistics.
     * @param statsText The formatted string containing real-time audio statistics (e.g., current position).
     */
    void updateStatsDisplay(const juce::String& statsText);

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name Placement Mode & Autoplay Status
     *  @{
     */

    /** @brief Gets the current `PlacementMode` of the application.
     *  @return The current `AppEnums::PlacementMode`.
     */
    AppEnums::PlacementMode getPlacementMode() const { return currentPlacementMode; }

    /**
     * @brief Sets the current `PlacementMode` of the application.
     * @param mode The new `AppEnums::PlacementMode` to set.
     */
    void setPlacementMode(AppEnums::PlacementMode mode) { currentPlacementMode = mode; }
    
    /** @brief Returns whether autoplay is currently enabled.
     *  @return True if autoplay is enabled, false otherwise.
     */
    bool shouldAutoplay() const { return m_shouldAutoplay; }

    /** @brief Returns whether "Cut Mode" is currently active.
     *  @return True if Cut Mode is active, false otherwise.
     */
    bool isCutModeActive() const { return m_isCutModeActive; }

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name Component Accessors
     *  Getters for internal components and properties.
     *  @{
     */
    
    /** @brief Gets the calculated bounds of the waveform display area within the control panel.
     *  @return A `juce::Rectangle<int>` representing the waveform display area.
     */
    juce::Rectangle<int> getWaveformBounds() const { return waveformBounds; }

    /** @brief Provides access to the `AudioPlayer` instance owned by `MainComponent`.
     *  @return A reference to the `AudioPlayer` object.
     */
    AudioPlayer& getAudioPlayer() const;

    /** @brief Provides access to the `statsDisplay` TextEditor.
     *  @return A reference to the `juce::TextEditor` used for displaying statistics.
     */
    juce::TextEditor& getStatsDisplay() { return statsDisplay; }

    /**
     * @brief Sets the loop-in position using a sample index.
     *
     * This method converts the sample index to a time in seconds and updates
     * the `loopInPosition` and its corresponding UI editor.
     * @param sampleIndex The sample index to set as the loop-in point.
     */
    void setLoopStart(int sampleIndex);

    /**
     * @brief Sets the loop-out position using a sample index.
     *
     * This method converts the sample index to a time in seconds and updates
     * the `loopOutPosition` and its corresponding UI editor.
     * @param sampleIndex The sample index to set as the loop-out point.
     */
    void setLoopEnd(int sampleIndex);

    /**
     * @brief Formats a time in seconds into a human-readable string (HH:MM:SS:mmm).
     * @param seconds The time in seconds to format.
     * @return A formatted `juce::String`.
     */
    juce::String formatTime(double seconds) const;

    /** @brief Provides access to the custom `ModernLookAndFeel` instance.
     *  @return A const reference to the `juce::LookAndFeel` (specifically `ModernLookAndFeel`).
     */
    const juce::LookAndFeel& getLookAndFeel() const;

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name juce::MouseListener Overrides
     *  Methods for handling mouse interaction events over the control panel.
     *  @{
     */

    /**
     * @brief Handles mouse movement events.
     *
     * Updates the internal mouse cursor position and triggers a repaint to
     * provide visual feedback (e.g., drawing the mouse line and time display).
     * @param event The mouse event details.
     */
    void mouseMove(const juce::MouseEvent& event) override;

    /**
     * @brief Handles mouse down events.
     *
     * Initiates dragging for seeking playback, or handles right-click events
     * for entering loop point placement mode.
     * @param event The mouse event details.
     */
    void mouseDown(const juce::MouseEvent& event) override;

    /**
     * @brief Handles mouse drag events.
     *
     * Updates the playback position if a seeking drag operation is active.
     * @param event The mouse event details.
     */
    void mouseDrag(const juce::MouseEvent& event) override;

    /**
     * @brief Handles mouse up events.
     *
     * Stops any active dragging operation and finalizes a seek operation.
     * Also handles left-click events for directly seeking to a position or
     * placing a loop point if in placement mode.
     * @param event The mouse event details.
     */
    void mouseUp(const juce::MouseEvent& event) override;

    /**
     * @brief Handles mouse exit events from the component.
     *
     * Resets the internal mouse cursor position to hide any visual feedback
     * when the mouse leaves the control panel area.
     * @param event The mouse event details.
     */
    void mouseExit(const juce::MouseEvent& event) override;

    /** @} */
    //==============================================================================

private:
    //==============================================================================
    /** @name juce::TextEditor::Listener Overrides (Private)
     *  Callbacks for handling text editor events.
     *  @{
     */

    /**
     * @brief Callback for when a listened-to TextEditor's text changes.
     *
     * Used for real-time validation or preview updates as the user types.
     * @param editor The TextEditor that triggered the event.
     */
    void textEditorTextChanged(juce::TextEditor& editor) override;

    /**
     * @brief Callback for when the return key is pressed in a TextEditor.
     *
     * Typically used to finalize input from the user.
     * @param editor The TextEditor that triggered the event.
     */
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;

    /**
     * @brief Callback for when the escape key is pressed in a TextEditor.
     *
     * Often used to cancel input or revert to a previous value.
     * @param editor The TextEditor that triggered the event.
     */
    void textEditorEscapeKeyPressed(juce::TextEditor& editor) override;

    /**
     * @brief Callback for when a TextEditor loses focus.
     *
     * Typically used to finalize input, similar to the return key being pressed.
     * @param editor The TextEditor that triggered the event.
     */
    void textEditorFocusLost(juce::TextEditor& editor) override;

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name Private Member Variables
     *  Internal components and state of the ControlPanel.
     *  @{
     */

    MainComponent& owner;                       ///< A reference to the owning `MainComponent` for inter-component communication.
    ModernLookAndFeel modernLF;                 ///< Custom look and feel instance for UI styling.
    std::unique_ptr<SilenceDetector> silenceDetector; ///< Manages silence detection logic and its UI.

    // --- UI Components ---
    juce::TextButton openButton, playStopButton, modeButton, exitButton, statsButton, loopButton, channelViewButton, qualityButton; ///< Standard TextButtons for various actions.
    juce::TextButton clearLoopInButton, clearLoopOutButton;                                                                      ///< Small buttons to clear specific loop points.
    juce::TextEditor statsDisplay, loopInEditor, loopOutEditor;                                                                   ///< TextEditors for displaying statistics and editing loop points.
    LoopButton loopInButton, loopOutButton;                                                                                       ///< Custom buttons for 'in' and 'out' loop point setting, with distinct left/right click behavior.
    juce::TextButton autoplayButton, autoCutInButton, autoCutOutButton, cutButton;                                                ///< Buttons for automation features.

    // --- Layout ---
    juce::Rectangle<int> waveformBounds;        ///< The calculated area within the control panel reserved for the waveform display.
    juce::Rectangle<int> statsBounds;           ///< The calculated area for the statistics display.
    juce::Rectangle<int> contentAreaBounds;     ///< The main content area excluding borders.

    // --- State ---
    AppEnums::ViewMode currentMode = AppEnums::ViewMode::Classic;           ///< The currently active view mode (e.g., Classic, Overlay).
    AppEnums::ChannelViewMode currentChannelViewMode = AppEnums::ChannelViewMode::Mono; ///< The currently selected channel view mode.
    AppEnums::ThumbnailQuality currentQuality = AppEnums::ThumbnailQuality::Low; ///< The currently selected waveform thumbnail quality.
    AppEnums::PlacementMode currentPlacementMode = AppEnums::PlacementMode::None; ///< Tracks if the user is in a mode to place a loop point.
    
    bool showStats = false;                     ///< Flag indicating if the statistics display is currently visible.
    bool shouldLoop = false;                    ///< Flag indicating if audio playback should loop.
    double loopInPosition = -1.0;               ///< The start time of the loop region in seconds (-1.0 means unset).
    double loopOutPosition = -1.0;              ///< The end time of the loop region in seconds (-1.0 means unset).

    int mouseCursorX = -1, mouseCursorY = -1;   ///< Current mouse cursor coordinates relative to this component.
    double mouseCursorTime = 0.0;               ///< Time in seconds corresponding to `mouseCursorX` over the waveform.
    bool isDragging = false;                    ///< True if the mouse is currently dragging to seek.
    double currentPlaybackPosOnDragStart = 0.0; ///< Playback position when a drag operation began.
    int mouseDragStartX = 0;                    ///< X-coordinate where a mouse drag operation began.

    int bottomRowTopY = 0;                      ///< Y-coordinate for the top edge of the bottom row of controls.
    int playbackLeftTextX = 0, playbackRightTextX = 0, playbackCenterTextX = 0; ///< X-coordinates for various playback time display positions.
    
    juce::String totalTimeStaticStr;            ///< Stores the formatted total duration of the loaded audio.
    juce::String loopInDisplayString, loopOutDisplayString; ///< Formatted strings for loop in/out display.
    int loopInTextX = 0, loopOutTextX = 0, loopTextY = 0;   ///< Coordinates for loop point text displays.

    bool m_shouldAutoplay = false;              ///< Flag indicating if autoplay is currently enabled.
    float glowAlpha = 0.0f;                     ///< Alpha value for animation effects (e.g., pulsing lines).
    bool m_isCutModeActive = false;             ///< Flag indicating if Cut Mode is currently active.

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name Private Helper Methods - Mouse Interaction
     *  @{
     */

    /**
     * @brief Handles right-click events specifically for entering loop point placement modes.
     * @param x The x-coordinate of the mouse click relative to the component.
     */
    void handleRightClickForLoopPlacement(int x);

    /**
     * @brief Seeks the audio player to the position corresponding to the given x-coordinate.
     *
     * This method translates a UI pixel coordinate within the waveform display area
     * into a time in seconds and instructs the `AudioPlayer` to seek to that position.
     * @param x The x-coordinate of the mouse position relative to the component.
     */
    void seekToMousePosition(int x);

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name Private Helper Methods - Initialization
     *  @{
     */

    /** @brief Initializes the custom `ModernLookAndFeel` for this component. */
    void initialiseLookAndFeel();
    /** @brief Initializes all `juce::TextButton` instances and adds them as child components. */
    void initialiseButtons();
    /** @brief Initializes the "Open Directory" button (`openButton`). */
    void initialiseOpenButton();
    /** @brief Initializes the "Play/Stop" button (`playStopButton`). */
    void initialisePlayStopButton();
    /** @brief Initializes the "View Mode" button (`modeButton`). */
    void initialiseModeButton();
    /** @brief Initializes the "Channel View" button (`channelViewButton`). */
    void initialiseChannelViewButton();
    /** @brief Initializes the "Thumbnail Quality" button (`qualityButton`). */
    void initialiseQualityButton();
    /** @brief Initializes the "Exit" button (`exitButton`). */
    void initialiseExitButton();
    /** @brief Initializes the "Stats" button (`statsButton`). */
    void initialiseStatsButton();
    /** @brief Initializes the "Loop" toggle button (`loopButton`). */
    void initialiseLoopButton();
    /** @brief Initializes the "Autoplay" toggle button (`autoplayButton`). */
    void initialiseAutoplayButton();
    /** @brief Initializes the "Auto Cut In" toggle button (`autoCutInButton`). */
    void initialiseAutoCutInButton();
    /** @brief Initializes the "Auto Cut Out" toggle button (`autoCutOutButton`). */
    void initialiseAutoCutOutButton();
    /** @brief Initializes the "Cut Mode" toggle button (`cutButton`). */
    void initialiseCutButton();
    /** @brief Initializes the custom `LoopButton` instances (`loopInButton`, `loopOutButton`). */
    void initialiseLoopButtons();
    /** @brief Initializes the "Clear Loop In" and "Clear Loop Out" buttons (`clearLoopInButton`, `clearLoopOutButton`). */
    void initialiseClearButtons();
    /** @brief Initializes the `juce::TextEditor` instances for loop point display (`loopInEditor`, `loopOutEditor`). */
    void initialiseLoopEditors();
    /** @brief Performs final setup steps after all components are initialized. */
    void finaliseSetup();

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name Private Helper Methods - Layout
     *  @{
     */

    /**
     * @brief Lays out the buttons in the top row of the control panel.
     * @param bounds The current available bounds for layout.
     * @param rowHeight The height allocated for this row.
     */
    void layoutTopRowButtons(juce::Rectangle<int>& bounds, int rowHeight);

    /**
     * @brief Lays out the loop and cut control buttons and editors.
     * @param bounds The current available bounds for layout.
     * @param rowHeight The height allocated for this row.
     */
    void layoutLoopAndCutControls(juce::Rectangle<int>& bounds, int rowHeight);

    /**
     * @brief Lays out buttons and text displays in the bottom row.
     * @param bounds The current available bounds for layout.
     * @param rowHeight The height allocated for this row.
     */
    void layoutBottomRowAndTextDisplay(juce::Rectangle<int>& bounds, int rowHeight);

    /**
     * @brief Calculates and sets the bounds for the waveform and statistics display areas.
     * @param bounds The current available bounds for layout.
     */
    void layoutWaveformAndStats(juce::Rectangle<int>& bounds);

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name Private Helper Methods - State Updates
     *  @{
     */

    /**
     * @brief Updates the enabled state of general buttons based on global conditions (e.g., audio loaded).
     * @param enabled True to enable the buttons, false to disable.
     */
    void updateGeneralButtonStates(bool enabled);

    /**
     * @brief Updates the enabled and visible states of controls related to "Cut Mode".
     * @param isCutModeActive True if cut mode is active.
     * @param enabled True to enable the buttons, false to disable.
     */
    void updateCutModeControlStates(bool isCutModeActive, bool enabled);

    /** @brief Updates the text displayed on the quality button based on the `currentQuality` setting. */
    void updateQualityButtonText();

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name Private Helper Methods - Drawing
     *  @{
     */

    /**
     * @brief Draws a reduced quality version of the waveform for a given channel.
     * @param g The graphics context to draw into.
     * @param channel The audio channel to draw.
     * @param pixelsPerSample The number of pixels representing each sample.
     */
    void drawReducedQualityWaveform(juce::Graphics& g, int channel, int pixelsPerSample);

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name Private Helper Methods - Utilities
     *  @{
     */

    /**
     * @brief Parses a time string (e.g., "00:01:23.456") into a double representing seconds.
     * @param timeString The string to parse.
     * @return The time in seconds, or 0.0 if parsing fails.
     */
    double parseTime(const juce::String& timeString);

    /** @} */
    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlPanel)
};