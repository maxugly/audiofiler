

#include "UI/MouseHandler.h"
#include "UI/FocusManager.h"
#include "UI/ControlPanel.h"
#include "Core/AudioPlayer.h"
#include "Utils/CoordinateMapper.h"

MouseHandler::MouseHandler(ControlPanel& controlPanel) : owner(controlPanel) {}

double MouseHandler::getMouseTime(int x, const juce::Rectangle<int>& bounds, double duration) const {
    if (duration <= 0.0) return 0.0;
    double rawTime = CoordinateMapper::pixelsToSeconds((float)(x - bounds.getX()), (float)bounds.getWidth(), duration);
    
    double sampleRate = 0.0;
    juce::int64 length = 0;
    owner.getAudioPlayer().getReaderInfo(sampleRate, length);
    return owner.getInteractionCoordinator().getSnappedTime(rawTime, sampleRate);
}

void MouseHandler::mouseMove(const juce::MouseEvent& event) {
    const auto waveformBounds = owner.getWaveformBounds();
    if (waveformBounds.contains(event.getPosition())) {
        mouseCursorX = event.x;
        mouseCursorY = event.y;
        hoveredHandle = getHandleAtPosition(event.getPosition());

        if (Config::Audio::lockHandlesWhenAutoCutActive) {
            const auto& sd = owner.getSilenceDetector();
            if ((hoveredHandle == CutMarkerHandle::In && sd.getIsAutoCutInActive()) ||
                (hoveredHandle == CutMarkerHandle::Out && sd.getIsAutoCutOutActive()) ||
                (hoveredHandle == CutMarkerHandle::Full && (sd.getIsAutoCutInActive() || sd.getIsAutoCutOutActive()))) {
                hoveredHandle = CutMarkerHandle::None;
            }
        }
        mouseCursorTime = getMouseTime(mouseCursorX, waveformBounds, owner.getAudioPlayer().getThumbnail().getTotalLength());
    } else {
        mouseCursorX = mouseCursorY = -1;
        mouseCursorTime = 0.0;
        isScrubbingState = false;
        hoveredHandle = CutMarkerHandle::None;
    }
    owner.repaint();
}

