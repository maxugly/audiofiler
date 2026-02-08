#pragma once

#include <JuceHeader.h>
#include "Config.h"
#include "ModernLookAndFeel.h"
#include "AppEnums.h"
#include "AudioPlayer.h" // Added for AudioPlayer type recognition

class MainComponent; // Forward declaration

#include <memory> // Required for std::unique_ptr
#include "SilenceDetector.h" // Include the new SilenceDetector class

/**
 * @brief A custom button class to handle left and right clicks differently.
 *
 * Used for the loop in/out buttons to allow setting the point with a left-click
 * and entering placement mode with a right-click.
 */
class LoopButton : public juce::TextButton {
public:
    std::function<void()> onLeftClick;
    std::function<void()> onRightClick;
    LoopButton (const juce::String& name = {}) : juce::TextButton (name) {}

private:
    void mouseUp (const juce::MouseEvent& event) override {
        if (isEnabled()) {
            if (event.mods.isRightButtonDown()) {
                if (onRightClick) onRightClick();
            } else if (event.mods.isLeftButtonDown()) {
                if (onLeftClick) onLeftClick();
            }
        }
        juce::TextButton::mouseUp(event);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoopButton)
};

/**
 * @brief A component that manages all UI controls for the Sorta++ application.
 *
 * This class encapsulates all buttons, editors, labels, and their associated
 * layout and state management logic. It is owned by MainComponent and handles
 * all visual aspects of the application, delegating user actions back to
 * the MainComponent where necessary.
 */
