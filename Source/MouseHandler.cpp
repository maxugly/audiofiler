#include "MouseHandler.h"
#include "FocusManager.h"
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
        
        hoveredHandle = getHandleAtPosition(event.getPosition());

        // Lock handles if autocut is active and locking is enabled in Config
        if (Config::Audio::lockHandlesWhenAutoCutActive)
        {
            const auto& silenceDetector = owner.getSilenceDetector();
            if ((hoveredHandle == CutMarkerHandle::In && silenceDetector.getIsAutoCutInActive()) ||
                (hoveredHandle == CutMarkerHandle::Out && silenceDetector.getIsAutoCutOutActive()) ||
                (hoveredHandle == CutMarkerHandle::Full && (silenceDetector.getIsAutoCutInActive() || silenceDetector.getIsAutoCutOutActive())))
            {
                hoveredHandle = CutMarkerHandle::None;
            }
        }

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
    isScrubbingState = false;
        }
    }
    else
    {
        mouseCursorX = -1;
        mouseCursorY = -1;
        mouseCursorTime = 0.0;
    isScrubbingState = false;
        hoveredHandle = CutMarkerHandle::None;
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
    clearTextEditorFocusIfNeeded(event);

    // --- ZOOM POPUP INTERACTION ---
    if (owner.getActiveZoomPoint() != ControlPanel::ActiveZoomPoint::None)
    {
        auto zoomBounds = owner.getZoomPopupBounds();
        if (zoomBounds.contains(event.getPosition()))
        {
            interactionStartedInZoom = true;
            auto timeRange = owner.getZoomTimeRange();
            float proportion = (float)(event.x - zoomBounds.getX()) / (float)zoomBounds.getWidth();
            double zoomedTime = timeRange.first + (proportion * (timeRange.second - timeRange.first));
            
            if (event.mods.isLeftButtonDown())
            {
                owner.setNeedsJumpToLoopIn(true);
                if (currentPlacementMode == AppEnums::PlacementMode::LoopIn)
                {
                    owner.setCutInPosition(zoomedTime);
                    owner.setAutoCutInActive(false);
                }
                else if (currentPlacementMode == AppEnums::PlacementMode::LoopOut)
                {
                    owner.setCutOutPosition(zoomedTime);
                    owner.setAutoCutOutActive(false);
                }
                else
                {
                    // NOT ARMED: Drag or Seek like the main waveform
                    double loopPointTime = (owner.getActiveZoomPoint() == ControlPanel::ActiveZoomPoint::In)
                                           ? owner.getCutInPosition() : owner.getCutOutPosition();
                    
                    // Check if click is near the indicator (within 20 pixels)
                    float indicatorX = (float)zoomBounds.getX();
                    if (timeRange.second > timeRange.first)
                    {
                        float proportionIndicator = (float)((loopPointTime - timeRange.first) / (timeRange.second - timeRange.first));
                        indicatorX += proportionIndicator * (float)zoomBounds.getWidth();
                    }

                    if (std::abs(event.x - (int)indicatorX) < 20)
                    {
                        draggedHandle = (owner.getActiveZoomPoint() == ControlPanel::ActiveZoomPoint::In) 
                                         ? CutMarkerHandle::In : CutMarkerHandle::Out;
                        dragStartMouseOffset = zoomedTime - loopPointTime; // Prevent jump
                        
                        if (draggedHandle == CutMarkerHandle::In)
                            owner.setAutoCutInActive(false);
                        else
                            owner.setAutoCutOutActive(false);
                    }
                    else
                    {
                        // Seek playback in zoom popup - CONSTRAINED TO LOOP
                        double effectiveLoopIn = juce::jmax(0.0, owner.getCutInPosition());
                        double effectiveLoopOut = owner.getCutOutPosition();
                        double constrainedTime = juce::jlimit(effectiveLoopIn, effectiveLoopOut, zoomedTime);

                        owner.getAudioPlayer().setPlayheadPosition(constrainedTime);
                        isDragging = true;
            isScrubbingState = true;
                        mouseDragStartX = event.x;
                    }
                }
                owner.repaint();
                return;
            }
        }
    }

    interactionStartedInZoom = false;
    const auto waveformBounds = owner.getWaveformBounds();
    if (! waveformBounds.contains(event.getPosition()))
        return;

    if (event.mods.isLeftButtonDown())
    {
        draggedHandle = getHandleAtPosition(event.getPosition());
        auto& silenceDetector = owner.getSilenceDetector();
        
        // Handle Locking logic
        if (Config::Audio::lockHandlesWhenAutoCutActive)
        {
            if ((draggedHandle == CutMarkerHandle::In && silenceDetector.getIsAutoCutInActive()) ||
                (draggedHandle == CutMarkerHandle::Out && silenceDetector.getIsAutoCutOutActive()) ||
                (draggedHandle == CutMarkerHandle::Full && (silenceDetector.getIsAutoCutInActive() || silenceDetector.getIsAutoCutOutActive())))
            {
                draggedHandle = CutMarkerHandle::None;
            }
        }
        
        // Auto-disable logic (if not locked)
        if (draggedHandle != CutMarkerHandle::None)
        {
            if (draggedHandle == CutMarkerHandle::In || draggedHandle == CutMarkerHandle::Full)
            {
                if (silenceDetector.getIsAutoCutInActive()) { owner.setAutoCutInActive(false); }
            }
            if (draggedHandle == CutMarkerHandle::Out || draggedHandle == CutMarkerHandle::Full)
            {
                if (silenceDetector.getIsAutoCutOutActive()) { owner.setAutoCutOutActive(false); }
            }
        }

        // Initialize drag operations
        if (draggedHandle == CutMarkerHandle::Full)
        {
            dragStartCutLength = std::abs(owner.getCutOutPosition() - owner.getCutInPosition());
            
            const auto waveformBounds = owner.getWaveformBounds();
            AudioPlayer& audioPlayer = owner.getAudioPlayer();
            auto audioLength = audioPlayer.getThumbnail().getTotalLength();
            float proportion = (float)(event.x - waveformBounds.getX()) / (float)waveformBounds.getWidth();
            double mouseTime = proportion * audioLength;
            
            dragStartMouseOffset = mouseTime - owner.getCutInPosition();
            owner.repaint();
        }
        else if (draggedHandle == CutMarkerHandle::None)
        {
            isDragging = true;
            isScrubbingState = true;
            mouseDragStartX = event.x;
            currentPlaybackPosOnDragStart = owner.getAudioPlayer().getTransportSource().getCurrentPosition();
            seekToMousePosition(event.x);
        }
        else
        {
            owner.repaint();
        }
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
    if (!event.mods.isLeftButtonDown())
        return;

    // Update cursor position even during drag
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
    }

    // --- ZOOM POPUP DRAG ---
    if (interactionStartedInZoom && owner.getActiveZoomPoint() != ControlPanel::ActiveZoomPoint::None && (draggedHandle != CutMarkerHandle::None || isDragging))
    {
        auto zoomBounds = owner.getZoomPopupBounds();
        if (zoomBounds.contains(event.getPosition()) || draggedHandle != CutMarkerHandle::None || isDragging)
        {
            auto timeRange = owner.getZoomTimeRange();
            int clampedX = juce::jlimit(zoomBounds.getX(), zoomBounds.getRight(), event.x);
            float proportion = (float)(clampedX - zoomBounds.getX()) / (float)zoomBounds.getWidth();
            double zoomedTime = timeRange.first + (proportion * (timeRange.second - timeRange.first));

            if (draggedHandle != CutMarkerHandle::None)
            {
                // Use offset if not in placement mode to prevent jumping
                double offset = (currentPlacementMode == AppEnums::PlacementMode::None) ? dragStartMouseOffset : 0.0;
                
                if (draggedHandle == CutMarkerHandle::In)
                    owner.getAudioPlayer().setCutIn(zoomedTime - offset);
                else if (draggedHandle == CutMarkerHandle::Out)
                    owner.getAudioPlayer().setCutOut(zoomedTime - offset);
                owner.ensureLoopOrder();
            }
            else if (isDragging)
            {
                double effectiveLoopIn = juce::jmax(0.0, owner.getCutInPosition());
                double effectiveLoopOut = owner.getCutOutPosition();
                double constrainedTime = juce::jlimit(effectiveLoopIn, effectiveLoopOut, zoomedTime);
                
                owner.getAudioPlayer().setPlayheadPosition(constrainedTime);
            }

            owner.updateLoopLabels();
            owner.repaint();
            return;
        }
    }

    if (draggedHandle != CutMarkerHandle::None)
    {
        AudioPlayer& audioPlayer = owner.getAudioPlayer();
        auto audioLength = audioPlayer.getThumbnail().getTotalLength();
        if (audioLength > 0.0)
        {
            int clampedX = juce::jlimit(waveformBounds.getX(), waveformBounds.getRight(), event.x);
            float proportion = (float)(clampedX - waveformBounds.getX()) / (float)waveformBounds.getWidth();
            double mouseTime = proportion * audioLength;

            if (draggedHandle == CutMarkerHandle::In)
            {
                owner.getAudioPlayer().setCutIn(mouseTime);
            }
            else if (draggedHandle == CutMarkerHandle::Out)
            {
                owner.getAudioPlayer().setCutOut(mouseTime);
            }
            else if (draggedHandle == CutMarkerHandle::Full)
            {
                double newIn = mouseTime - dragStartMouseOffset;
                double newOut = newIn + dragStartCutLength;
                
                // Clamp to bounds while preserving length
                if (newIn < 0.0)
                {
                    newIn = 0.0;
                    newOut = dragStartCutLength;
                }
                else if (newOut > audioLength)
                {
                    newOut = audioLength;
                    newIn = audioLength - dragStartCutLength;
                }
                
                owner.getAudioPlayer().setCutIn(newIn);
                owner.getAudioPlayer().setCutOut(newOut);

                // Ensure cursor stays in the moving loop
                audioPlayer.setPlayheadPosition(audioPlayer.getTransportSource().getCurrentPosition());
            }

            owner.ensureLoopOrder();
            owner.updateLoopLabels();
            owner.repaint();
        }
    }
    else if (isDragging && waveformBounds.contains(event.getPosition()))
    {
        seekToMousePosition(event.x);
        owner.repaint();
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
    // --- ZOOM POPUP UP ---
    if (owner.getActiveZoomPoint() != ControlPanel::ActiveZoomPoint::None && (isDragging || draggedHandle != CutMarkerHandle::None || currentPlacementMode != AppEnums::PlacementMode::None))
    {
        if (currentPlacementMode != AppEnums::PlacementMode::None)
        {
            currentPlacementMode = AppEnums::PlacementMode::None;
            owner.updateLoopButtonColors();
        }
        isDragging = false;
    isScrubbingState = false;
        draggedHandle = CutMarkerHandle::None;
        owner.repaint();
        // We don't return here yet because we might want to let the regular logic run too, 
        // but for zoom popup interaction, it's usually enough.
        // Actually, if we were interacting with zoom, we should consume this.
        return;
    }

    isDragging = false;
    isScrubbingState = false;
    draggedHandle = CutMarkerHandle::None;
    owner.jumpToLoopIn(); // Jump after regular drag
    
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
                    owner.setCutInPosition(time);
                    owner.setAutoCutInActive(false);
                }
                else if (currentPlacementMode == AppEnums::PlacementMode::LoopOut)
                {
                    owner.setCutOutPosition(time);
                    owner.setAutoCutOutActive(false);
                }
                owner.ensureLoopOrder();
                owner.updateLoopLabels();
                owner.jumpToLoopIn(); // Immediate jump on waveform placement
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
    isScrubbingState = false;
    hoveredHandle = CutMarkerHandle::None;
    owner.repaint();
}

