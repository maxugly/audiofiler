

#ifndef AUDIOFILER_MOUSEHANDLER_H
#define AUDIOFILER_MOUSEHANDLER_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

#include "Utils/Config.h"
#include "Core/AppEnums.h"

class ControlPanel;

class MouseHandler : public juce::MouseListener
{
public:

    explicit MouseHandler(ControlPanel& controlPanel);
    ~MouseHandler() override = default;

    void mouseMove(const juce::MouseEvent& event) override;

    void mouseDown(const juce::MouseEvent& event) override;

    void mouseDrag(const juce::MouseEvent& event) override;

    void mouseUp(const juce::MouseEvent& event) override;

    void mouseExit(const juce::MouseEvent& event) override;

    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

    int getMouseCursorX() const { return mouseCursorX; }

    int getMouseCursorY() const { return mouseCursorY; }

    double getMouseCursorTime() const { return mouseCursorTime; }

    enum class CutMarkerHandle {
        None,
        In,
        Out,
        Full
    };

    CutMarkerHandle getHoveredHandle() const { return hoveredHandle; }

    CutMarkerHandle getDraggedHandle() const { return draggedHandle; }

    /** @brief Returns true if the given handle is currently hovered, dragged, or armed. */
    bool isHandleActive(CutMarkerHandle handle) const;

    bool isScrubbing() const { return isScrubbingState; }

private:
    double getMouseTime(int x, const juce::Rectangle<int>& bounds, double duration) const;

    ControlPanel& owner;
    int mouseCursorX = -1, mouseCursorY = -1;
    double mouseCursorTime = 0.0;
    bool isDragging = false;
    double currentPlaybackPosOnDragStart = 0.0;
    int mouseDragStartX = 0;

    CutMarkerHandle hoveredHandle = CutMarkerHandle::None;
    CutMarkerHandle draggedHandle = CutMarkerHandle::None;
    double dragStartCutLength = 0.0;
    double dragStartMouseOffset = 0.0;
    bool interactionStartedInZoom = false;
    bool isScrubbingState = false;

    CutMarkerHandle getHandleAtPosition(juce::Point<int> pos) const;

    void handleRightClickForCutPlacement(int x);

    void seekToMousePosition(int x);

    void clearTextEditorFocusIfNeeded(const juce::MouseEvent& event);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MouseHandler)
};

#endif 
