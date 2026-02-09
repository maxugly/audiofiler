#include "MouseHandler.h"
#include "ControlPanel.h" // Full header required for ControlPanel access
#include "AudioPlayer.h"    // Required for AudioPlayer types

/**
 * @file MouseHandler.cpp
 * @brief Implements the MouseHandler class, abstracting mouse interaction logic from ControlPanel.
 */

/**
 * @brief Constructs a MouseHandler.
 * @param controlPanel The `ControlPanel` instance that owns this `MouseHandler`.
 *                     This reference is used to access shared state and trigger UI updates.
 */
MouseHandler::MouseHandler(ControlPanel& controlPanel) : owner(controlPanel)
{
}

/**
 * @brief Handles mouse movement events.
 *
 * Updates the internal mouse cursor position and corresponding time, and
 * triggers a repaint on the owning `ControlPanel` to provide visual feedback
 * (e.g., drawing the mouse line and time display).
 * @param event The mouse event details, including position and modifiers.
 */
void MouseHandler::mouseMove(const juce::MouseEvent& event)
{
    const auto waveformBounds = owner.getWaveformBounds();
    if (waveformBounds.contains(event.getPosition()))
    {
        mouseCursorX = event.x;
        mouseCursorY = event.y;
        AudioPlayer& audioPlayer = owner.getAudioPlayer();
        auto audioLength = audioPlayer.getThumbnail().getTotalLength();
        if (audioLength > 0.0)
        {
            float proportion = (float)(mouseCursorX - waveformBounds.getX()) / (float)waveformBounds.getWidth();
            mouseCursorTime = proportion * audioLength;
        }
        else
        {
            mouseCursorTime = 0.0;
        }
    }
    else
    {
        mouseCursorX = -1;
        mouseCursorY = -1;
        mouseCursorTime = 0.0;
    }
    owner.repaint(); // Trigger repaint on owner
}

/**
 * @brief Handles mouse down events.
 *
 * Initiates dragging for seeking playback if the left button is pressed,
 * or handles right-click events for entering loop point placement mode.
 * It also manages keyboard focus for text editors.
 * @param event The mouse event details, including position and modifiers.
 */
void MouseHandler::mouseDown(const juce::MouseEvent& event)
{
    // Why: Clicking on the waveform should defocus text fields so transport shortcuts keep working.
    clearTextEditorFocusIfNeeded(event.getPosition());

    const auto waveformBounds = owner.getWaveformBounds();
    if (! waveformBounds.contains(event.getPosition()))
        return;

    if (event.mods.isLeftButtonDown())
    {
        isDragging = true;
        mouseDragStartX = event.x;
        currentPlaybackPosOnDragStart = owner.getAudioPlayer().getTransportSource().getCurrentPosition();
        seekToMousePosition(event.x);
    }
    else if (event.mods.isRightButtonDown())
    {
        handleRightClickForLoopPlacement(event.x);
    }
}

/**
 * @brief Handles mouse drag events.
 *
 * Continuously updates the audio playback position if a seeking drag
 * operation is active and within the waveform bounds.
 * @param event The mouse event details, including position, delta, and modifiers.
 */
void MouseHandler::mouseDrag(const juce::MouseEvent& event)
{
    const auto waveformBounds = owner.getWaveformBounds();
    if (isDragging && event.mods.isLeftButtonDown() && waveformBounds.contains(event.getPosition()))
    {
        seekToMousePosition(event.x);
    }
}

/**
 * @brief Handles mouse up events.
 *
 * Stops any active dragging operation. Finalizes a seek operation or, if
 * in loop placement mode, sets the corresponding loop point based on the
 * mouse position. Resets the placement mode after interaction.
 * @param event The mouse event details, including position and modifiers.
 */