void MouseHandler::mouseDown(const juce::MouseEvent& event) {
    clearTextEditorFocusIfNeeded(event);
    auto& coordinator = owner.getInteractionCoordinator();
    const double audioLength = owner.getAudioPlayer().getThumbnail().getTotalLength();

    if (coordinator.getActiveZoomPoint() != AppEnums::ActiveZoomPoint::None) {
        auto zb = coordinator.getZoomPopupBounds();
        if (zb.contains(event.getPosition())) {
            interactionStartedInZoom = true;
            auto tr = coordinator.getZoomTimeRange();
            double zoomedTime = CoordinateMapper::pixelsToSeconds((float)(event.x - zb.getX()), (float)zb.getWidth(), tr.second - tr.first) + tr.first;

            if (event.mods.isLeftButtonDown()) {
                coordinator.setNeedsJumpToCutIn(true);
                const auto pm = coordinator.getPlacementMode();
                if (pm != AppEnums::PlacementMode::None) {
                    auto marker = (pm == AppEnums::PlacementMode::CutIn) ? AppEnums::ActiveZoomPoint::In : AppEnums::ActiveZoomPoint::Out;
                    coordinator.validateMarkerPosition(marker, zoomedTime, owner.getCutInPosition(), owner.getCutOutPosition(), audioLength);
                    if (pm == AppEnums::PlacementMode::CutIn) { owner.setCutInPosition(zoomedTime); owner.setAutoCutInActive(false); }
                    else { owner.setCutOutPosition(zoomedTime); owner.setAutoCutOutActive(false); }
                } else {
                    const auto azp = coordinator.getActiveZoomPoint();
                    double cpt = (azp == AppEnums::ActiveZoomPoint::In) ? owner.getCutInPosition() : owner.getCutOutPosition();
                    float indicatorX = (float)zb.getX() + CoordinateMapper::secondsToPixels(cpt - tr.first, (float)zb.getWidth(), tr.second - tr.first);

                    if (std::abs(event.x - (int)indicatorX) < 20) {
                        draggedHandle = (azp == AppEnums::ActiveZoomPoint::In) ? CutMarkerHandle::In : CutMarkerHandle::Out;
                        dragStartMouseOffset = zoomedTime - cpt;
                        if (draggedHandle == CutMarkerHandle::In) owner.setAutoCutInActive(false); else owner.setAutoCutOutActive(false);
                    } else {
                        owner.getAudioPlayer().setPlayheadPosition(zoomedTime);
                        isDragging = isScrubbingState = true;
                        mouseDragStartX = event.x;
                    }
                }
                owner.repaint(); return;
            }
        }
    }

    interactionStartedInZoom = false;
    const auto wb = owner.getWaveformBounds();
    if (!wb.contains(event.getPosition())) return;

    if (event.mods.isLeftButtonDown()) {
        draggedHandle = getHandleAtPosition(event.getPosition());
        auto& sd = owner.getSilenceDetector();

        if (Config::Audio::lockHandlesWhenAutoCutActive && 
           ((draggedHandle == CutMarkerHandle::In && sd.getIsAutoCutInActive()) ||
            (draggedHandle == CutMarkerHandle::Out && sd.getIsAutoCutOutActive()) ||
            (draggedHandle == CutMarkerHandle::Full && (sd.getIsAutoCutInActive() || sd.getIsAutoCutOutActive())))) {
            draggedHandle = CutMarkerHandle::None;
        }

        if (draggedHandle != CutMarkerHandle::None) {
            if (draggedHandle == CutMarkerHandle::In || draggedHandle == CutMarkerHandle::Full) owner.setAutoCutInActive(false);
            if (draggedHandle == CutMarkerHandle::Out || draggedHandle == CutMarkerHandle::Full) owner.setAutoCutOutActive(false);
            
            if (draggedHandle == CutMarkerHandle::Full) {
                dragStartCutLength = std::abs(owner.getCutOutPosition() - owner.getCutInPosition());
                dragStartMouseOffset = getMouseTime(event.x, wb, audioLength) - owner.getCutInPosition();
            }
            owner.repaint();
        } else {
            isDragging = isScrubbingState = true;
            mouseDragStartX = event.x;
            seekToMousePosition(event.x);
        }
    } else if (event.mods.isRightButtonDown()) {
        handleRightClickForCutPlacement(event.x);
    }
}

void MouseHandler::mouseDrag(const juce::MouseEvent& event) {
    if (!event.mods.isLeftButtonDown()) return;
    const auto wb = owner.getWaveformBounds();
    const double audioLength = owner.getAudioPlayer().getThumbnail().getTotalLength();
    auto& coordinator = owner.getInteractionCoordinator();

    if (wb.contains(event.getPosition())) {
        mouseCursorX = event.x; mouseCursorY = event.y;
        mouseCursorTime = getMouseTime(event.x, wb, audioLength);
    }

    if (interactionStartedInZoom && coordinator.getActiveZoomPoint() != AppEnums::ActiveZoomPoint::None && (draggedHandle != CutMarkerHandle::None || isDragging)) {
        auto zb = coordinator.getZoomPopupBounds();
        auto tr = coordinator.getZoomTimeRange();
        int cx = juce::jlimit(zb.getX(), zb.getRight(), event.x);
        double zt = CoordinateMapper::pixelsToSeconds((float)(cx - zb.getX()), (float)zb.getWidth(), tr.second - tr.first) + tr.first;

        if (draggedHandle != CutMarkerHandle::None) {
            double offset = (coordinator.getPlacementMode() == AppEnums::PlacementMode::None) ? dragStartMouseOffset : 0.0;
            double tt = zt - offset;
            auto marker = (draggedHandle == CutMarkerHandle::In) ? AppEnums::ActiveZoomPoint::In : AppEnums::ActiveZoomPoint::Out;
            coordinator.validateMarkerPosition(marker, tt, owner.getCutInPosition(), owner.getCutOutPosition(), audioLength);
            if (draggedHandle == CutMarkerHandle::In) owner.getAudioPlayer().setCutIn(tt); else owner.getAudioPlayer().setCutOut(tt);
            owner.ensureCutOrder();
        } else if (isDragging) {
            owner.getAudioPlayer().setPlayheadPosition(zt);
        }
        owner.refreshLabels(); owner.repaint(); return;
    }

    if (draggedHandle != CutMarkerHandle::None) {
        double mt = getMouseTime(juce::jlimit(wb.getX(), wb.getRight(), event.x), wb, audioLength);
        if (draggedHandle == CutMarkerHandle::Full) {
            double ni = mt - dragStartMouseOffset, no = ni + dragStartCutLength;
            coordinator.constrainFullRegionMove(ni, no, dragStartCutLength, audioLength);
            owner.getAudioPlayer().setCutIn(ni); owner.getAudioPlayer().setCutOut(no);
            owner.getAudioPlayer().setPlayheadPosition(owner.getAudioPlayer().getCurrentPosition());
        } else {
            auto marker = (draggedHandle == CutMarkerHandle::In) ? AppEnums::ActiveZoomPoint::In : AppEnums::ActiveZoomPoint::Out;
            coordinator.validateMarkerPosition(marker, mt, owner.getCutInPosition(), owner.getCutOutPosition(), audioLength);
            if (draggedHandle == CutMarkerHandle::In) owner.getAudioPlayer().setCutIn(mt); else owner.getAudioPlayer().setCutOut(mt);
        }
        owner.ensureCutOrder(); owner.refreshLabels(); owner.repaint();
    } else if (isDragging && wb.contains(event.getPosition())) {
        seekToMousePosition(event.x); owner.repaint();
    }
}

