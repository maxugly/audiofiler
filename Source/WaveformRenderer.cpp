#include "WaveformRenderer.h"

#include "ControlPanel.h"
#include "AudioPlayer.h"
#include "MouseHandler.h"
#include "SilenceDetector.h"
#include "Config.h"
#include "PlaybackCursorGlow.h"

WaveformRenderer::WaveformRenderer(ControlPanel& controlPanelIn)
    : controlPanel(controlPanelIn)
{
}

void WaveformRenderer::render(juce::Graphics& g)
{
    AudioPlayer& audioPlayer = controlPanel.getAudioPlayer();
    drawWaveform(g, audioPlayer);

    const float audioLength = (float)audioPlayer.getThumbnail().getTotalLength();
    if (audioLength > 0.0f)
    {
        if (controlPanel.isCutModeActive())
            drawCutModeOverlays(g, audioPlayer, audioLength);

        drawPlaybackCursor(g, audioPlayer, audioLength);
    }

    drawMouseCursorOverlays(g, audioPlayer, audioLength);
    drawZoomPopup(g);
}

void WaveformRenderer::drawWaveform(juce::Graphics& g, AudioPlayer& audioPlayer) const
{
    const auto numChannels = audioPlayer.getThumbnail().getNumChannels();
    if (numChannels <= 0)
        return;

    int pixelsPerSample = 1;
    if (controlPanel.getCurrentQualitySetting() == AppEnums::ThumbnailQuality::Low)
        pixelsPerSample = Config::pixelsPerSampleLow;
    else if (controlPanel.getCurrentQualitySetting() == AppEnums::ThumbnailQuality::Medium)
        pixelsPerSample = Config::pixelsPerSampleMedium;

    g.setColour(Config::waveformColor);
    if (controlPanel.getChannelViewMode() == AppEnums::ChannelViewMode::Mono || numChannels == 1)
    {
        if (pixelsPerSample > 1)
            drawReducedQualityWaveform(g, audioPlayer, 0, pixelsPerSample);
        else
            audioPlayer.getThumbnail().drawChannel(g, controlPanel.getWaveformBounds(), 0.0, audioPlayer.getThumbnail().getTotalLength(), 0, 1.0f);
        return;
    }

    if (pixelsPerSample > 1)
    {
        for (int ch = 0; ch < numChannels; ++ch)
            drawReducedQualityWaveform(g, audioPlayer, ch, pixelsPerSample);
        return;
    }

    audioPlayer.getThumbnail().drawChannels(g, controlPanel.getWaveformBounds(), 0.0, audioPlayer.getThumbnail().getTotalLength(), 1.0f);
}

void WaveformRenderer::drawReducedQualityWaveform(juce::Graphics& g, AudioPlayer& audioPlayer, int channel, int pixelsPerSample) const
{
    const auto audioLength = audioPlayer.getThumbnail().getTotalLength();
    if (audioLength <= 0.0)
        return;

    const auto waveformBounds = controlPanel.getWaveformBounds();
    const int width = waveformBounds.getWidth();
    const int height = waveformBounds.getHeight();
    const int centerY = waveformBounds.getCentreY();

    for (int x = 0; x < width; x += pixelsPerSample)
    {
        const double proportion = (double)x / (double)width;
        const double time = proportion * audioLength;
        float minVal = 0.0f, maxVal = 0.0f;
        audioPlayer.getThumbnail().getApproximateMinMax(time, time + (audioLength / width) * pixelsPerSample, channel, minVal, maxVal);
        const auto topY = (float)centerY - (maxVal * height * Config::waveformHeightScale);
        const auto bottomY = (float)centerY - (minVal * height * Config::waveformHeightScale);
        g.drawVerticalLine(waveformBounds.getX() + x, topY, bottomY);
    }
}

