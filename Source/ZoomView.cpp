/**
 * @file ZoomView.cpp
 * @brief Defines the ZoomView class.
 * @ingroup Views
 */

#include "ZoomView.h"
#include "ControlPanel.h"
#include "AudioPlayer.h"
#include "CoordinateMapper.h"
#include "FocusManager.h"
#include "MouseHandler.h"
#include "Config.h"
#include "PlaybackCursorGlow.h"

ZoomView::ZoomView(ControlPanel& ownerIn)
    : owner(ownerIn)
{
    /**
     * @brief Sets the InterceptsMouseClicks.
     * @param false [in] Description for false.
     * @param false [in] Description for false.
     */
    setInterceptsMouseClicks(false, false);
    /**
     * @brief Sets the Opaque.
     * @param false [in] Description for false.
     */
    setOpaque(false);
}

void ZoomView::updateZoomState()
{
    const auto& mouse = owner.getMouseHandler();
    const int currentMouseX = mouse.getMouseCursorX();
    const int currentMouseY = mouse.getMouseCursorY();

    const bool zDown = owner.isZKeyDown();
    const auto activePoint = owner.getActiveZoomPoint();
    const bool isZooming = zDown || activePoint != ControlPanel::ActiveZoomPoint::None;

    // 1. Handle Mouse Lines Repaint
    if (currentMouseX != lastMouseX || currentMouseY != lastMouseY)
    {
        if (lastMouseX != -1)
        {
            repaint(lastMouseX - 1, 0, 3, getHeight());
            repaint(0, lastMouseY - 1, getWidth(), 3);
        }
        
        if (currentMouseX != -1)
        {
            repaint(currentMouseX - 1, 0, 3, getHeight());
            repaint(0, currentMouseY - 1, getWidth(), 3);
        }

        lastMouseX = currentMouseX;
        lastMouseY = currentMouseY;
    }

    // 2. Handle Zoom Popup Repaint
    if (isZooming)
    {
        const auto waveformBounds = getLocalBounds();
        const int popupWidth = juce::roundToInt((float)waveformBounds.getWidth() * Config::Layout::Zoom::popupScale);
        const int popupHeight = juce::roundToInt((float)waveformBounds.getHeight() * Config::Layout::Zoom::popupScale);
        const juce::Rectangle<int> currentPopupBounds(
            waveformBounds.getCentreX() - popupWidth / 2,
            waveformBounds.getCentreY() - popupHeight / 2,
            popupWidth,
            popupHeight
        );

        if (currentPopupBounds != lastPopupBounds)
        {
            repaint(lastPopupBounds.expanded(5));
            repaint(currentPopupBounds.expanded(5));
            lastPopupBounds = currentPopupBounds;
        }
        else
        {
            // Even if bounds didn't change, we might need to update the internal waveform or playhead
            repaint(currentPopupBounds.expanded(5));
        }
    }
    else if (!lastPopupBounds.isEmpty())
    {
        repaint(lastPopupBounds.expanded(5));
        lastPopupBounds = juce::Rectangle<int>();
    }
}