void MouseHandler::mouseUp(const juce::MouseEvent& event) {
    auto& coordinator = owner.getInteractionCoordinator();
    if (coordinator.getActiveZoomPoint() != AppEnums::ActiveZoomPoint::None && (isDragging || draggedHandle != CutMarkerHandle::None || coordinator.getPlacementMode() != AppEnums::PlacementMode::None)) {
        if (coordinator.getPlacementMode() != AppEnums::PlacementMode::None) {
            coordinator.setPlacementMode(AppEnums::PlacementMode::None);
            owner.updateCutButtonColors();
        }
        isDragging = isScrubbingState = false; draggedHandle = CutMarkerHandle::None;
        owner.repaint(); return;
    }

    isDragging = isScrubbingState = false; draggedHandle = CutMarkerHandle::None;
    owner.jumpToCutIn();

    const auto wb = owner.getWaveformBounds();
    if (wb.contains(event.getPosition()) && event.mods.isLeftButtonDown()) {
        const auto pm = coordinator.getPlacementMode();
        if (pm != AppEnums::PlacementMode::None) {
            double t = getMouseTime(event.x, wb, owner.getAudioPlayer().getThumbnail().getTotalLength());
            auto marker = (pm == AppEnums::PlacementMode::CutIn) ? AppEnums::ActiveZoomPoint::In : AppEnums::ActiveZoomPoint::Out;
            coordinator.validateMarkerPosition(marker, t, owner.getCutInPosition(), owner.getCutOutPosition(), owner.getAudioPlayer().getThumbnail().getTotalLength());
            if (pm == AppEnums::PlacementMode::CutIn) { owner.setCutInPosition(t); owner.setAutoCutInActive(false); }
            else { owner.setCutOutPosition(t); owner.setAutoCutOutActive(false); }
            owner.ensureCutOrder(); owner.refreshLabels(); owner.jumpToCutIn();
            coordinator.setPlacementMode(AppEnums::PlacementMode::None);
            owner.updateCutButtonColors();
        } else if (mouseDragStartX == event.x) {
            seekToMousePosition(event.x);
        }
    }
    owner.repaint();
}

void MouseHandler::mouseExit(const juce::MouseEvent&) {
    mouseCursorX = mouseCursorY = -1; mouseCursorTime = 0.0;
    isScrubbingState = false; hoveredHandle = CutMarkerHandle::None;
    owner.repaint();
}

void MouseHandler::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) {
    const auto wb = owner.getWaveformBounds();
    if (!wb.contains(event.getPosition())) return;

    if (event.mods.isCtrlDown() && !event.mods.isShiftDown()) {
        owner.setZoomFactor(owner.getZoomFactor() * (wheel.deltaY > 0 ? 1.1f : 0.9f));
        return;
    }

    double step = 0.01 * FocusManager::getStepMultiplier(event.mods.isShiftDown(), event.mods.isCtrlDown());
    if (event.mods.isAltDown()) step *= 10.0;
    owner.getAudioPlayer().setPlayheadPosition(owner.getAudioPlayer().getCurrentPosition() + ((wheel.deltaY > 0) ? 1.0 : -1.0) * step);
    owner.repaint();
}

