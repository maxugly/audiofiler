#include "FocusManager.h"
#include "ControlPanel.h"
#include "MouseHandler.h"
#include "AudioPlayer.h"

FocusManager::FocusManager(ControlPanel& owner) : owner(owner) {}

FocusTarget FocusManager::getCurrentTarget() const
{
    const auto& mouseHandler = owner.getMouseHandler();

    // Priority 1: Dragging a handle (Highest Priority)
    if (mouseHandler.getDraggedHandle() == MouseHandler::CutMarkerHandle::In)
        return FocusTarget::LoopIn;
    if (mouseHandler.getDraggedHandle() == MouseHandler::CutMarkerHandle::Out)
        return FocusTarget::LoopOut;

    // Priority 2: MouseManual (Active Scrubbing/Right-click Placement)
    if (mouseHandler.isScrubbing())
        return FocusTarget::MouseManual;

    // Priority 3: Hovering (Timer Boxes)
    const auto activePoint = owner.getActiveZoomPoint();
    if (activePoint == ControlPanel::ActiveZoomPoint::In)
        return FocusTarget::LoopIn;
    if (activePoint == ControlPanel::ActiveZoomPoint::Out)
        return FocusTarget::LoopOut;

    // Priority 4: Playback (Default)
    return FocusTarget::Playback;
}

double FocusManager::getFocusedTime() const
{
    FocusTarget target = getCurrentTarget();

    switch (target)
    {
        case FocusTarget::LoopIn:
            return owner.getLoopInPosition();
        case FocusTarget::LoopOut:
            return owner.getLoopOutPosition();
        case FocusTarget::MouseManual:
        case FocusTarget::Playback:
        default:
            return owner.getAudioPlayer().getTransportSource().getCurrentPosition();
    }
}

double FocusManager::getStepMultiplier(bool shift, bool ctrl)
{
    // Unified multiplier logic:
    // Normal: 1.0
    // Shift: 0.1 (Fine)
    // Shift + Ctrl: 0.01 (Very Fine)

    if (shift && ctrl)
        return 0.01;
    if (shift)
        return 0.1;

    return 1.0;
}
