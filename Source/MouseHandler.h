/**
 * @file MouseHandler.h
 * @brief Defines the MouseHandler class.
 * @ingroup Engine
 */

#ifndef AUDIOFILER_MOUSEHANDLER_H
#define AUDIOFILER_MOUSEHANDLER_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

#include "Config.h"
#include "AppEnums.h"

/**
 * @class ControlPanel
 * @brief Home: View.
 *
 */
class ControlPanel;

/**
 * @class MouseHandler
 * @brief Handles all mouse interaction logic for the ControlPanel.
 */
class MouseHandler : public juce::MouseListener
{
public:
    /**
     * @brief Undocumented method.
     * @param controlPanel [in] Description for controlPanel.
     */
    explicit MouseHandler(ControlPanel& controlPanel);
    ~MouseHandler() override = default;

    /**
     * @brief Undocumented method.
     * @param event [in] Description for event.
     */
    void mouseMove(const juce::MouseEvent& event) override;
    /**
     * @brief Undocumented method.
     * @param event [in] Description for event.
     */
    void mouseDown(const juce::MouseEvent& event) override;
    /**
     * @brief Undocumented method.
     * @param event [in] Description for event.
     */
    void mouseDrag(const juce::MouseEvent& event) override;
    /**
     * @brief Undocumented method.
     * @param event [in] Description for event.
     */
    void mouseUp(const juce::MouseEvent& event) override;
    /**
     * @brief Undocumented method.
     * @param event [in] Description for event.
     */
    void mouseExit(const juce::MouseEvent& event) override;
    /**
     * @brief Undocumented method.
     * @param event [in] Description for event.
     * @param wheel [in] Description for wheel.
     */
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

    /**
     * @brief Gets the MouseCursorX.
     * @return int
     */
    int getMouseCursorX() const { return mouseCursorX; }
    /**
     * @brief Gets the MouseCursorY.
     * @return int
     */
    int getMouseCursorY() const { return mouseCursorY; }
    /**
     * @brief Gets the MouseCursorTime.
     * @return double
     */
    double getMouseCursorTime() const { return mouseCursorTime; }
    
    /**
     * @brief Gets the CurrentPlacementMode.
     * @return AppEnums::PlacementMode
     */
    AppEnums::PlacementMode getCurrentPlacementMode() const { return currentPlacementMode; }
    /**
     * @brief Sets the PlacementMode.
     * @param mode [in] Description for mode.
     */
    void setPlacementMode(AppEnums::PlacementMode mode) { currentPlacementMode = mode; }

    enum class CutMarkerHandle {
        None,
        In,
        Out,
        Full
    };

    /**
     * @brief Gets the HoveredHandle.
     * @return CutMarkerHandle
     */
    CutMarkerHandle getHoveredHandle() const { return hoveredHandle; }
    /**
     * @brief Gets the DraggedHandle.
     * @return CutMarkerHandle
     */
    CutMarkerHandle getDraggedHandle() const { return draggedHandle; }
    /**
     * @brief Checks if Scrubbing.
     * @return bool
     */
    bool isScrubbing() const { return isScrubbingState; }

private:
    ControlPanel& owner;
    int mouseCursorX = -1, mouseCursorY = -1;
    double mouseCursorTime = 0.0;
    bool isDragging = false;
    double currentPlaybackPosOnDragStart = 0.0;
    int mouseDragStartX = 0;
    AppEnums::PlacementMode currentPlacementMode = AppEnums::PlacementMode::None;

    CutMarkerHandle hoveredHandle = CutMarkerHandle::None;
    CutMarkerHandle draggedHandle = CutMarkerHandle::None;
    double dragStartCutLength = 0.0;
    double dragStartMouseOffset = 0.0;
    bool interactionStartedInZoom = false;
    bool isScrubbingState = false;

    /**
     * @brief Gets the HandleAtPosition.
     * @param pos [in] Description for pos.
     * @return CutMarkerHandle
     */
    CutMarkerHandle getHandleAtPosition(juce::Point<int> pos) const;
    /**
     * @brief Undocumented method.
     * @param x [in] Description for x.
     */
    void handleRightClickForCutPlacement(int x);
    /**
     * @brief Undocumented method.
     * @param x [in] Description for x.
     */
    void seekToMousePosition(int x);
    /**
     * @brief Undocumented method.
     * @param event [in] Description for event.
     */
    void clearTextEditorFocusIfNeeded(const juce::MouseEvent& event);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MouseHandler)
};

#endif 