void MouseHandler::mouseUp(const juce::MouseEvent& event)
{
    isDragging = false;
    const auto waveformBounds = owner.getWaveformBounds();
    if (waveformBounds.contains(event.getPosition()) && event.mods.isLeftButtonDown())
    {
        if (currentPlacementMode != AppEnums::PlacementMode::None)
        {
            AudioPlayer& audioPlayer = owner.getAudioPlayer();
            auto audioLength = audioPlayer.getThumbnail().getTotalLength();
            if (audioLength > 0.0)
            {
                float proportion = (float)(event.x - waveformBounds.getX()) / (float)waveformBounds.getWidth();
                double time = proportion * audioLength;

                if (currentPlacementMode == AppEnums::PlacementMode::LoopIn)
                {
                    owner.setLoopInPosition(time);
                    owner.getSilenceDetector().setIsAutoCutInActive(false);
                    owner.updateComponentStates();
                }
                else if (currentPlacementMode == AppEnums::PlacementMode::LoopOut)
                {
                    owner.setLoopOutPosition(time);
                }
                owner.ensureLoopOrder();
                owner.updateLoopLabels();
            }
            currentPlacementMode = AppEnums::PlacementMode::None; // Reset placement mode
            owner.updateLoopButtonColors(); // Update button colours
            owner.repaint();
        }
        // If it was a click (not a drag) then seek to position.
        // If it was a drag, seekToMousePosition would have already been called.
        else if (mouseDragStartX == event.x)
        {
            seekToMousePosition(event.x);
        }
    }
}

/**
 * @brief Handles mouse exit events from the component's bounds.
 *
 * Resets the internal mouse cursor position state (`mouseCursorX`, `mouseCursorY`)
 * to indicate the mouse is no longer over the component, which typically
 * hides any visual feedback like cursor lines.
 * @param event The mouse event details.
 */
void MouseHandler::mouseExit(const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    mouseCursorX = -1;
    mouseCursorY = -1;
    mouseCursorTime = 0.0;
    owner.repaint();
}

/**
 * @brief Handles right-click events specifically for entering loop point placement modes.
 *
 * This method translates the mouse's X-coordinate into a time, and if
 * in an appropriate placement mode, sets the corresponding loop point
 * in the `ControlPanel`. It also triggers UI updates.
 * @param x The x-coordinate of the mouse click relative to the component.
 */
void MouseHandler::handleRightClickForLoopPlacement(int x)
{
    AudioPlayer& audioPlayer = owner.getAudioPlayer();
    auto audioLength = audioPlayer.getThumbnail().getTotalLength();
    if (audioLength <= 0.0) return;

    const auto waveformBounds = owner.getWaveformBounds();
    float proportion = (float)(x - waveformBounds.getX()) / (float)waveformBounds.getWidth();
    double time = proportion * audioLength;

    if (currentPlacementMode == AppEnums::PlacementMode::LoopIn)
    {
        owner.setLoopInPosition(time);
    }
    else if (currentPlacementMode == AppEnums::PlacementMode::LoopOut)
    {
        owner.setLoopOutPosition(time);
    }
    owner.ensureLoopOrder();
    owner.updateLoopButtonColors();
    owner.updateLoopLabels();
    owner.repaint();
}

/**
 * @brief Seeks the audio player to the position corresponding to the given x-coordinate.
 *
 * This method translates a UI pixel coordinate within the waveform display area
 * into a time in seconds and instructs the `AudioPlayer` (accessed via `ControlPanel`)
 * to seek to that position.
 * @param x The x-coordinate of the mouse position relative to the component.
 */
void MouseHandler::seekToMousePosition(int x)
{
    AudioPlayer& audioPlayer = owner.getAudioPlayer();
    auto audioLength = audioPlayer.getThumbnail().getTotalLength();
    if (audioLength <= 0.0) return;

    const auto waveformBounds = owner.getWaveformBounds();
    float proportion = (float)(x - waveformBounds.getX()) / (float)waveformBounds.getWidth();
    double time = proportion * audioLength;
    audioPlayer.getTransportSource().setPosition(time);
}

void MouseHandler::clearTextEditorFocusIfNeeded(const juce::Point<int>& clickPosition)
{
    for (int i = 0; i < owner.getNumChildComponents(); ++i)
    {
        if (auto* editorChild = dynamic_cast<juce::TextEditor*>(owner.getChildComponent(i)))
        {
            const bool clickInsideEditor = editorChild->getBoundsInParent().contains(clickPosition);
            if (editorChild->hasKeyboardFocus(false) && ! clickInsideEditor)
            {
                editorChild->giveAwayKeyboardFocus();
            }
        }
    }
}