void WaveformRenderer::drawCutModeOverlays(juce::Graphics& g, AudioPlayer& audioPlayer, float audioLength) const
{
    const auto waveformBounds = controlPanel.getWaveformBounds();
    const auto& silenceDetector = controlPanel.getSilenceDetector();

    auto drawThresholdVisualisation = [&](double loopPos, float threshold)
    {
        if (audioLength <= 0.0f)
            return;

        const float normalisedThreshold = threshold;
        const float centerY = (float)waveformBounds.getCentreY();
        const float halfHeight = (float)waveformBounds.getHeight() / 2.0f;

        float topThresholdY = centerY - (normalisedThreshold * halfHeight);
        float bottomThresholdY = centerY + (normalisedThreshold * halfHeight);

        topThresholdY = juce::jlimit((float)waveformBounds.getY(), (float)waveformBounds.getBottom(), topThresholdY);
        bottomThresholdY = juce::jlimit((float)waveformBounds.getY(), (float)waveformBounds.getBottom(), bottomThresholdY);

        const float xPos = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (loopPos / audioLength);
        const float halfThresholdLineWidth = Config::thresholdLineWidth / 2.0f;
        float lineStartX = xPos - halfThresholdLineWidth;
        float lineEndX = xPos + halfThresholdLineWidth;

        lineStartX = juce::jmax(lineStartX, (float)waveformBounds.getX());
        lineEndX = juce::jmin(lineEndX, (float)waveformBounds.getRight());
        const float currentLineWidth = lineEndX - lineStartX;

        g.setColour(Config::thresholdRegionColor);
        g.fillRect(lineStartX, topThresholdY, currentLineWidth, bottomThresholdY - topThresholdY);

        if (controlPanel.isCutModeActive())
        {
            const juce::Colour glowColor = Config::thresholdLineColor.withAlpha(Config::thresholdLineColor.getFloatAlpha() * controlPanel.getGlowAlpha());
            g.setColour(glowColor);
            g.fillRect(lineStartX, topThresholdY - (Config::thresholdGlowThickness * Config::glowOffsetFactor - 0.5f), currentLineWidth, Config::thresholdGlowThickness);
            g.fillRect(lineStartX, bottomThresholdY - (Config::thresholdGlowThickness * Config::glowOffsetFactor - 0.5f), currentLineWidth, Config::thresholdGlowThickness);
        }

        g.setColour(Config::thresholdLineColor);
        g.drawHorizontalLine((int)topThresholdY, lineStartX, lineEndX);
        g.drawHorizontalLine((int)bottomThresholdY, lineStartX, lineEndX);
    };

    drawThresholdVisualisation(controlPanel.getLoopInPosition(), silenceDetector.getCurrentInSilenceThreshold());
    drawThresholdVisualisation(controlPanel.getLoopOutPosition(), silenceDetector.getCurrentOutSilenceThreshold());

    const double actualIn = juce::jmin(controlPanel.getLoopInPosition(), controlPanel.getLoopOutPosition());
    const double actualOut = juce::jmax(controlPanel.getLoopInPosition(), controlPanel.getLoopOutPosition());
    const float inX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (actualIn / audioLength);
    const float outX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (actualOut / audioLength);
    const float fadeLength = waveformBounds.getWidth() * Config::waveBoxHaze;
    const float boxHeight = (float)Config::loopMarkerBoxHeight;

    const juce::Rectangle<float> leftRegion((float)waveformBounds.getX(), (float)waveformBounds.getY(), inX - (float)waveformBounds.getX(), (float)waveformBounds.getHeight());
    if (leftRegion.getWidth() > 0.0f)
    {
        const float actualFade = juce::jmin(fadeLength, leftRegion.getWidth());
        
        // 1. Black out the area beyond the fade
        juce::Rectangle<float> solidBlackLeft = leftRegion.withWidth(leftRegion.getWidth() - actualFade);
        g.setColour(juce::Colours::black);
        g.fillRect(solidBlackLeft);

        // 2. Fade from blue loop color to black
        juce::Rectangle<float> fadeAreaLeft(inX - actualFade, (float)waveformBounds.getY(), actualFade, (float)waveformBounds.getHeight());
        juce::ColourGradient leftFadeGradient(Config::loopRegionColor, inX, leftRegion.getCentreY(),
                                              juce::Colours::black, inX - actualFade, leftRegion.getCentreY(), false);
        g.setGradientFill(leftFadeGradient);
        g.fillRect(fadeAreaLeft);
    }

    const juce::Rectangle<float> rightRegion(outX, (float)waveformBounds.getY(), (float)waveformBounds.getRight() - outX, (float)waveformBounds.getHeight());
    if (rightRegion.getWidth() > 0.0f)
    {
        const float actualFade = juce::jmin(fadeLength, rightRegion.getWidth());

        // 1. Black out the area beyond the fade
        float solidBlackStart = outX + actualFade;
        juce::Rectangle<float> solidBlackRight(solidBlackStart, (float)waveformBounds.getY(), (float)waveformBounds.getRight() - solidBlackStart, (float)waveformBounds.getHeight());
        g.setColour(juce::Colours::black);
        g.fillRect(solidBlackRight);

        // 2. Fade from blue loop color to black
        juce::Rectangle<float> fadeAreaRight(outX, (float)waveformBounds.getY(), actualFade, (float)waveformBounds.getHeight());
        juce::ColourGradient rightFadeGradient(Config::loopRegionColor, outX, rightRegion.getCentreY(),
                                               juce::Colours::black, outX + actualFade, rightRegion.getCentreY(), false);
        g.setGradientFill(rightFadeGradient);
        g.fillRect(fadeAreaRight);
    }

    const juce::Colour glowColor = Config::loopLineColor.withAlpha(Config::loopLineColor.getFloatAlpha() * (1.0f - controlPanel.getGlowAlpha()));
    g.setColour(glowColor);
    g.fillRect(inX - (Config::loopLineGlowThickness * Config::glowOffsetFactor - 0.5f), (float)waveformBounds.getY() + boxHeight, Config::loopLineGlowThickness, (float)waveformBounds.getHeight() - (2.0f * boxHeight));
    g.fillRect(outX - (Config::loopLineGlowThickness * Config::glowOffsetFactor - 0.5f), (float)waveformBounds.getY() + boxHeight, Config::loopLineGlowThickness, (float)waveformBounds.getHeight() - (2.0f * boxHeight));

    g.setColour(Config::loopLineColor);
    auto drawLoopMarker = [&](float x, MouseHandler::LoopMarkerHandle handleType) {
        const auto& mouseHandler = controlPanel.getMouseHandler();
        const auto& silenceDetector = controlPanel.getSilenceDetector();
        
        juce::Colour markerColor = Config::loopLineColor;
        
        // Base color based on Auto-Cut status
        if (handleType == MouseHandler::LoopMarkerHandle::In && silenceDetector.getIsAutoCutInActive())
            markerColor = Config::loopMarkerAutoColor;
        else if (handleType == MouseHandler::LoopMarkerHandle::Out && silenceDetector.getIsAutoCutOutActive())
            markerColor = Config::loopMarkerAutoColor;

        float thickness = Config::loopBoxOutlineThickness;

        if (mouseHandler.getDraggedHandle() == handleType)
        {
            markerColor = Config::loopMarkerDragColor;
            thickness = Config::loopBoxOutlineThicknessInteracting;
        }
        else if (mouseHandler.getHoveredHandle() == handleType)
        {
            markerColor = Config::loopMarkerHoverColor;
            thickness = Config::loopBoxOutlineThicknessInteracting;
        }

        const float boxWidth = Config::loopMarkerBoxWidth;
        const float halfBoxWidth = boxWidth / 2.0f;

        // Draw the top hollow box
        g.setColour(markerColor);
        g.drawRect(x - halfBoxWidth, (float)waveformBounds.getY(), boxWidth, boxHeight, thickness);
        
        // Draw the bottom hollow box
        g.drawRect(x - halfBoxWidth, (float)waveformBounds.getBottom() - boxHeight, boxWidth, boxHeight, thickness);

        // Draw the thin middle line between the boxes
        g.setColour(markerColor); // Use the marker color for the middle line too if auto/interacting
        g.fillRect(x - Config::loopMarkerWidthThin / Config::loopMarkerCenterDivisor, 
                   (float)waveformBounds.getY() + boxHeight, 
                   Config::loopMarkerWidthThin, 
                   (float)waveformBounds.getHeight() - (2.0f * boxHeight));
    };

    drawLoopMarker(inX, MouseHandler::LoopMarkerHandle::In);
    drawLoopMarker(outX, MouseHandler::LoopMarkerHandle::Out);

    const auto& mouseHandler = controlPanel.getMouseHandler();
    juce::Colour hollowColor = Config::loopLineColor;
    float thickness = Config::loopBoxOutlineThickness;

    if (mouseHandler.getDraggedHandle() == MouseHandler::LoopMarkerHandle::Full)
    {
        hollowColor = Config::loopMarkerDragColor;
        thickness = Config::loopBoxOutlineThicknessInteracting;
    }
    else if (mouseHandler.getHoveredHandle() == MouseHandler::LoopMarkerHandle::Full)
    {
        hollowColor = Config::loopMarkerHoverColor;
        thickness = Config::loopBoxOutlineThicknessInteracting;
    }

    g.setColour(hollowColor);
    const float halfBoxWidth = Config::loopMarkerBoxWidth / 2.0f;
    
    // Calculate the gap between the boxes
    const float startX = inX + halfBoxWidth;
    const float endX = outX - halfBoxWidth;

    if (startX < endX)
    {
        // Draw top box horizontal lines (terminating at the boxes)
        g.drawLine(startX, (float)waveformBounds.getY(), endX, (float)waveformBounds.getY(), thickness);
        g.drawLine(startX, (float)waveformBounds.getY() + boxHeight, endX, (float)waveformBounds.getY() + boxHeight, thickness);
        
        // Draw bottom box horizontal lines (terminating at the boxes)
        g.drawLine(startX, (float)waveformBounds.getBottom() - 1.0f, endX, (float)waveformBounds.getBottom() - 1.0f, thickness);
        g.drawLine(startX, (float)waveformBounds.getBottom() - boxHeight, endX, (float)waveformBounds.getBottom() - boxHeight, thickness);
    }
}

