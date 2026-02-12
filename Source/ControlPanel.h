#pragma once

#include <JuceHeader.h>
#include <tuple>
#include "Config.h"
#include "ModernLookAndFeel.h" // Added include for ModernLookAndFeel
#include "AppEnums.h"
#include "AudioPlayer.h" // Added for AudioPlayer type recognition
#include <memory>    // Required for std::unique_ptr
#include "SilenceDetector.h" // Include the new SilenceDetector class
#include "MouseHandler.h" // Include the new MouseHandler class
#include "ControlPanelLayoutCache.h"
#include "LoopButton.h"

class MainComponent; // Forward declaration
class LayoutManager;
class WaveformRenderer;
class StatsPresenter;
class LoopPresenter;
class ControlStatePresenter;
class TransportPresenter;
class SilenceDetectionPresenter;
class ControlButtonsPresenter;
class LoopEditorPresenter;
class LoopResetPresenter;
class LoopButtonPresenter;
class PlaybackTextPresenter;

/**
 * @file ControlPanel.h
 * @brief Defines the ControlPanel class, which manages the application's UI controls.
 */

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
class ControlPanel : public juce::Component
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
    /** @name Zoom Popup State
     *  @{
     */
    enum class ActiveZoomPoint { None, In, Out };
    ActiveZoomPoint getActiveZoomPoint() const { return m_activeZoomPoint; }
    void setActiveZoomPoint(ActiveZoomPoint point) { m_activeZoomPoint = point; repaint(); }

    float getZoomFactor() const { return m_zoomFactor; }
    void setZoomFactor(float factor) { m_zoomFactor = juce::jlimit(1.0f, 1000000.0f, factor); repaint(); }

    bool isZKeyDown() const { return m_isZKeyDown; }
    void setZKeyDown(bool isDown);

    juce::Rectangle<int> getZoomPopupBounds() const { return m_zoomPopupBounds; }
    void setZoomPopupBounds(juce::Rectangle<int> bounds) { m_zoomPopupBounds = bounds; }

    std::pair<double, double> getZoomTimeRange() const { return m_zoomTimeRange; }
    void setZoomTimeRange(double start, double end) { m_zoomTimeRange = {start, end}; }

    void jumpToLoopIn();
    void setNeedsJumpToLoopIn(bool needs) { m_needsJumpToLoopIn = needs; }
    void performDelayedJumpIfNeeded();
    /** @} */

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

    /**
     * @brief Sets whether audio playback should loop.
     * @param shouldLoopParam True to enable looping, false to disable.
     */
    void setShouldLoop(bool shouldLoopParam);

    /** @brief Gets the current loop-in position.
     *  @return The loop-in position in seconds.
     */
    double getLoopInPosition() const;

    /** @brief Gets the current loop-out position.
     *  @return The loop-out position in seconds.
     */
    double getLoopOutPosition() const;

    /**
     * @brief Sets the loop-in position.
     * @param pos The new loop-in position in seconds.
     */
    void setLoopInPosition(double pos);

    /**
     * @brief Sets the loop-out position.
     * @param pos The new loop-out position in seconds.
     */
    void setLoopOutPosition(double pos);

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
     * @brief Pulls the latest audio statistics via the StatsPresenter and updates the display.
     */
    void updateStatsFromAudio();

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name Placement Mode & Autoplay Status
     *  @{
     */

    /** @brief Gets the current `PlacementMode` of the application.
     *  @return The current `AppEnums::PlacementMode`.
     */
    AppEnums::PlacementMode getPlacementMode() const;
    
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
    juce::Rectangle<int> getWaveformBounds() const { return layoutCache.waveformBounds; }

    /** @brief Provides access to the `AudioPlayer` instance owned by `MainComponent`.
     *  @return A reference to the `AudioPlayer` object.
     */
    AudioPlayer& getAudioPlayer() const;

    /**
     * @brief Retrieves the current waveform thumbnail quality setting.
     * @return The active `AppEnums::ThumbnailQuality` value.
     */
    AppEnums::ThumbnailQuality getCurrentQualitySetting() const { return currentQuality; }

    /**
     * @brief Retrieves the current channel view mode (Mono/Stereo).
     * @return The active `AppEnums::ChannelViewMode` value.
     */
    AppEnums::ChannelViewMode getChannelViewMode() const { return currentChannelViewMode; }

    /**
     * @brief Provides read-only access to the current glow alpha for pulsing effects.
     * @return Alpha value between 0 and 1.
     */
    float getGlowAlpha() const { return glowAlpha; }

    /**
     * @brief Provides read-only access to the mouse handler for rendering.
     * @return Reference to the owned `MouseHandler`.
     */
    const MouseHandler& getMouseHandler() const { return *mouseHandler; }

    /**
     * @brief Provides access to the silence detector for threshold rendering.
     * @return Reference to the owned `SilenceDetector`.
     */
    SilenceDetector& getSilenceDetector() { return *silenceDetector; }
    const SilenceDetector& getSilenceDetector() const { return *silenceDetector; }

    /**
     * @brief Gets the cached Y coordinate for the bottom row's top.
     * @return Y coordinate in pixels.
     */
    int getBottomRowTopY() const { return layoutCache.bottomRowTopY; }

    /**
     * @brief Retrieves the cached playback label X positions (left, centre, right).
     * @return Tuple of left/centre/right X coordinates.
     */
    std::tuple<int, int, int> getPlaybackLabelXs() const { return { layoutCache.playbackLeftTextX, layoutCache.playbackCenterTextX, layoutCache.playbackRightTextX }; }

    /** @brief Provides access to the stats TextEditor managed by `StatsPresenter`.
     *  @return A reference to the `juce::TextEditor` used for displaying statistics.
     */
    juce::TextEditor& getStatsDisplay();

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
     * Forwards the event to the owned `MouseHandler` instance.
     * @param event The mouse event details.
     */
    void mouseMove(const juce::MouseEvent& event) override;

    /**
     * @brief Handles mouse down events.
     *
     * Forwards the event to the owned `MouseHandler` instance after
     * handling text editor focus.
     * @param event The mouse event details.
     */
    void mouseDown(const juce::MouseEvent& event) override;

    /**
     * @brief Handles mouse drag events.
     *
     * Forwards the event to the owned `MouseHandler` instance.
     * @param event The mouse event details.
     */
    void mouseDrag(const juce::MouseEvent& event) override;

    /**
     * @brief Handles mouse up events.
     *
     * Forwards the event to the owned `MouseHandler` instance.
     * @param event The mouse event details.
     */
    void mouseUp(const juce::MouseEvent& event) override;

    /**
     * @brief Handles mouse exit events from the component.
     *
     * Forwards the event to the owned `MouseHandler` instance.
     * @param event The mouse event details.
     */
    void mouseExit(const juce::MouseEvent& event) override;

    /**
     * @brief Handles mouse wheel events.
     *
     * Forwards the event to the owned `MouseHandler` instance.
     * @param event The mouse event details.
     * @param wheel The mouse wheel details.
     */
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

    /** @} */
    //==============================================================================

private:
    friend class LayoutManager;
    friend class ControlStatePresenter;
    friend class TransportPresenter;
    friend class SilenceDetectionPresenter;
    friend class ControlButtonsPresenter;
    friend class LoopEditorPresenter;
    friend class LoopButtonPresenter;
    friend class LoopResetPresenter;
    friend class PlaybackTextPresenter;

    //==============================================================================
    /** @name juce::TextEditor::Listener Overrides (Private)
     *  Callbacks for handling text editor events.
     *  @{
     */

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
    std::unique_ptr<MouseHandler> mouseHandler;     ///< Manages all mouse interaction logic.
    std::unique_ptr<LayoutManager> layoutManager;   ///< Extracted helper that owns layout calculations.
    std::unique_ptr<WaveformRenderer> waveformRenderer; ///< Handles waveform and overlay painting.
    std::unique_ptr<PlaybackTextPresenter> playbackTextPresenter; ///< Draws playback time labels under the waveform.
    std::unique_ptr<StatsPresenter> statsPresenter; ///< Handles stats building, layout, and presentation.
    std::unique_ptr<LoopPresenter> loopPresenter;   ///< Owns the loop editors and loop position logic.
    std::unique_ptr<ControlStatePresenter> controlStatePresenter; ///< Centralises component enable/visibility logic.
    std::unique_ptr<TransportPresenter> transportPresenter; ///< Handles loop/autoplay/cut button behaviour.
    std::unique_ptr<SilenceDetectionPresenter> silenceDetectionPresenter; ///< Owns auto-cut toggle behaviour.
    std::unique_ptr<ControlButtonsPresenter> buttonPresenter; ///< Handles button initialization.
    std::unique_ptr<LoopEditorPresenter> loopEditorPresenter; ///< Manages loop editor setup and validation.
    std::unique_ptr<LoopButtonPresenter> loopButtonPresenter; ///< Handles loop button colouring.
    std::unique_ptr<LoopResetPresenter> loopResetPresenter; ///< Clears loop bounds.

    // --- UI Components ---
    juce::TextButton openButton, playStopButton, modeButton, exitButton, statsButton, loopButton, channelViewButton, qualityButton; ///< Standard TextButtons for various actions.
    juce::TextButton clearLoopInButton, clearLoopOutButton;                                                                      ///< Small buttons to clear specific loop points.
    juce::TextEditor loopInEditor, loopOutEditor;                                                                                ///< TextEditors for displaying statistics and editing loop points.
    juce::TextEditor elapsedTimeEditor, remainingTimeEditor, loopLengthEditor;                                                   ///< Editors for playback and loop length.
    LoopButton loopInButton, loopOutButton;                                                                                       ///< Custom buttons for 'in' and 'out' loop point setting, with distinct left/right click behavior.
    juce::TextButton autoplayButton, autoCutInButton, autoCutOutButton, cutButton;                                                ///< Buttons for automation features.

    // --- Layout ---
    ControlPanelLayoutCache layoutCache;

    // --- State ---
    AppEnums::ViewMode currentMode = AppEnums::ViewMode::Classic;           ///< The currently active view mode (e.g., Classic, Overlay).
    AppEnums::ChannelViewMode currentChannelViewMode = AppEnums::ChannelViewMode::Mono; ///< The currently selected channel view mode.
    AppEnums::ThumbnailQuality currentQuality = AppEnums::ThumbnailQuality::Low; ///< The currently selected waveform thumbnail quality.
    
    bool shouldLoop = false;                    ///< Flag indicating if audio playback should loop.

    juce::String loopInDisplayString, loopOutDisplayString; ///< Formatted strings for loop in/out display.
    int loopInTextX = 0, loopOutTextX = 0, loopTextY = 0;   ///< Coordinates for loop point text displays.

    bool m_shouldAutoplay = false;              ///< Flag indicating if autoplay is currently enabled.
    float glowAlpha = 0.0f;                     ///< Alpha value for animation effects (e.g., pulsing lines).
    bool m_isCutModeActive = false;             ///< Flag indicating if Cut Mode is currently active.
    ActiveZoomPoint m_activeZoomPoint = ActiveZoomPoint::None; ///< Currently zoomed loop point.
    float m_zoomFactor = 10.0f;                 ///< Dynamic zoom factor.
    bool m_isZKeyDown = false;                  ///< State of the 'z' key.
    bool m_needsJumpToLoopIn = false;           ///< Flag for delayed playback jump.
    juce::Rectangle<int> m_zoomPopupBounds;     ///< Cached bounds of the zoom popup.
    std::pair<double, double> m_zoomTimeRange;  ///< Cached time range of the zoom popup.

    /** @} */
    //==============================================================================

    //==============================================================================

    //==============================================================================

    //==============================================================================
    /** @name Private Helper Methods - Initialization
     *  @{
     */

    /** @brief Initializes the custom `ModernLookAndFeel` for this component. */
    void initialiseLookAndFeel();
    /** @brief Initializes the `juce::TextEditor` instances for loop point display (`loopInEditor`, `loopOutEditor`). */
    void initialiseLoopEditors();
    void invokeOwnerOpenDialog();
    /** @brief Performs final setup steps after all components are initialized. */
    void finaliseSetup();

    /** @} */
    //==============================================================================

    //==============================================================================
    //==============================================================================
    /** @name Private Helper Methods - State Updates
     *  @{
     */

    /** @brief Updates the text displayed on the quality button based on the `currentQuality` setting. */
    void updateQualityButtonText();

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name Private Helper Methods - Drawing
     *  @{
     */

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name Private Helper Methods - Utilities
     *  @{
     */

    /** @} */
    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlPanel)
};
