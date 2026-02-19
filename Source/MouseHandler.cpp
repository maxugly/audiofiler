

#include "MouseHandler.h"
#include "FocusManager.h"
#include "ControlPanel.h"
#include "AudioPlayer.h"
#include "CoordinateMapper.h"

MouseHandler::MouseHandler(ControlPanel& controlPanel) : owner(controlPanel)
{
}

void MouseHandler::mouseMove(const juce::MouseEvent& event)
{
    const auto waveformBounds = owner.getWaveformBounds();
    if (waveformBounds.contains(event.getPosition()))
    {
        mouseCursorX = event.x;
        mouseCursorY = event.y;

        hoveredHandle = getHandleAtPosition(event.getPosition());

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
            mouseCursorTime = CoordinateMapper::pixelsToSeconds((float)(mouseCursorX - waveformBounds.getX()), 
                                                                (float)waveformBounds.getWidth(), 
                                                                audioLength);
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
    owner.repaint();
}

void MouseHandler::mouseDown(const juce::MouseEvent& event)
{

    clearTextEditorFocusIfNeeded(event);

    if (owner.getActiveZoomPoint() != AppEnums::ActiveZoomPoint::None)
    {
        auto zoomBounds = owner.getZoomPopupBounds();
        if (zoomBounds.contains(event.getPosition()))
        {
            interactionStartedInZoom = true;
            auto timeRange = owner.getZoomTimeRange();
            double zoomedTime = CoordinateMapper::pixelsToSeconds((float)(event.x - zoomBounds.getX()), 
                                                                  (float)zoomBounds.getWidth(), 
                                                                  timeRange.second - timeRange.first) + timeRange.first;

            if (event.mods.isLeftButtonDown())
            {
                owner.setNeedsJumpToCutIn(true);
                if (currentPlacementMode == AppEnums::PlacementMode::CutIn)
                {
                    owner.setCutInPosition(zoomedTime);
                    owner.setAutoCutInActive(false);
                }
                else if (currentPlacementMode == AppEnums::PlacementMode::CutOut)
                {
                    owner.setCutOutPosition(zoomedTime);
                    owner.setAutoCutOutActive(false);
                }
                else
                {
                    double cutPointTime = (owner.getActiveZoomPoint() == AppEnums::ActiveZoomPoint::In)
                                           ? owner.getCutInPosition() : owner.getCutOutPosition();

                    float indicatorX = (float)zoomBounds.getX() + 
                                       CoordinateMapper::secondsToPixels(cutPointTime - timeRange.first, 
                                                                         (float)zoomBounds.getWidth(), 
                                                                         timeRange.second - timeRange.first);

                    if (std::abs(event.x - (int)indicatorX) < 20)
                    {
                        draggedHandle = (owner.getActiveZoomPoint() == AppEnums::ActiveZoomPoint::In) 
                                         ? CutMarkerHandle::In : CutMarkerHandle::Out;
                        dragStartMouseOffset = zoomedTime - cutPointTime;

                        if (draggedHandle == CutMarkerHandle::In)
                            owner.setAutoCutInActive(false);
                        else
                            owner.setAutoCutOutActive(false);
                    }
                    else
                    {
                        owner.getAudioPlayer().setPlayheadPosition(zoomedTime);
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

        if (Config::Audio::lockHandlesWhenAutoCutActive)
        {
            if ((draggedHandle == CutMarkerHandle::In && silenceDetector.getIsAutoCutInActive()) ||
                (draggedHandle == CutMarkerHandle::Out && silenceDetector.getIsAutoCutOutActive()) ||
                (draggedHandle == CutMarkerHandle::Full && (silenceDetector.getIsAutoCutInActive() || silenceDetector.getIsAutoCutOutActive())))
            {
                draggedHandle = CutMarkerHandle::None;
            }
        }

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

        if (draggedHandle == CutMarkerHandle::Full)
        {
            dragStartCutLength = std::abs(owner.getCutOutPosition() - owner.getCutInPosition());

            const auto waveformBounds = owner.getWaveformBounds();
            AudioPlayer& audioPlayer = owner.getAudioPlayer();
            auto audioLength = audioPlayer.getThumbnail().getTotalLength();
            double mouseTime = CoordinateMapper::pixelsToSeconds((float)(event.x - waveformBounds.getX()), 
                                                                 (float)waveformBounds.getWidth(), 
                                                                 audioLength);

            dragStartMouseOffset = mouseTime - owner.getCutInPosition();
            owner.repaint();
        }
        else if (draggedHandle == CutMarkerHandle::None)
        {
            isDragging = true;
            isScrubbingState = true;
            mouseDragStartX = event.x;
            currentPlaybackPosOnDragStart = owner.getAudioPlayer().getCurrentPosition();

            seekToMousePosition(event.x);
        }
        else
        {
            owner.repaint();
        }
    }
    else if (event.mods.isRightButtonDown())
    {

        handleRightClickForCutPlacement(event.x);
    }
}

void MouseHandler::mouseDrag(const juce::MouseEvent& event)
{
    const auto waveformBounds = owner.getWaveformBounds();
    if (!event.mods.isLeftButtonDown())
        return;

    AudioPlayer& audioPlayer = owner.getAudioPlayer();
    auto audioLength = audioPlayer.getThumbnail().getTotalLength();

    if (waveformBounds.contains(event.getPosition()))
    {
        mouseCursorX = event.x;
        mouseCursorY = event.y;
        if (audioLength > 0.0)
        {
            mouseCursorTime = CoordinateMapper::pixelsToSeconds((float)(mouseCursorX - waveformBounds.getX()), 
                                                                (float)waveformBounds.getWidth(), 
                                                                audioLength);
        }
    }

    if (interactionStartedInZoom && owner.getActiveZoomPoint() != AppEnums::ActiveZoomPoint::None && (draggedHandle != CutMarkerHandle::None || isDragging))
    {
        auto zoomBounds = owner.getZoomPopupBounds();
        if (zoomBounds.contains(event.getPosition()) || draggedHandle != CutMarkerHandle::None || isDragging)
        {
            auto timeRange = owner.getZoomTimeRange();
            int clampedX = juce::jlimit(zoomBounds.getX(), zoomBounds.getRight(), event.x);
            double zoomedTime = CoordinateMapper::pixelsToSeconds((float)(clampedX - zoomBounds.getX()), 
                                                                  (float)zoomBounds.getWidth(), 
                                                                  timeRange.second - timeRange.first) + timeRange.first;

            if (draggedHandle != CutMarkerHandle::None)
            {
                double offset = (currentPlacementMode == AppEnums::PlacementMode::None) ? dragStartMouseOffset : 0.0;

                if (draggedHandle == CutMarkerHandle::In)
                    owner.getAudioPlayer().setCutIn(zoomedTime - offset);
                else if (draggedHandle == CutMarkerHandle::Out)
                    owner.getAudioPlayer().setCutOut(zoomedTime - offset);
                owner.ensureCutOrder();
            }
            else if (isDragging)
            {
                owner.getAudioPlayer().setPlayheadPosition(zoomedTime);
            }

            owner.updateCutLabels();
            owner.repaint();
            return;
        }
    }

    if (draggedHandle != CutMarkerHandle::None)
    {
        if (audioLength > 0.0)
        {
            int clampedX = juce::jlimit(waveformBounds.getX(), waveformBounds.getRight(), event.x);
            double mouseTime = CoordinateMapper::pixelsToSeconds((float)(clampedX - waveformBounds.getX()), 
                                                                 (float)waveformBounds.getWidth(), 
                                                                 audioLength);

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

                audioPlayer.setPlayheadPosition(audioPlayer.getCurrentPosition());
            }

            owner.ensureCutOrder();
            owner.updateCutLabels();
            owner.repaint();
        }
    }
    else if (isDragging && waveformBounds.contains(event.getPosition()))
    {

        seekToMousePosition(event.x);
        owner.repaint();
    }
}

void MouseHandler::mouseUp(const juce::MouseEvent& event)
{
    if (owner.getActiveZoomPoint() != AppEnums::ActiveZoomPoint::None && (isDragging || draggedHandle != CutMarkerHandle::None || currentPlacementMode != AppEnums::PlacementMode::None))
    {
        if (currentPlacementMode != AppEnums::PlacementMode::None)
        {
            currentPlacementMode = AppEnums::PlacementMode::None;
            owner.updateCutButtonColors();
        }
        isDragging = false;
        isScrubbingState = false;
        draggedHandle = CutMarkerHandle::None;
        owner.repaint();
        return;
    }

    isDragging = false;
    isScrubbingState = false;
    draggedHandle = CutMarkerHandle::None;
    owner.jumpToCutIn();

    const auto waveformBounds = owner.getWaveformBounds();
    if (waveformBounds.contains(event.getPosition()) && event.mods.isLeftButtonDown())
    {
        if (currentPlacementMode != AppEnums::PlacementMode::None)
        {
            AudioPlayer& audioPlayer = owner.getAudioPlayer();
            auto audioLength = audioPlayer.getThumbnail().getTotalLength();
            if (audioLength > 0.0)
            {
                double time = CoordinateMapper::pixelsToSeconds((float)(event.x - waveformBounds.getX()), 
                                                                (float)waveformBounds.getWidth(), 
                                                                audioLength);

                if (currentPlacementMode == AppEnums::PlacementMode::CutIn)
                {
                    owner.setCutInPosition(time);
                    owner.setAutoCutInActive(false);
                }
                else if (currentPlacementMode == AppEnums::PlacementMode::CutOut)
                {
                    owner.setCutOutPosition(time);
                    owner.setAutoCutOutActive(false);
                }
                owner.ensureCutOrder();
                owner.updateCutLabels();
                owner.jumpToCutIn();
            }
            currentPlacementMode = AppEnums::PlacementMode::None;
            owner.updateCutButtonColors();
            owner.repaint();
        }
        else if (mouseDragStartX == event.x)
        {

            seekToMousePosition(event.x);
        }
    }
}

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

    if (event.mods.isCtrlDown() && !event.mods.isShiftDown())
    {
        float currentZoom = owner.getZoomFactor();
        float zoomDelta = wheel.deltaY > 0 ? 1.1f : 0.9f;
        owner.setZoomFactor(currentZoom * zoomDelta);
        return;
    }

    AudioPlayer& audioPlayer = owner.getAudioPlayer();
    double currentPos = audioPlayer.getCurrentPosition();
    double multiplier = FocusManager::getStepMultiplier(event.mods.isShiftDown(), event.mods.isCtrlDown());
    double step = 0.01 * multiplier;

    if (event.mods.isAltDown())
        step *= 10.0;

    double direction = (wheel.deltaY > 0) ? 1.0 : -1.0;
    double newPos = currentPos + (direction * step);

    audioPlayer.setPlayheadPosition(newPos);
    owner.repaint();
}

void MouseHandler::handleRightClickForCutPlacement(int x)
{
    AudioPlayer& audioPlayer = owner.getAudioPlayer();
    auto audioLength = audioPlayer.getThumbnail().getTotalLength();
    if (audioLength <= 0.0) return;

    const auto waveformBounds = owner.getWaveformBounds();
    double time = CoordinateMapper::pixelsToSeconds((float)(x - waveformBounds.getX()), 
                                                    (float)waveformBounds.getWidth(), 
                                                    audioLength);

    if (currentPlacementMode == AppEnums::PlacementMode::CutIn)
    {
        owner.setCutInPosition(time);
        owner.setAutoCutInActive(false);
    }
    else if (currentPlacementMode == AppEnums::PlacementMode::CutOut)
    {
        owner.setCutOutPosition(time);
        owner.setAutoCutOutActive(false);
    }
    owner.ensureCutOrder();
    owner.updateCutButtonColors();
    owner.updateCutLabels();
    owner.repaint();
}

void MouseHandler::seekToMousePosition(int x)
{
    AudioPlayer& audioPlayer = owner.getAudioPlayer();
    auto audioLength = audioPlayer.getThumbnail().getTotalLength();

    const auto waveformBounds = owner.getWaveformBounds();
    double time = CoordinateMapper::pixelsToSeconds((float)(x - waveformBounds.getX()), 
                                                    (float)waveformBounds.getWidth(), 
                                                    audioLength);

    audioPlayer.setPlayheadPosition(time);
}

void MouseHandler::clearTextEditorFocusIfNeeded(const juce::MouseEvent& event)
{
    const auto screenPos = event.getScreenPosition();

    for (int i = 0; i < owner.getNumChildComponents(); ++i)
    {
        auto* child = owner.getChildComponent(i);
        if (auto* editorChild = dynamic_cast<juce::TextEditor*>(child))
        {
            if (editorChild->getScreenBounds().contains(screenPos))
                return;
        }
    }

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
        float x = (float)waveformBounds.getX() + 
                  CoordinateMapper::secondsToPixels(time, (float)waveformBounds.getWidth(), audioLength);

        juce::Rectangle<int> hitStrip((int)(x - Config::Layout::Glow::cutMarkerBoxWidth / 2.0f),
                                      waveformBounds.getY(), 
                                      (int)Config::Layout::Glow::cutMarkerBoxWidth,
                                      waveformBounds.getHeight());

        return hitStrip.contains(pos);
    };

    if (checkHandle(owner.getCutInPosition())) return CutMarkerHandle::In;
    if (checkHandle(owner.getCutOutPosition())) return CutMarkerHandle::Out;

    const double cutIn = owner.getCutInPosition();
    const double cutOut = owner.getCutOutPosition();
    const double actualIn = juce::jmin(cutIn, cutOut);
    const double actualOut = juce::jmax(cutIn, cutOut);

    float inX = (float)waveformBounds.getX() + 
                CoordinateMapper::secondsToPixels(actualIn, (float)waveformBounds.getWidth(), audioLength);
    float outX = (float)waveformBounds.getX() + 
                 CoordinateMapper::secondsToPixels(actualOut, (float)waveformBounds.getWidth(), audioLength);

    int hollowHeight = Config::Layout::Glow::cutMarkerBoxHeight;

    juce::Rectangle<int> topHollow((int)inX, waveformBounds.getY(), (int)(outX - inX), hollowHeight);
    juce::Rectangle<int> bottomHollow((int)inX, waveformBounds.getBottom() - hollowHeight, (int)(outX - inX), hollowHeight);

    if (topHollow.contains(pos) || bottomHollow.contains(pos))
        return CutMarkerHandle::Full;

    return CutMarkerHandle::None;
}