void WaveformRenderer::drawPlaybackCursor(juce::Graphics& g, AudioPlayer& audioPlayer, float audioLength) const
{
    const auto waveformBounds = controlPanel.getWaveformBounds();
    PlaybackCursorGlow::renderGlow(g, controlPanel, waveformBounds);
    g.setColour(Config::playbackCursorColor);
    const float drawPosition = (float)audioPlayer.getTransportSource().getCurrentPosition();
    const float x = (drawPosition / audioLength) * (float)waveformBounds.getWidth() + (float)waveformBounds.getX();
    g.drawVerticalLine((int)x, (float)waveformBounds.getY(), (float)waveformBounds.getBottom());
}

void WaveformRenderer::drawMouseCursorOverlays(juce::Graphics& g, AudioPlayer& audioPlayer, float audioLength) const
{
    const auto waveformBounds = controlPanel.getWaveformBounds();
    const auto& mouseHandler = controlPanel.getMouseHandler();
    if (mouseHandler.getMouseCursorX() == -1)
        return;

    juce::Colour currentLineColor;
    juce::Colour currentHighlightColor;
    juce::Colour currentGlowColor;
    float currentGlowThickness = 0.0f;

    if (mouseHandler.getCurrentPlacementMode() == AppEnums::PlacementMode::LoopIn
        || mouseHandler.getCurrentPlacementMode() == AppEnums::PlacementMode::LoopOut)
    {
        currentLineColor = Config::placementModeCursorColor;
        currentHighlightColor = Config::placementModeCursorColor.withAlpha(0.4f);
        currentGlowColor = Config::placementModeGlowColor;
        currentGlowThickness = Config::placementModeGlowThickness;

        g.setColour(currentGlowColor.withAlpha(Config::mouseGlowAlpha));
        g.fillRect(mouseHandler.getMouseCursorX() - (int)(currentGlowThickness * Config::glowOffsetFactor) - 1, waveformBounds.getY(),
                   (int)currentGlowThickness + Config::mouseGlowPadding, waveformBounds.getHeight());
        g.fillRect(waveformBounds.getX(), mouseHandler.getMouseCursorY() - (int)(currentGlowThickness * Config::glowOffsetFactor) - 1,
                   waveformBounds.getWidth(), (int)currentGlowThickness + Config::mouseGlowPadding);
    }
    else
    {
        currentLineColor = Config::mouseCursorLineColor;
        currentHighlightColor = Config::mouseCursorHighlightColor;
        currentGlowColor = Config::mouseAmplitudeGlowColor;
    }

    g.setColour(currentHighlightColor);
    g.fillRect(mouseHandler.getMouseCursorX() - Config::mouseHighlightOffset, waveformBounds.getY(), Config::mouseHighlightSize, waveformBounds.getHeight());
    g.fillRect(waveformBounds.getX(), mouseHandler.getMouseCursorY() - Config::mouseHighlightOffset, waveformBounds.getWidth(), Config::mouseHighlightSize);

    if (audioLength > 0.0f)
    {
        float amplitude = 0.0f;
        if (audioPlayer.getThumbnail().getNumChannels() > 0)
        {
            if (const auto* reader = audioPlayer.getAudioFormatReader())
            {
                const double sampleRate = reader->sampleRate;
                if (sampleRate > 0.0)
                {
                    float minVal = 0.0f, maxVal = 0.0f;
                    audioPlayer.getThumbnail().getApproximateMinMax(mouseHandler.getMouseCursorTime(),
                                                                    mouseHandler.getMouseCursorTime() + (1.0 / sampleRate),
                                                                    0, minVal, maxVal);
                    amplitude = juce::jmax(std::abs(minVal), std::abs(maxVal));
                }
            }
        }

        const float centerY = (float)waveformBounds.getCentreY();
        const float amplitudeY = centerY - (amplitude * waveformBounds.getHeight() * Config::waveformHeightScale);
        const float bottomAmplitudeY = centerY + (amplitude * waveformBounds.getHeight() * Config::waveformHeightScale);

        juce::ColourGradient amplitudeGlowGradient(currentGlowColor.withAlpha(0.0f), (float)mouseHandler.getMouseCursorX(), amplitudeY,
                                                   currentGlowColor.withAlpha(Config::mouseAmplitudeGlowAlpha), (float)mouseHandler.getMouseCursorX(), centerY, true);
        g.setGradientFill(amplitudeGlowGradient);
        g.fillRect(juce::Rectangle<float>((float)mouseHandler.getMouseCursorX() - Config::mouseAmplitudeGlowThickness * Config::glowOffsetFactor, amplitudeY,
                                          Config::mouseAmplitudeGlowThickness, centerY - amplitudeY));
        g.fillRect(juce::Rectangle<float>((float)mouseHandler.getMouseCursorX() - Config::mouseAmplitudeGlowThickness * Config::glowOffsetFactor, centerY,
                                          Config::mouseAmplitudeGlowThickness, centerY - amplitudeY));

        g.setColour(Config::mouseAmplitudeLineColor);
        g.drawVerticalLine(mouseHandler.getMouseCursorX(), amplitudeY, bottomAmplitudeY);

        const float halfLineLength = Config::mouseAmplitudeLineLength * Config::glowOffsetFactor;
        const float leftExtent = (float)mouseHandler.getMouseCursorX() - halfLineLength;
        const float rightExtent = (float)mouseHandler.getMouseCursorX() + halfLineLength;
        g.drawHorizontalLine(juce::roundToInt(amplitudeY), leftExtent, rightExtent);
        g.drawHorizontalLine(juce::roundToInt(bottomAmplitudeY), leftExtent, rightExtent);

        g.setColour(Config::playbackTextColor);
        g.setFont(Config::mouseCursorTextSize);
        g.drawText(juce::String(amplitude, 2), mouseHandler.getMouseCursorX() + Config::mouseTextOffset, (int)amplitudeY - Config::mouseCursorTextSize,
                   100, Config::mouseCursorTextSize, juce::Justification::left, true);
        g.drawText(juce::String(-amplitude, 2), mouseHandler.getMouseCursorX() + Config::mouseTextOffset, (int)bottomAmplitudeY,
                   100, Config::mouseCursorTextSize, juce::Justification::left, true);

        const juce::String timeText = controlPanel.formatTime(mouseHandler.getMouseCursorTime());
        g.drawText(timeText, mouseHandler.getMouseCursorX() + Config::mouseTextOffset, mouseHandler.getMouseCursorY() + Config::mouseTextOffset, 100,
                   Config::mouseCursorTextSize, juce::Justification::left, true);
    }

    g.setColour(currentLineColor);
    g.drawVerticalLine(mouseHandler.getMouseCursorX(), (float)waveformBounds.getY(), (float)waveformBounds.getBottom());
    g.drawHorizontalLine(mouseHandler.getMouseCursorY(), (float)waveformBounds.getX(), (float)waveformBounds.getRight());
}