void ZoomView::paint(juce::Graphics& g)
{
    auto& audioPlayer = owner.getAudioPlayer();
    const double audioLength = audioPlayer.getWaveformManager().getThumbnail().getTotalLength();
    if (audioLength <= 0.0)
        return;

    const auto waveformBounds = getLocalBounds();
    const auto& mouse = owner.getMouseHandler();

    // 1. Draw Mouse Cursor Overlays (Top-most dynamic guides)
    if (mouse.getMouseCursorX() != -1)
    {
        // Translate mouse position from ControlPanel coordinates to local ZoomView coordinates
        const int localMouseX = mouse.getMouseCursorX() - getX();
        const int localMouseY = mouse.getMouseCursorY() - getY();

        juce::Colour currentLineColor;
        juce::Colour currentHighlightColor;
        juce::Colour currentGlowColor;
        float currentGlowThickness = 0.0f;

        if (mouse.getCurrentPlacementMode() == AppEnums::PlacementMode::CutIn
            || mouse.getCurrentPlacementMode() == AppEnums::PlacementMode::CutOut)
        {
            currentLineColor = Config::Colors::mousePlacementMode;
            currentHighlightColor = Config::Colors::mousePlacementMode.withAlpha(0.4f);
            currentGlowColor = Config::Colors::placementModeGlow;
            currentGlowThickness = Config::Layout::Glow::placementModeGlowThickness;

            g.setColour(currentGlowColor.withAlpha(Config::Layout::Glow::mouseAlpha));
            g.fillRect(localMouseX - (int)(currentGlowThickness * Config::Layout::Glow::offsetFactor) - 1, waveformBounds.getY(),
                       (int)currentGlowThickness + Config::Layout::Glow::mousePadding, waveformBounds.getHeight());
            g.fillRect(waveformBounds.getX(), localMouseY - (int)(currentGlowThickness * Config::Layout::Glow::offsetFactor) - 1,
                       waveformBounds.getWidth(), (int)currentGlowThickness + Config::Layout::Glow::mousePadding);
        }
        else
        {
            if (owner.isZKeyDown())
            {
                currentLineColor = Config::Colors::mousePlacementMode;
                currentHighlightColor = Config::Colors::mousePlacementMode.withAlpha(0.4f);
            }
            else
            {
                currentLineColor = Config::Colors::mouseCursorLine;
                currentHighlightColor = Config::Colors::mouseCursorHighlight;
            }
            currentGlowColor = Config::Colors::mouseAmplitudeGlow;
        }

        g.setColour(currentHighlightColor);
        g.fillRect(localMouseX - Config::Layout::Glow::mouseHighlightOffset, waveformBounds.getY(), Config::Layout::Glow::mouseHighlightSize, waveformBounds.getHeight());
        g.fillRect(waveformBounds.getX(), localMouseY - Config::Layout::Glow::mouseHighlightOffset, waveformBounds.getWidth(), Config::Layout::Glow::mouseHighlightSize);

        // Amplitude and Time labels
        float amplitude = 0.0f;
        if (audioPlayer.getWaveformManager().getThumbnail().getNumChannels() > 0)
        {
            double sampleRate = 0.0;
            juce::int64 length = 0;
            if (audioPlayer.getReaderInfo(sampleRate, length) && sampleRate > 0.0)
            {
                float minVal = 0.0f, maxVal = 0.0f;
                audioPlayer.getWaveformManager().getThumbnail().getApproximateMinMax(mouse.getMouseCursorTime(),
                                                                                     mouse.getMouseCursorTime() + (1.0 / sampleRate),
                                                                                     0, minVal, maxVal);
                amplitude = juce::jmax(std::abs(minVal), std::abs(maxVal));
            }
        }

        const float centerY = (float)waveformBounds.getCentreY();
        const float amplitudeY = centerY - (amplitude * waveformBounds.getHeight() * Config::Layout::Waveform::heightScale);
        const float bottomAmplitudeY = centerY + (amplitude * waveformBounds.getHeight() * Config::Layout::Waveform::heightScale);

        juce::ColourGradient amplitudeGlowGradient(currentGlowColor.withAlpha(0.0f), (float)localMouseX, amplitudeY,
                                                   currentGlowColor.withAlpha(Config::Layout::Glow::mouseAmplitudeAlpha), (float)localMouseX, centerY, true);
        g.setGradientFill(amplitudeGlowGradient);
        g.fillRect(juce::Rectangle<float>((float)localMouseX - Config::Layout::Glow::mouseAmplitudeGlowThickness * Config::Layout::Glow::offsetFactor, amplitudeY,
                                          Config::Layout::Glow::mouseAmplitudeGlowThickness, centerY - amplitudeY));
        g.fillRect(juce::Rectangle<float>((float)localMouseX - Config::Layout::Glow::mouseAmplitudeGlowThickness * Config::Layout::Glow::offsetFactor, centerY,
                                          Config::Layout::Glow::mouseAmplitudeGlowThickness, centerY - amplitudeY));

        g.setColour(Config::Colors::mouseAmplitudeLine);
        g.drawVerticalLine(localMouseX, amplitudeY, bottomAmplitudeY);

        const float halfLineLength = Config::Animation::mouseAmplitudeLineLength * Config::Layout::Glow::offsetFactor;
        const float leftExtent = (float)localMouseX - halfLineLength;
        const float rightExtent = (float)localMouseX + halfLineLength;
        g.drawHorizontalLine(juce::roundToInt(amplitudeY), leftExtent, rightExtent);
        g.drawHorizontalLine(juce::roundToInt(bottomAmplitudeY), leftExtent, rightExtent);

        g.setColour(Config::Colors::playbackText);
        g.setFont(Config::Layout::Text::mouseCursorSize);
        g.drawText(juce::String(amplitude, 2), localMouseX + Config::Layout::Glow::mouseTextOffset, (int)amplitudeY - Config::Layout::Text::mouseCursorSize,
                   100, Config::Layout::Text::mouseCursorSize, juce::Justification::left, true);
        g.drawText(juce::String(-amplitude, 2), localMouseX + Config::Layout::Glow::mouseTextOffset, (int)bottomAmplitudeY,
                   100, Config::Layout::Text::mouseCursorSize, juce::Justification::left, true);

        const juce::String timeText = owner.formatTime(mouse.getMouseCursorTime());
        g.drawText(timeText, localMouseX + Config::Layout::Glow::mouseTextOffset, localMouseY + Config::Layout::Glow::mouseTextOffset, 100,
                   Config::Layout::Text::mouseCursorSize, juce::Justification::left, true);

        PlaybackCursorGlow::renderGlow(g, localMouseX, waveformBounds.getY(), waveformBounds.getBottom(), currentLineColor);
        g.setColour(currentLineColor);
        g.drawHorizontalLine(localMouseY, (float)waveformBounds.getX(), (float)waveformBounds.getRight());
    }

    // 2. Draw Zoom Popup
    const bool zDown = owner.isZKeyDown();
    const auto activePoint = owner.getActiveZoomPoint();

    if (zDown || activePoint != ControlPanel::ActiveZoomPoint::None)
    {
        const int popupWidth = juce::roundToInt((float)waveformBounds.getWidth() * Config::Layout::Zoom::popupScale);
        const int popupHeight = juce::roundToInt((float)waveformBounds.getHeight() * Config::Layout::Zoom::popupScale);
        const juce::Rectangle<int> popupBounds(
            waveformBounds.getCentreX() - popupWidth / 2,
            waveformBounds.getCentreY() - popupHeight / 2,
            popupWidth,
            popupHeight
        );

        double zoomCenterTime = owner.getFocusManager().getFocusedTime();
        double timeRange = audioLength / (double)owner.getZoomFactor();
        timeRange = juce::jlimit(0.00005, audioLength, timeRange);

        const double startTime = zoomCenterTime - (timeRange / 2.0);
        const double endTime = startTime + timeRange;

        // Sync with owner for interaction logic (mouse events in zoom)
        owner.setZoomPopupBounds(popupBounds.translated(getX(), getY()));
        owner.setZoomTimeRange(startTime, endTime);

        g.setColour(juce::Colours::black);
        g.fillRect(popupBounds);

        g.setColour(Config::Colors::waveform);
        const auto channelMode = owner.getChannelViewMode();
        const int numChannels = audioPlayer.getWaveformManager().getThumbnail().getNumChannels();

        if (channelMode == AppEnums::ChannelViewMode::Mono || numChannels == 1)
        {
            audioPlayer.getWaveformManager().getThumbnail().drawChannel(g, popupBounds, startTime, endTime, 0, 1.0f);
            g.setColour(Config::Colors::zoomPopupZeroLine);
            g.drawHorizontalLine(popupBounds.getCentreY(), (float)popupBounds.getX(), (float)popupBounds.getRight());
        }
        else
        {
            auto topBounds = popupBounds.withHeight(popupBounds.getHeight() / 2);
            auto bottomBounds = popupBounds.withTop(topBounds.getBottom()).withHeight(popupBounds.getHeight() / 2);

            audioPlayer.getWaveformManager().getThumbnail().drawChannel(g, topBounds, startTime, endTime, 0, 1.0f);
            audioPlayer.getWaveformManager().getThumbnail().drawChannel(g, bottomBounds, startTime, endTime, 1, 1.0f);

            g.setColour(Config::Colors::zoomPopupZeroLine);
            g.drawHorizontalLine(topBounds.getCentreY(), (float)topBounds.getX(), (float)topBounds.getRight());
            g.drawHorizontalLine(bottomBounds.getCentreY(), (float)bottomBounds.getX(), (float)bottomBounds.getRight());
        }

        auto drawShadow = [&](double startT, double endT, juce::Colour color) {
            if (endT <= startTime || startT >= endTime) return;
            double vStart = juce::jmax(startT, startTime);
            double vEnd = juce::jmin(endT, endTime);
            float x1 = (float)popupBounds.getX() + CoordinateMapper::secondsToPixels(vStart - startTime, (float)popupBounds.getWidth(), timeRange);
            float x2 = (float)popupBounds.getX() + CoordinateMapper::secondsToPixels(vEnd - startTime, (float)popupBounds.getWidth(), timeRange);
            g.setColour(color);
            g.fillRect(x1, (float)popupBounds.getY(), x2 - x1, (float)popupBounds.getHeight());
        };

        const double cutIn = owner.getCutInPosition();
        const double cutOut = owner.getCutOutPosition();

        drawShadow(startTime, cutIn, juce::Colours::black.withAlpha(0.5f));
        drawShadow(cutOut, endTime, juce::Colours::black.withAlpha(0.5f));

        if (startTime < 0.0)
            /**
             * @brief Undocumented method.
             * @param startTime [in] Description for startTime.
             * @param 0.0 [in] Description for 0.0.
             * @param juce::Colours::black [in] Description for juce::Colours::black.
             */
            drawShadow(startTime, 0.0, juce::Colours::black);
        if (endTime > audioLength)
            /**
             * @brief Undocumented method.
             * @param audioLength [in] Description for audioLength.
             * @param endTime [in] Description for endTime.
             * @param juce::Colours::black [in] Description for juce::Colours::black.
             */
            drawShadow(audioLength, endTime, juce::Colours::black);

        auto drawFineLine = [&](double time, juce::Colour color, float thickness) {
            if (time >= startTime && time <= endTime) {
                float x = (float)popupBounds.getX() + CoordinateMapper::secondsToPixels(time - startTime, (float)popupBounds.getWidth(), timeRange);
                g.setColour(color);
                g.drawLine(x, (float)popupBounds.getY(), x, (float)popupBounds.getBottom(), thickness);
            }
        };

        bool isDraggingCutIn = mouse.getDraggedHandle() == MouseHandler::CutMarkerHandle::In;
        bool isDraggingCutOut = mouse.getDraggedHandle() == MouseHandler::CutMarkerHandle::Out;

        /**
         * @brief Undocumented method.
         * @param cutIn [in] Description for cutIn.
         * @param Config::Colors::cutLine [in] Description for Config::Colors::cutLine.
         * @param 1.0f [in] Description for 1.0f.
         */
        drawFineLine(cutIn, Config::Colors::cutLine, 1.0f);
        /**
         * @brief Undocumented method.
         * @param cutOut [in] Description for cutOut.
         * @param Config::Colors::cutLine [in] Description for Config::Colors::cutLine.
         * @param 1.0f [in] Description for 1.0f.
         */
        drawFineLine(cutOut, Config::Colors::cutLine, 1.0f);
        drawFineLine(audioPlayer.getCurrentPosition(), Config::Colors::playbackCursor, 1.0f);

        if (isDraggingCutIn || isDraggingCutOut)
            /**
             * @brief Undocumented method.
             * @param cutOut [in] Description for cutOut.
             * @param Config::Colors::zoomPopupTrackingLine [in] Description for Config::Colors::zoomPopupTrackingLine.
             * @param 2.0f [in] Description for 2.0f.
             */
            drawFineLine(isDraggingCutIn ? cutIn : cutOut, Config::Colors::zoomPopupTrackingLine, 2.0f);
        else
            drawFineLine(audioPlayer.getCurrentPosition(), Config::Colors::zoomPopupPlaybackLine, 2.0f);

        g.setColour(Config::Colors::zoomPopupBorder);
        g.drawRect(popupBounds.toFloat(), Config::Layout::Zoom::borderThickness);
    }
}
