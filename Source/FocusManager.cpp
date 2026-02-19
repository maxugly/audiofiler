/**
 * @file FocusManager.cpp
 * @brief Defines the FocusManager class.
 * @ingroup Engine
 */

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
        return FocusTarget::CutIn;
    if (mouseHandler.getDraggedHandle() == MouseHandler::CutMarkerHandle::Out)
        return FocusTarget::CutOut;

    // Priority 2: MouseManual (Active Scrubbing/Right-click Placement)
    if (mouseHandler.isScrubbing())
        return FocusTarget::MouseManual;

    // Priority 3: Hovering (Timer Boxes)
    const auto activePoint = owner.getActiveZoomPoint();
    if (activePoint == ControlPanel::ActiveZoomPoint::In)
        return FocusTarget::CutIn;
    if (activePoint == ControlPanel::ActiveZoomPoint::Out)
        return FocusTarget::CutOut;

    // Priority 4: Playback (Default)
    return FocusTarget::Playback;
}

double FocusManager::getFocusedTime() const
{
    FocusTarget target = getCurrentTarget();

    switch (target)
    {
        case FocusTarget::CutIn:
            return owner.getCutInPosition();
        case FocusTarget::CutOut:
            return owner.getCutOutPosition();
        case FocusTarget::MouseManual:
        case FocusTarget::Playback:
        default:
            return owner.getAudioPlayer().getCurrentPosition();
    }
}

double FocusManager::getStepMultiplier(bool shift, bool ctrl)
{
    if (shift && ctrl)
        return 0.01;
    if (shift)
        return 0.1;

    return 1.0;
}