class ControlPanel : public juce::Component,
                     public juce::TextEditor::Listener
{
public:
    /**
     * @brief Constructs the ControlPanel.
     * @param owner A reference to the MainComponent that owns this panel.
     */
    explicit ControlPanel(MainComponent& owner);

    /**
     * @brief Destructor.
     */
    ~ControlPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    /**
     * @brief Updates the text of the play/stop button based on playback state.
     * @param isPlaying True if audio is currently playing.
     */
    void updatePlayButtonText(bool isPlaying);

    /**
     * @brief Updates the loop time labels from the current loop positions.
     */
    void updateLoopLabels();

    /**
     * @brief Updates the enabled/disabled and visible states of all components.
     */
    void updateComponentStates();

    bool getShouldLoop() const { return shouldLoop; }

    /**
     * @brief Gets the current loop-in position.
     * @return The loop-in position in seconds.
     */
    double getLoopInPosition() const { return loopInPosition; }

    /**
     * @brief Gets the current loop-out position.
     * @return The loop-out position in seconds.
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
     * @brief Ensures loopInPosition is not after loopOutPosition, swapping if necessary.
     */
    void ensureLoopOrder();

    /**
     * @brief Toggles the stats display.
     */
    void toggleStats();

    /**
     * @brief Triggers the quality button's action.
     */
    void triggerQualityButton();

    /**
     * @brief Triggers the mode button's action.
     */
    void triggerModeButton();
    
    /**
     * @brief Triggers the channel view button's action.
     */
    void triggerChannelViewButton();

    /**
     * @brief Triggers the loop button's action.
     */
    void triggerLoopButton();
    
    /**
     * @brief Clicks the clear loop in button.
     */
    void clearLoopIn();

    /**
     * @brief Clicks the clear loop out button.
     */
    void clearLoopOut();

    /**
     * @brief Sets a flag to show or hide stats.
     * @param shouldShowStats A boolean to determine if stats are shown.
     */
    void setShouldShowStats(bool shouldShowStats);

    /**
     * @brief Sets the total time string to display.
     * @param timeString The formatted string for the total time.
     */
    void setTotalTimeStaticString(const juce::String& timeString);

    /**
     * @brief Sets the text in the stats display box.
     * @param text The text to display.
     * @param color The color of the text.
     */
    void setStatsDisplayText(const juce::String& text, juce::Colour color = Config::statsDisplayTextColour);

    /**
     * @brief Updates the stats display with dynamic audio statistics.
     * @param statsText The formatted string containing the dynamic statistics.
     */
    void updateStatsDisplay(const juce::String& statsText);

    /**
     * @brief Gets the current placement mode.
     */
    AppEnums::PlacementMode getPlacementMode() const { return currentPlacementMode; }

    /**
     * @brief Sets the current placement mode.
     */
    void setPlacementMode(AppEnums::PlacementMode mode) { currentPlacementMode = mode; }
    
    /**
     * @brief Returns whether autoplay is enabled.
     * @return True if autoplay is enabled, false otherwise.
     */
    bool shouldAutoplay() const { return m_shouldAutoplay; }


    
    /**
     * @brief Returns whether Cut Mode is currently active.
     * @return True if Cut Mode is active, false otherwise.
     */
    bool isCutModeActive() const { return m_isCutModeActive; }



    /**
     * @brief Updates the colors of the loop buttons based on the current placement mode.
     */
    void updateLoopButtonColors();
    
    /**
     * @brief Gets the bounds of the waveform display area.
     */
    juce::Rectangle<int> getWaveformBounds() const { return waveformBounds; }

    /**
     * @brief Provides access to the AudioPlayer instance.
     * @return A reference to the AudioPlayer.
     */
    AudioPlayer& getAudioPlayer() const;

    /**
     * @brief Provides access to the stats display TextEditor.
     * @return A reference to the stats display TextEditor.
     */
    juce::TextEditor& getStatsDisplay() { return statsDisplay; }

    /**
     * @brief Sets the loop-in position using sample index.
     * @param sampleIndex The sample index to set as the loop-in point.
     */
    void setLoopStart(int sampleIndex);

    /**
     * @brief Sets the loop-out position using sample index.
     * @param sampleIndex The sample index to set as the loop-out point.
     */
    void setLoopEnd(int sampleIndex);

    /**
     * @brief Formats a time in seconds into a human-readable string (HH:MM:SS:mmm).
     * @param seconds The time in seconds.
     * @return A formatted juce::String.
     */
    juce::String formatTime(double seconds) const;

    /**
     * @brief Provides access to the LookAndFeel for drawing.
     * @return A const reference to the LookAndFeel.
     */
    const juce::LookAndFeel& getLookAndFeel() const;

    /**
     * @brief Handles mouse movement events.
     *        Updates the mouse cursor position and triggers repaint for visual feedback.
     * @param event The mouse event details.
     */
    void mouseMove(const juce::MouseEvent& event) override;

    /**
     * @brief Handles mouse down events.
     *        Initiates dragging for seeking or handles right-click for loop placement.
     * @param event The mouse event details.
     */
    void mouseDown(const juce::MouseEvent& event) override;

    /**
     * @brief Handles mouse drag events.
     *        Updates playback position if dragging is active.
     * @param event The mouse event details.
     */
    void mouseDrag(const juce::MouseEvent& event) override;

    /**
     * @brief Handles mouse up events.
     *        Stops dragging and finalizes seek operation, or handles left-click for seeking.
     * @param event The mouse event details.
     */
    void mouseUp(const juce::MouseEvent& event) override;

    /**
     * @brief Handles mouse exit events from the component.
     *        Resets mouse cursor position to hide visual feedback.
     * @param event The mouse event details.
     */
    void mouseExit(const juce::MouseEvent& event) override;



private:
    /**
     * @brief Overrides from juce::TextEditor::Listener to handle text changes in editors.
     * @param editor The TextEditor that triggered the event.
     */
    void textEditorTextChanged(juce::TextEditor& editor) override;

    /**
     * @brief Overrides from juce::TextEditor::Listener to handle return key presses in editors.
     * @param editor The TextEditor that triggered the event.
     */
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;

    /**
     * @brief Overrides from juce::TextEditor::Listener to handle escape key presses in editors.
     * @param editor The TextEditor that triggered the event.
     */
    void textEditorEscapeKeyPressed(juce::TextEditor& editor) override;

    /**
     * @brief Overrides from juce::TextEditor::Listener to handle focus loss from editors.
     * @param editor The TextEditor that triggered the event.
     */
    void textEditorFocusLost(juce::TextEditor& editor) override;

    MainComponent& owner;
    ModernLookAndFeel modernLF;
    std::unique_ptr<SilenceDetector> silenceDetector; // Owned instance of SilenceDetector

    // UI Components
    juce::TextButton openButton, playStopButton, modeButton, exitButton, statsButton, loopButton, channelViewButton, qualityButton;
    juce::TextButton clearLoopInButton, clearLoopOutButton;
    juce::TextEditor statsDisplay, loopInEditor, loopOutEditor;
    LoopButton loopInButton, loopOutButton;
    juce::TextButton autoplayButton, autoCutInButton, autoCutOutButton, cutButton;

    // Layout
    juce::Rectangle<int> waveformBounds, statsBounds, contentAreaBounds;

    // State
    AppEnums::ViewMode currentMode = AppEnums::ViewMode::Classic;
    AppEnums::ChannelViewMode currentChannelViewMode = AppEnums::ChannelViewMode::Mono;
    AppEnums::ThumbnailQuality currentQuality = AppEnums::ThumbnailQuality::Low;
    AppEnums::PlacementMode currentPlacementMode = AppEnums::PlacementMode::None;
    
    bool showStats = false;
    bool shouldLoop = false;
    double loopInPosition = -1.0;
    double loopOutPosition = -1.0;

    int mouseCursorX = -1, mouseCursorY = -1;
    double mouseCursorTime = 0.0; // Time in seconds corresponding to mouseCursorX
    bool isDragging = false;
    double currentPlaybackPosOnDragStart = 0.0;
    int mouseDragStartX = 0;

    int bottomRowTopY = 0;
    int playbackLeftTextX = 0, playbackRightTextX = 0, playbackCenterTextX = 0;
    
    juce::String totalTimeStaticStr;
    juce::String loopInDisplayString, loopOutDisplayString;
    int loopInTextX = 0, loopOutTextX = 0, loopTextY = 0;

    bool m_shouldAutoplay = false; ///< @brief Flag indicating if autoplay is currently enabled.
    float glowAlpha = 0.0f;
    bool m_isCutModeActive = false; ///< @brief Flag indicating if Cut Mode is currently active.



    // Private helper methods for mouse interaction
    /**
     * @brief Handles right-click events for placing loop in/out points.
     * @param x The x-coordinate of the mouse click relative to the component.
     */
    void handleRightClickForLoopPlacement(int x);

    /**
     * @brief Seeks the audio player to the position corresponding to the given x-coordinate.
     * @param x The x-coordinate of the mouse position relative to the component.
     */
    void seekToMousePosition(int x);

    // Initialization
    void initialiseLookAndFeel();
    void initialiseButtons();
    void initialiseOpenButton();
    void initialisePlayStopButton();
    void initialiseModeButton();
    void initialiseChannelViewButton();
    void initialiseQualityButton();
    void initialiseExitButton();
    void initialiseStatsButton();
    void initialiseLoopButton();
    void initialiseAutoplayButton();
    void initialiseAutoCutInButton();
    void initialiseAutoCutOutButton();
    void initialiseCutButton();
    void initialiseLoopButtons();
    void initialiseClearButtons();
    void initialiseLoopEditors();
    void finaliseSetup();

    // Layout
    void layoutTopRowButtons(juce::Rectangle<int>& bounds, int rowHeight);
    void layoutLoopAndCutControls(juce::Rectangle<int>& bounds, int rowHeight);
    void layoutBottomRowAndTextDisplay(juce::Rectangle<int>& bounds, int rowHeight);
    void layoutWaveformAndStats(juce::Rectangle<int>& bounds);

    // State Updates
    void updateGeneralButtonStates(bool enabled);
    void updateCutModeControlStates(bool isCutModeActive, bool enabled);
    void updateQualityButtonText();

    // Drawing
    void drawReducedQualityWaveform(juce::Graphics& g, int channel, int pixelsPerSample);

    // Helpers
    double parseTime(const juce::String& timeString);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlPanel)
};