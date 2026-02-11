#pragma once

#include <JuceHeader.h>
#include "Config.h" // For access to Config::
#include "AppEnums.h" // For AppEnums::PlacementMode

class ControlPanel; // Forward declaration of ControlPanel

/**
 * @class MouseHandler
 * @brief Handles all mouse interaction logic for the ControlPanel, abstracting it
 *        from the main panel class to improve modularity and maintainability.
 *
 * This class inherits from `juce::MouseListener` and processes mouse events
 * such as movement, clicks, and drags within the context of the ControlPanel's
 * waveform display. It manages internal state related to mouse position, dragging,
 * and specific interaction modes (e.g., loop point placement).
 */
class MouseHandler : public juce::MouseListener
{
public:
    //==============================================================================
    /**
     * @brief Constructs a MouseHandler.
     * @param controlPanel The `ControlPanel` instance that owns this `MouseHandler`.
     *                     This reference is used to access shared state (e.g., AudioPlayer,
     *                     waveform bounds, loop positions) and trigger UI updates.
     */
    explicit MouseHandler(ControlPanel& controlPanel);

    /**
     * @brief Destructor.
     */
    ~MouseHandler() override = default;

    //==============================================================================
    /** @name juce::MouseListener Overrides
     *  Methods for handling mouse interaction events over the component.
     *  @{
     */

    /**
     * @brief Handles mouse movement events.
     *
     * Updates the internal mouse cursor position and corresponding time, and
     * triggers a repaint on the owning `ControlPanel` to provide visual feedback
     * (e.g., drawing the mouse line and time display).
     * @param event The mouse event details, including position and modifiers.
     */
    void mouseMove(const juce::MouseEvent& event) override;

    /**
     * @brief Handles mouse down events.
     *
     * Initiates dragging for seeking playback if the left button is pressed,
     * or handles right-click events for entering loop point placement mode.
     * It also manages keyboard focus for text editors.
     * @param event The mouse event details, including position and modifiers.
     */
    void mouseDown(const juce::MouseEvent& event) override;

    /**
     * @brief Handles mouse drag events.
     *
     * Continuously updates the audio playback position if a seeking drag
     * operation is active and within the waveform bounds.
     * @param event The mouse event details, including position, delta, and modifiers.
     */
    void mouseDrag(const juce::MouseEvent& event) override;

    /**
     * @brief Handles mouse up events.
     *
     * Stops any active dragging operation. Finalizes a seek operation or, if
     * in loop placement mode, sets the corresponding loop point based on the
     * mouse position. Resets the placement mode after interaction.
     * @param event The mouse event details, including position and modifiers.
     */
    void mouseUp(const juce::MouseEvent& event) override;

    /**
     * @brief Handles mouse exit events from the component's bounds.
     *
     * Resets the internal mouse cursor position state (`mouseCursorX`, `mouseCursorY`)
     * to indicate the mouse is no longer over the component, which typically
     * hides any visual feedback like cursor lines.
     * @param event The mouse event details.
     */
    void mouseExit(const juce::MouseEvent& event) override;

    /** @} */
    //==============================================================================

    //==============================================================================
    /**
     * @brief Retrieves the current X-coordinate of the mouse cursor relative to the component.
     * @return The X-coordinate, or -1 if the mouse is not over the component.
     */
    int getMouseCursorX() const { return mouseCursorX; }

    /**
     * @brief Retrieves the current Y-coordinate of the mouse cursor relative to the component.
     * @return The Y-coordinate, or -1 if the mouse is not over the component.
     */
    int getMouseCursorY() const { return mouseCursorY; }

    /**
     * @brief Retrieves the time in seconds corresponding to the current mouse cursor X position
     *        over the waveform.
     * @return The time in seconds, or 0.0 if no audio is loaded or mouse is not over waveform.
     */
    double getMouseCursorTime() const { return mouseCursorTime; }
    
    /**
     * @brief Retrieves the current placement mode.
     * @return The current `AppEnums::PlacementMode`.
     */
    AppEnums::PlacementMode getCurrentPlacementMode() const { return currentPlacementMode; }

    /**
     * @brief Sets the current placement mode.
     * @param mode The new `AppEnums::PlacementMode` to set.
     */
    void setCurrentPlacementMode(AppEnums::PlacementMode mode) { currentPlacementMode = mode; }

    /** @enum LoopMarkerHandle
     *  @brief Identifies which loop marker handle is being interacted with.
     */
    enum class LoopMarkerHandle {
        None,
        In,
        Out,
        Full
    };

    /** @brief Gets the handle currently under the mouse. */
    LoopMarkerHandle getHoveredHandle() const { return hoveredHandle; }

    /** @brief Gets the handle currently being dragged. */
    LoopMarkerHandle getDraggedHandle() const { return draggedHandle; }

private:
    //==============================================================================
    /** @name Private Member Variables
     *  Internal state variables for mouse interaction.
     *  @{
     */
    ControlPanel& owner;                        ///< Reference to the owning `ControlPanel`.
    int mouseCursorX = -1, mouseCursorY = -1;   ///< Current mouse cursor coordinates relative to the owner component.
    double mouseCursorTime = 0.0;               ///< Time in seconds corresponding to `mouseCursorX` over the waveform.
    bool isDragging = false;                    ///< True if the mouse is currently dragging for seeking.
    double currentPlaybackPosOnDragStart = 0.0; ///< Playback position when a drag operation began.
    int mouseDragStartX = 0;                    ///< X-coordinate where a mouse drag operation began.
    AppEnums::PlacementMode currentPlacementMode = AppEnums::PlacementMode::None; ///< Tracks if the user is in a mode to place a loop point.

    LoopMarkerHandle hoveredHandle = LoopMarkerHandle::None;
    LoopMarkerHandle draggedHandle = LoopMarkerHandle::None;
    double dragStartLoopLength = 0.0;
    double dragStartMouseOffset = 0.0;
    bool interactionStartedInZoom = false;

    /** @} */
    //==============================================================================

    //==============================================================================
    /** @name Private Helper Methods
     *  Internal utility methods for mouse event processing.
     *  @{
     */

    /**
     * @brief Identifies which loop marker handle is at a given pixel position.
     * @param pos The position relative to the ControlPanel.
     * @return The handle found, or LoopMarkerHandle::None.
     */
    LoopMarkerHandle getHandleAtPosition(juce::Point<int> pos) const;

    /**
     * @brief Handles right-click events specifically for entering loop point placement modes.
     *
     * This method translates the mouse's X-coordinate into a time, and if
     * in an appropriate placement mode, sets the corresponding loop point
     * in the `ControlPanel`. It also triggers UI updates.
     * @param x The x-coordinate of the mouse click relative to the component.
     */
    void handleRightClickForLoopPlacement(int x);

    /**
     * @brief Seeks the audio player to the position corresponding to the given x-coordinate.
     *
     * This method translates a UI pixel coordinate within the waveform display area
     * into a time in seconds and instructs the `AudioPlayer` (accessed via `ControlPanel`)
     * to seek to that position.
     * @param x The x-coordinate of the mouse position relative to the component.
     */
    void seekToMousePosition(int x);

    /**
     * @brief Removes keyboard focus from any focused `TextEditor` if the mouse click happens outside it.
     *
     * @param clickPosition Position of the mouse click, relative to the owning `ControlPanel`.
     */
    void clearTextEditorFocusIfNeeded(const juce::Point<int>& clickPosition);

    /** @} */
    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MouseHandler)
};