void MouseHandler::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    const auto waveformBounds = owner.getWaveformBounds();
    if (!waveformBounds.contains(event.getPosition()))
        return;

    // CTRL + Mouse Wheel (without Shift) ALWAYS controls zoom
    if (event.mods.isCtrlDown() && !event.mods.isShiftDown())
    {
        float currentZoom = owner.getZoomFactor();
        float zoomDelta = wheel.deltaY > 0 ? 1.1f : 0.9f;
        owner.setZoomFactor(currentZoom * zoomDelta);
        return;
    }

    AudioPlayer& audioPlayer = owner.getAudioPlayer();
    auto& transport = audioPlayer.getTransportSource();
    double currentPos = transport.getCurrentPosition();
    double audioLength = audioPlayer.getThumbnail().getTotalLength();
    
    if (audioLength <= 0.0)
        return;
    // Use fixed time steps (0.01s base) scaled by FocusManager
    double multiplier = FocusManager::getStepMultiplier(event.mods.isShiftDown(), event.mods.isCtrlDown());
    double step = 0.01 * multiplier;
    
    // Alt is a x10 multiplier
    if (event.mods.isAltDown())
        step *= 10.0;

    double direction = (wheel.deltaY > 0) ? 1.0 : -1.0;
    double newPos = currentPos + (direction * step);

    // Constrain to loop
    double effectiveLoopIn = juce::jmax(0.0, owner.getCutInPosition());
    double effectiveLoopOut = owner.getCutOutPosition();
    if (effectiveLoopOut <= 0.0) effectiveLoopOut = audioLength;

    audioPlayer.setPlayheadPosition(newPos);
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
        owner.setCutInPosition(time);
        owner.setAutoCutInActive(false);
    }
    else if (currentPlacementMode == AppEnums::PlacementMode::LoopOut)
    {
        owner.setCutOutPosition(time);
        owner.setAutoCutOutActive(false);
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

    audioPlayer.setPlayheadPosition(time);
}

