

#include "FocusManager.h"
#include "ControlPanel.h"
#include "MouseHandler.h"
#include "AudioPlayer.h"

FocusManager::FocusManager(ControlPanel& owner) : owner(owner) {}

FocusTarget FocusManager::getCurrentTarget() const
{
    const auto& mouseHandler = owner.getMouseHandler();

    if (mouseHandler.getDraggedHandle() == MouseHandler::CutMarkerHandle::In)
        return FocusTarget::CutIn;
    if (mouseHandler.getDraggedHandle() == MouseHandler::CutMarkerHandle::Out)
        return FocusTarget::CutOut;

    if (mouseHandler.isScrubbing())
        return FocusTarget::MouseManual;

    const auto activePoint = owner.getActiveZoomPoint();
    if (activePoint == ControlPanel::ActiveZoomPoint::In)
        return FocusTarget::CutIn;
    if (activePoint == ControlPanel::ActiveZoomPoint::Out)
        return FocusTarget::CutOut;

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