void MouseHandler::handleRightClickForCutPlacement(int x) {
    const auto wb = owner.getWaveformBounds();
    const double al = owner.getAudioPlayer().getThumbnail().getTotalLength();
    if (al <= 0.0) return;

    double t = getMouseTime(x, wb, al);
    auto& coordinator = owner.getInteractionCoordinator();
    const auto pm = coordinator.getPlacementMode();
    if (pm != AppEnums::PlacementMode::None) {
        auto m = (pm == AppEnums::PlacementMode::CutIn) ? AppEnums::ActiveZoomPoint::In : AppEnums::ActiveZoomPoint::Out;
        coordinator.validateMarkerPosition(m, t, owner.getCutInPosition(), owner.getCutOutPosition(), al);
        if (pm == AppEnums::PlacementMode::CutIn) { owner.setCutInPosition(t); owner.setAutoCutInActive(false); }
        else { owner.setCutOutPosition(t); owner.setAutoCutOutActive(false); }
        owner.ensureCutOrder(); owner.updateCutButtonColors(); owner.refreshLabels();
    }
    owner.repaint();
}

void MouseHandler::seekToMousePosition(int x) {
    const auto wb = owner.getWaveformBounds();
    owner.getAudioPlayer().setPlayheadPosition(getMouseTime(x, wb, owner.getAudioPlayer().getThumbnail().getTotalLength()));
}

void MouseHandler::clearTextEditorFocusIfNeeded(const juce::MouseEvent& event) {
    const auto sp = event.getScreenPosition();
    for (int i = 0; i < owner.getNumChildComponents(); ++i) {
        auto* c = owner.getChildComponent(i);
        if (auto* e = dynamic_cast<juce::TextEditor*>(c)) {
            if (e->getScreenBounds().contains(sp)) return;
        }
    }
    for (int i = 0; i < owner.getNumChildComponents(); ++i) {
        if (auto* e = dynamic_cast<juce::TextEditor*>(owner.getChildComponent(i))) {
            if (e->hasKeyboardFocus(false)) e->giveAwayKeyboardFocus();
        }
    }
}

bool MouseHandler::isHandleActive(CutMarkerHandle h) const {
    if (draggedHandle == h || hoveredHandle == h) return true;
    auto& c = owner.getInteractionCoordinator();
    return (h == CutMarkerHandle::In && c.getPlacementMode() == AppEnums::PlacementMode::CutIn) ||
           (h == CutMarkerHandle::Out && c.getPlacementMode() == AppEnums::PlacementMode::CutOut);
}

MouseHandler::CutMarkerHandle MouseHandler::getHandleAtPosition(juce::Point<int> pos) const {
    const auto wb = owner.getWaveformBounds();
    const double al = owner.getAudioPlayer().getThumbnail().getTotalLength();
    if (al <= 0.0) return CutMarkerHandle::None;

    auto check = [&](double t) {
        float x = (float)wb.getX() + CoordinateMapper::secondsToPixels(t, (float)wb.getWidth(), al);
        return juce::Rectangle<int>((int)(x - Config::Layout::Glow::cutMarkerBoxWidth / 2.0f), wb.getY(), (int)Config::Layout::Glow::cutMarkerBoxWidth, wb.getHeight()).contains(pos);
    };

    if (check(owner.getCutInPosition())) return CutMarkerHandle::In;
    if (check(owner.getCutOutPosition())) return CutMarkerHandle::Out;

    const double actualIn = juce::jmin(owner.getCutInPosition(), owner.getCutOutPosition()), actualOut = juce::jmax(owner.getCutInPosition(), owner.getCutOutPosition());
    float inX = (float)wb.getX() + CoordinateMapper::secondsToPixels(actualIn, (float)wb.getWidth(), al), outX = (float)wb.getX() + CoordinateMapper::secondsToPixels(actualOut, (float)wb.getWidth(), al);
    int hh = Config::Layout::Glow::cutMarkerBoxHeight;
    if (juce::Rectangle<int>((int)inX, wb.getY(), (int)(outX - inX), hh).contains(pos) || juce::Rectangle<int>((int)inX, wb.getBottom() - hh, (int)(outX - inX), hh).contains(pos)) return CutMarkerHandle::Full;

    return CutMarkerHandle::None;
}