void MouseHandler::clearTextEditorFocusIfNeeded(const juce::MouseEvent& event)
{
    // Robust "inside" check using Screen coordinates to avoid coordinate-space errors.
    const auto screenPos = event.getScreenPosition();
    
    for (int i = 0; i < owner.getNumChildComponents(); ++i)
    {
        auto* child = owner.getChildComponent(i);
        if (auto* editorChild = dynamic_cast<juce::TextEditor*>(child))
        {
            if (editorChild->getScreenBounds().contains(screenPos))
                return; // STOP: We clicked inside an editor. Do not clear focus.
        }
    }

    // If we didn't return, clear focus from all editors (we clicked the waveform/background).
    for (int i = 0; i < owner.getNumChildComponents(); ++i)
    {
        if (auto* editorChild = dynamic_cast<juce::TextEditor*>(owner.getChildComponent(i)))
        {
            if (editorChild->hasKeyboardFocus(false))
                editorChild->giveAwayKeyboardFocus();
        }
    }
}

MouseHandler::CutMarkerHandle MouseHandler::getHandleAtPosition(juce::Point<int> pos) const
{
    const auto waveformBounds = owner.getWaveformBounds();
    AudioPlayer& audioPlayer = owner.getAudioPlayer();
    auto audioLength = audioPlayer.getThumbnail().getTotalLength();
    if (audioLength <= 0.0) return CutMarkerHandle::None;

    auto checkHandle = [&](double time) -> bool {
        float x = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (float)(time / audioLength);
        
        // Define handle hitboxes: Full height vertical strip (30px wide)
        juce::Rectangle<int> hitStrip((int)(x - Config::Layout::Glow::loopMarkerBoxWidth / 2.0f),
                                      waveformBounds.getY(), 
                                      (int)Config::Layout::Glow::loopMarkerBoxWidth,
                                      waveformBounds.getHeight());
                                       
        return hitStrip.contains(pos);
    };

    if (checkHandle(owner.getCutInPosition())) return CutMarkerHandle::In;
    if (checkHandle(owner.getCutOutPosition())) return CutMarkerHandle::Out;

    // Check for Full cut handle (top/bottom box areas between markers)
    const double cutIn = owner.getCutInPosition();
    const double cutOut = owner.getCutOutPosition();
    const double actualIn = juce::jmin(cutIn, cutOut);
    const double actualOut = juce::jmax(cutIn, cutOut);
    
    float inX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (float)(actualIn / audioLength);
    float outX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (float)(actualOut / audioLength);
    
    int hollowHeight = Config::Layout::Glow::loopMarkerBoxHeight;
    
    juce::Rectangle<int> topHollow((int)inX, waveformBounds.getY(), (int)(outX - inX), hollowHeight);
    juce::Rectangle<int> bottomHollow((int)inX, waveformBounds.getBottom() - hollowHeight, (int)(outX - inX), hollowHeight);
    
    if (topHollow.contains(pos) || bottomHollow.contains(pos))
        return CutMarkerHandle::Full;

    return CutMarkerHandle::None;
}