void WaveformRenderer::drawZoomPopup(juce::Graphics& g) const
{
    const auto activePoint = controlPanel.getActiveZoomPoint();
    if (activePoint == ControlPanel::ActiveZoomPoint::None)
        return;

    AudioPlayer& audioPlayer = controlPanel.getAudioPlayer();
    const double audioLength = audioPlayer.getThumbnail().getTotalLength();
    if (audioLength <= 0.0)
        return;

    const auto waveformBounds = controlPanel.getWaveformBounds();
    
    // Calculate popup bounds: 80% of waveform area, centered
    const int popupWidth = juce::roundToInt((float)waveformBounds.getWidth() * Config::zoomPopupScale);
    const int popupHeight = juce::roundToInt((float)waveformBounds.getHeight() * Config::zoomPopupScale);
    const juce::Rectangle<int> popupBounds(
        waveformBounds.getCentreX() - popupWidth / 2,
        waveformBounds.getCentreY() - popupHeight / 2,
        popupWidth,
        popupHeight
    );

    const auto& mouseHandler = controlPanel.getMouseHandler();
    
    // Determine the center time for the zoom window
    double zoomCenterTime = 0.0;
    if (mouseHandler.getDraggedHandle() == MouseHandler::LoopMarkerHandle::In)
    {
        zoomCenterTime = controlPanel.getLoopInPosition();
    }
    else if (mouseHandler.getDraggedHandle() == MouseHandler::LoopMarkerHandle::Out)
    {
        zoomCenterTime = controlPanel.getLoopOutPosition();
    }
    else
    {
        zoomCenterTime = audioPlayer.getTransportSource().getCurrentPosition();
    }

    // Determine current time point for the indicator (the setting being adjusted)
    const double indicatorTime = (activePoint == ControlPanel::ActiveZoomPoint::In) 
                                ? controlPanel.getLoopInPosition() 
                                : controlPanel.getLoopOutPosition();

    // Calculate time range for dynamic zoom
    double timeRange = audioLength / (double)controlPanel.getZoomFactor();
    timeRange = juce::jmin(timeRange, audioLength);

    const double startTime = zoomCenterTime - (timeRange / 2.0);
    const double endTime = startTime + timeRange;

    // Cache for MouseHandler
    controlPanel.setZoomPopupBounds(popupBounds);
    controlPanel.setZoomTimeRange(startTime, endTime);

    // Draw background
    g.setColour(juce::Colours::black);
    g.fillRect(popupBounds);

    // Draw zoomed waveform
    g.setColour(Config::waveformColor);
    audioPlayer.getThumbnail().drawChannels(g, popupBounds, startTime, endTime, 1.0f);

    // --- Boundary Shadows ---
    auto drawShadow = [&](double startT, double endT, juce::Colour color) {
        if (endT <= startTime || startT >= endTime) return;
        double vStart = juce::jmax(startT, startTime);
        double vEnd = juce::jmin(endT, endTime);
        float x1 = (float)popupBounds.getX() + (float)popupBounds.getWidth() * (float)((vStart - startTime) / timeRange);
        float x2 = (float)popupBounds.getX() + (float)popupBounds.getWidth() * (float)((vEnd - startTime) / timeRange);
        g.setColour(color);
        g.fillRect(x1, (float)popupBounds.getY(), x2 - x1, (float)popupBounds.getHeight());
    };

    const double loopIn = controlPanel.getLoopInPosition();
    const double loopOut = controlPanel.getLoopOutPosition();

    // Shadow regions outside the loop
    drawShadow(startTime, loopIn, juce::Colours::black.withAlpha(0.5f));
    drawShadow(loopOut, endTime, juce::Colours::black.withAlpha(0.5f));

    // Shadow regions outside the file (solid black)
    if (startTime < 0.0)
        drawShadow(startTime, 0.0, juce::Colours::black);
    if (endTime > audioLength)
        drawShadow(audioLength, endTime, juce::Colours::black);

    // --- Tracking Lines ---
    auto drawFineLine = [&](double time, juce::Colour color, float thickness) {
        if (time >= startTime && time <= endTime) {
            float proportion = (float)((time - startTime) / timeRange);
            float x = (float)popupBounds.getX() + proportion * (float)popupBounds.getWidth();
            g.setColour(color);
            g.drawLine(x, (float)popupBounds.getY(), x, (float)popupBounds.getBottom(), thickness);
        }
    };

    bool isDraggingIn = mouseHandler.getDraggedHandle() == MouseHandler::LoopMarkerHandle::In;
    bool isDraggingOut = mouseHandler.getDraggedHandle() == MouseHandler::LoopMarkerHandle::Out;

    // Draw Loop Lines (Fine)
    drawFineLine(loopIn, Config::loopLineColor, 1.0f);
    drawFineLine(loopOut, Config::loopLineColor, 1.0f);
    
    // Draw Playback Cursor (Fine)
    drawFineLine(audioPlayer.getTransportSource().getCurrentPosition(), Config::playbackCursorColor, 1.0f);

    // Draw the "Tracking" line (the one we are centered on) with more prominence
    if (isDraggingIn || isDraggingOut)
        drawFineLine(isDraggingIn ? loopIn : loopOut, juce::Colours::dodgerblue, 2.0f); // Blue tracking
    else
        drawFineLine(audioPlayer.getTransportSource().getCurrentPosition(), juce::Colours::lime, 2.0f); // Green tracking

    // Draw blue border
    g.setColour(Config::zoomPopupBorderColor);
    g.drawRect(popupBounds.toFloat(), Config::zoomPopupBorderThickness);
}
