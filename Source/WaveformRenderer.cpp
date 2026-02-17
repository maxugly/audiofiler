#include "WaveformRenderer.h"

#include "ControlPanel.h"
#include "AudioPlayer.h"
#include "FocusManager.h"
#include "MouseHandler.h"
#include "SilenceDetector.h"
#include "Config.h"
#include "PlaybackCursorGlow.h"

void WaveformRenderer::invalidateWaveformCache()
{
    waveformCache = juce::Image();
    lastAudioLength = -1.0;
}

WaveformRenderer::WaveformRenderer(ControlPanel& controlPanelIn)
    : controlPanel(controlPanelIn)
{
}

void WaveformRenderer::renderWaveform(juce::Graphics& g)
{
    AudioPlayer& audioPlayer = controlPanel.getAudioPlayer();
    drawWaveform(g, audioPlayer);
}

void WaveformRenderer::renderOverlays(juce::Graphics& g)
{
    AudioPlayer& audioPlayer = controlPanel.getAudioPlayer();
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

    const auto bounds = controlPanel.getWaveformBounds();
    const double audioLength = audioPlayer.getThumbnail().getTotalLength();
    const int quality = (int)controlPanel.getCurrentQualitySetting();
    const int channelMode = (int)controlPanel.getChannelViewMode();
    const float scale = g.getInternalContext().getPhysicalPixelScaleFactor();

    if (!waveformCache.isValid()
        || lastBounds != bounds
        || std::abs(lastAudioLength - audioLength) > 0.001
        || lastQuality != quality
        || lastChannelMode != channelMode
        || std::abs(lastScale - scale) > 0.001f)
    {
        // Re-render cache
        int w = juce::roundToInt((float)bounds.getWidth() * scale);
        int h = juce::roundToInt((float)bounds.getHeight() * scale);

        if (w <= 0 || h <= 0) return;

        waveformCache = juce::Image(juce::Image::ARGB, w, h, true);
        juce::Graphics ig(waveformCache);

        ig.addTransform(juce::AffineTransform::scale(scale));
        ig.setOrigin(-bounds.getX(), -bounds.getY());

        int pixelsPerSample = 1;
        if (controlPanel.getCurrentQualitySetting() == AppEnums::ThumbnailQuality::Low)
            pixelsPerSample = Config::Layout::Waveform::pixelsPerSampleLow;
        else if (controlPanel.getCurrentQualitySetting() == AppEnums::ThumbnailQuality::Medium)
            pixelsPerSample = Config::Layout::Waveform::pixelsPerSampleMedium;

        ig.setColour(Config::Colors::waveform);
        if (controlPanel.getChannelViewMode() == AppEnums::ChannelViewMode::Mono || numChannels == 1)
        {
            if (pixelsPerSample > 1)
                drawReducedQualityWaveform(ig, audioPlayer, 0, pixelsPerSample);
            else
                audioPlayer.getThumbnail().drawChannel(ig, bounds, 0.0, audioPlayer.getThumbnail().getTotalLength(), 0, 1.0f);
        }
        else
        {
            if (pixelsPerSample > 1)
            {
                for (int ch = 0; ch < numChannels; ++ch)
                    drawReducedQualityWaveform(ig, audioPlayer, ch, pixelsPerSample);
            }
            else
            {
                audioPlayer.getThumbnail().drawChannels(ig, bounds, 0.0, audioPlayer.getThumbnail().getTotalLength(), 1.0f);
            }
        }

        lastBounds = bounds;
        lastAudioLength = audioLength;
        lastQuality = quality;
        lastChannelMode = channelMode;
        lastScale = scale;
    }

    g.drawImage(waveformCache, bounds.toFloat());
}

void WaveformRenderer::drawReducedQualityWaveform(juce::Graphics& g, AudioPlayer& audioPlayer, int channel, int pixelsPerSample) const
{
    const auto audioLength = audioPlayer.getThumbnail().getTotalLength();
    if (audioLength <= 0.0)
        return;

    const auto waveformBounds = controlPanel.getWaveformBounds();
    const int width = waveformBounds.getWidth();
    if (width <= 0)
        return;

    const float height = (float)waveformBounds.getHeight();
    const float centerY = (float)waveformBounds.getCentreY();
    const float halfHeightScale = height * Config::Layout::Waveform::heightScale;
    if (pixelsPerSample <= 0)
        return;
    const double timePerPixel = audioLength / (double)width;
    const double timeDelta = timePerPixel * pixelsPerSample;
    const int offsetX = waveformBounds.getX();

    juce::RectangleList<float> waveformRects;

    for (int x = 0; x < width; x += pixelsPerSample)
    {
        const double time = (double)x * timePerPixel;
        float minVal = 0.0f, maxVal = 0.0f;
        audioPlayer.getThumbnail().getApproximateMinMax(time, time + timeDelta, channel, minVal, maxVal);

        const float topY = centerY - (maxVal * halfHeightScale);
        const float bottomY = centerY - (minVal * halfHeightScale);

        const float xPos = (float)(offsetX + x);
    waveformRects.ensureStorageAllocated(width / pixelsPerSample + 1);
        waveformRects.addWithoutMerging({ xPos, topY, 1.0f, bottomY - topY });
    }

    g.fillRectList(waveformRects);
}

void WaveformRenderer::drawCutModeOverlays(juce::Graphics& g, AudioPlayer& audioPlayer, float audioLength) const
{
    const auto waveformBounds = controlPanel.getWaveformBounds();
    const auto& silenceDetector = controlPanel.getSilenceDetector();

    auto drawThresholdVisualisation = [&](double cutPos, float threshold)
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

        const float xPos = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (cutPos / audioLength);
        const float halfThresholdLineWidth = Config::Animation::thresholdLineWidth / 2.0f;
        float lineStartX = xPos - halfThresholdLineWidth;
        float lineEndX = xPos + halfThresholdLineWidth;

        lineStartX = juce::jmax(lineStartX, (float)waveformBounds.getX());
        lineEndX = juce::jmin(lineEndX, (float)waveformBounds.getRight());
        const float currentLineWidth = lineEndX - lineStartX;

        g.setColour(Config::Colors::thresholdRegion);
        g.fillRect(lineStartX, topThresholdY, currentLineWidth, bottomThresholdY - topThresholdY);

        if (controlPanel.isCutModeActive())
        {
            const juce::Colour glowColor = Config::Colors::thresholdLine.withAlpha(Config::Colors::thresholdLine.getFloatAlpha() * controlPanel.getGlowAlpha());
            g.setColour(glowColor);
            g.fillRect(lineStartX, topThresholdY - (Config::Layout::Glow::thresholdGlowThickness * Config::Layout::Glow::offsetFactor - 0.5f), currentLineWidth, Config::Layout::Glow::thresholdGlowThickness);
            g.fillRect(lineStartX, bottomThresholdY - (Config::Layout::Glow::thresholdGlowThickness * Config::Layout::Glow::offsetFactor - 0.5f), currentLineWidth, Config::Layout::Glow::thresholdGlowThickness);
        }

        g.setColour(Config::Colors::thresholdLine);
        g.drawHorizontalLine((int)topThresholdY, lineStartX, lineEndX);
        g.drawHorizontalLine((int)bottomThresholdY, lineStartX, lineEndX);
    };

    const double cutIn = audioPlayer.getCutIn();
    const double cutOut = audioPlayer.getCutOut();
    drawThresholdVisualisation(cutIn, silenceDetector.getCurrentInSilenceThreshold());
    drawThresholdVisualisation(cutOut, silenceDetector.getCurrentOutSilenceThreshold());

    const double actualIn = juce::jmin(cutIn, cutOut);
    const double actualOut = juce::jmax(cutIn, cutOut);
    const float inX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (actualIn / audioLength);
    const float outX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (actualOut / audioLength);
    const float fadeLength = waveformBounds.getWidth() * Config::Layout::Waveform::loopRegionFadeProportion;
    const float boxHeight = (float)Config::Layout::Glow::loopMarkerBoxHeight;

    const juce::Rectangle<float> leftRegion((float)waveformBounds.getX(), (float)waveformBounds.getY(), inX - (float)waveformBounds.getX(), (float)waveformBounds.getHeight());
    if (leftRegion.getWidth() > 0.0f)
    {
        const float actualFade = juce::jmin(fadeLength, leftRegion.getWidth());
        
        // 1. Black out the area beyond the fade
        juce::Rectangle<float> solidBlackLeft = leftRegion.withWidth(leftRegion.getWidth() - actualFade);
        g.setColour(juce::Colours::black);
        g.fillRect(solidBlackLeft);

        // 2. Fade from blue cut color to black
        juce::Rectangle<float> fadeAreaLeft(inX - actualFade, (float)waveformBounds.getY(), actualFade, (float)waveformBounds.getHeight());
        juce::ColourGradient leftFadeGradient(Config::Colors::loopRegion, inX, leftRegion.getCentreY(),
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

        // 2. Fade from blue cut color to black
        juce::Rectangle<float> fadeAreaRight(outX, (float)waveformBounds.getY(), actualFade, (float)waveformBounds.getHeight());
        juce::ColourGradient rightFadeGradient(Config::Colors::loopRegion, outX, rightRegion.getCentreY(),
                                               juce::Colours::black, outX + actualFade, rightRegion.getCentreY(), false);
        g.setGradientFill(rightFadeGradient);
        g.fillRect(fadeAreaRight);
    }

    const juce::Colour glowColor = Config::Colors::loopLine.withAlpha(Config::Colors::loopLine.getFloatAlpha() * (1.0f - controlPanel.getGlowAlpha()));
    g.setColour(glowColor);
    g.fillRect(inX - (Config::Layout::Glow::loopLineGlowThickness * Config::Layout::Glow::offsetFactor - 0.5f), (float)waveformBounds.getY() + boxHeight, Config::Layout::Glow::loopLineGlowThickness, (float)waveformBounds.getHeight() - (2.0f * boxHeight));
    g.fillRect(outX - (Config::Layout::Glow::loopLineGlowThickness * Config::Layout::Glow::offsetFactor - 0.5f), (float)waveformBounds.getY() + boxHeight, Config::Layout::Glow::loopLineGlowThickness, (float)waveformBounds.getHeight() - (2.0f * boxHeight));

    g.setColour(Config::Colors::loopLine);
    auto drawCutMarker = [&](float x, MouseHandler::CutMarkerHandle handleType) {
        const auto& mouseHandler = controlPanel.getMouseHandler();
        const auto& silenceDetector = controlPanel.getSilenceDetector();
        
        juce::Colour markerColor = Config::Colors::loopLine;
        
        // Base color based on Auto-Cut status
        if (handleType == MouseHandler::CutMarkerHandle::In && silenceDetector.getIsAutoCutInActive())
            markerColor = Config::Colors::loopMarkerAuto;
        else if (handleType == MouseHandler::CutMarkerHandle::Out && silenceDetector.getIsAutoCutOutActive())
            markerColor = Config::Colors::loopMarkerAuto;

        float thickness = Config::Layout::Glow::loopBoxOutlineThickness;

        if (mouseHandler.getDraggedHandle() == handleType)
        {
            markerColor = Config::Colors::loopMarkerDrag;
            thickness = Config::Layout::Glow::loopBoxOutlineThicknessInteracting;
        }
        else if (mouseHandler.getHoveredHandle() == handleType)
        {
            markerColor = Config::Colors::loopMarkerHover;
            thickness = Config::Layout::Glow::loopBoxOutlineThicknessInteracting;
        }

        const float boxWidth = Config::Layout::Glow::loopMarkerBoxWidth;
        const float halfBoxWidth = boxWidth / 2.0f;

        // Draw the top hollow box
        g.setColour(markerColor);
        g.drawRect(x - halfBoxWidth, (float)waveformBounds.getY(), boxWidth, boxHeight, thickness);
        
        // Draw the bottom hollow box
        g.drawRect(x - halfBoxWidth, (float)waveformBounds.getBottom() - boxHeight, boxWidth, boxHeight, thickness);

        // Draw the thin middle line between the boxes
        g.setColour(markerColor); // Use the marker color for the middle line too if auto/interacting
        g.fillRect(x - Config::Layout::Glow::loopMarkerWidthThin / Config::Layout::Glow::loopMarkerCenterDivisor,
                   (float)waveformBounds.getY() + boxHeight, 
                   Config::Layout::Glow::loopMarkerWidthThin,
                   (float)waveformBounds.getHeight() - (2.0f * boxHeight));
    };

    drawCutMarker(inX, MouseHandler::CutMarkerHandle::In);
    drawCutMarker(outX, MouseHandler::CutMarkerHandle::Out);

    const auto& mouseHandler = controlPanel.getMouseHandler();
    juce::Colour hollowColor = Config::Colors::loopLine;
    float thickness = Config::Layout::Glow::loopBoxOutlineThickness;

    if (mouseHandler.getDraggedHandle() == MouseHandler::CutMarkerHandle::Full)
    {
        hollowColor = Config::Colors::loopMarkerDrag;
        thickness = Config::Layout::Glow::loopBoxOutlineThicknessInteracting;
    }
    else if (mouseHandler.getHoveredHandle() == MouseHandler::CutMarkerHandle::Full)
    {
        hollowColor = Config::Colors::loopMarkerHover;
        thickness = Config::Layout::Glow::loopBoxOutlineThicknessInteracting;
    }

    g.setColour(hollowColor);
    const float halfBoxWidth = Config::Layout::Glow::loopMarkerBoxWidth / 2.0f;
    
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
    const float drawPosition = (float)audioPlayer.getTransportSource().getCurrentPosition();
    const float x = (drawPosition / audioLength) * (float)waveformBounds.getWidth() + (float)waveformBounds.getX();

    PlaybackCursorGlow::renderGlow(g, (int)x, waveformBounds.getY(), waveformBounds.getBottom(), Config::Colors::playbackText);
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
        currentLineColor = Config::Colors::mousePlacementMode;
        currentHighlightColor = Config::Colors::mousePlacementMode.withAlpha(0.4f);
        currentGlowColor = Config::Colors::placementModeGlow;
        currentGlowThickness = Config::Layout::Glow::placementModeGlowThickness;

        g.setColour(currentGlowColor.withAlpha(Config::Layout::Glow::mouseAlpha));
        g.fillRect(mouseHandler.getMouseCursorX() - (int)(currentGlowThickness * Config::Layout::Glow::offsetFactor) - 1, waveformBounds.getY(),
                   (int)currentGlowThickness + Config::Layout::Glow::mousePadding, waveformBounds.getHeight());
        g.fillRect(waveformBounds.getX(), mouseHandler.getMouseCursorY() - (int)(currentGlowThickness * Config::Layout::Glow::offsetFactor) - 1,
                   waveformBounds.getWidth(), (int)currentGlowThickness + Config::Layout::Glow::mousePadding);
    }
    else
    {
        if (controlPanel.isZKeyDown())
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
    g.fillRect(mouseHandler.getMouseCursorX() - Config::Layout::Glow::mouseHighlightOffset, waveformBounds.getY(), Config::Layout::Glow::mouseHighlightSize, waveformBounds.getHeight());
    g.fillRect(waveformBounds.getX(), mouseHandler.getMouseCursorY() - Config::Layout::Glow::mouseHighlightOffset, waveformBounds.getWidth(), Config::Layout::Glow::mouseHighlightSize);

    if (audioLength > 0.0f)
    {
        float amplitude = 0.0f;
        if (audioPlayer.getThumbnail().getNumChannels() > 0)
        {
            double sampleRate = 0.0;
            juce::int64 length = 0;
            if (audioPlayer.getReaderInfo(sampleRate, length) && sampleRate > 0.0)
            {
                float minVal = 0.0f, maxVal = 0.0f;
                audioPlayer.getThumbnail().getApproximateMinMax(mouseHandler.getMouseCursorTime(),
                                                                mouseHandler.getMouseCursorTime() + (1.0 / sampleRate),
                                                                0, minVal, maxVal);
                amplitude = juce::jmax(std::abs(minVal), std::abs(maxVal));
            }
        }

        const float centerY = (float)waveformBounds.getCentreY();
        const float amplitudeY = centerY - (amplitude * waveformBounds.getHeight() * Config::Layout::Waveform::heightScale);
        const float bottomAmplitudeY = centerY + (amplitude * waveformBounds.getHeight() * Config::Layout::Waveform::heightScale);

        juce::ColourGradient amplitudeGlowGradient(currentGlowColor.withAlpha(0.0f), (float)mouseHandler.getMouseCursorX(), amplitudeY,
                                                   currentGlowColor.withAlpha(Config::Layout::Glow::mouseAmplitudeAlpha), (float)mouseHandler.getMouseCursorX(), centerY, true);
        g.setGradientFill(amplitudeGlowGradient);
        g.fillRect(juce::Rectangle<float>((float)mouseHandler.getMouseCursorX() - Config::Layout::Glow::mouseAmplitudeGlowThickness * Config::Layout::Glow::offsetFactor, amplitudeY,
                                          Config::Layout::Glow::mouseAmplitudeGlowThickness, centerY - amplitudeY));
        g.fillRect(juce::Rectangle<float>((float)mouseHandler.getMouseCursorX() - Config::Layout::Glow::mouseAmplitudeGlowThickness * Config::Layout::Glow::offsetFactor, centerY,
                                          Config::Layout::Glow::mouseAmplitudeGlowThickness, centerY - amplitudeY));

        g.setColour(Config::Colors::mouseAmplitudeLine);
        g.drawVerticalLine(mouseHandler.getMouseCursorX(), amplitudeY, bottomAmplitudeY);

        const float halfLineLength = Config::Animation::mouseAmplitudeLineLength * Config::Layout::Glow::offsetFactor;
        const float leftExtent = (float)mouseHandler.getMouseCursorX() - halfLineLength;
        const float rightExtent = (float)mouseHandler.getMouseCursorX() + halfLineLength;
        g.drawHorizontalLine(juce::roundToInt(amplitudeY), leftExtent, rightExtent);
        g.drawHorizontalLine(juce::roundToInt(bottomAmplitudeY), leftExtent, rightExtent);

        g.setColour(Config::Colors::playbackText);
        g.setFont(Config::Layout::Text::mouseCursorSize);
        g.drawText(juce::String(amplitude, 2), mouseHandler.getMouseCursorX() + Config::Layout::Glow::mouseTextOffset, (int)amplitudeY - Config::Layout::Text::mouseCursorSize,
                   100, Config::Layout::Text::mouseCursorSize, juce::Justification::left, true);
        g.drawText(juce::String(-amplitude, 2), mouseHandler.getMouseCursorX() + Config::Layout::Glow::mouseTextOffset, (int)bottomAmplitudeY,
                   100, Config::Layout::Text::mouseCursorSize, juce::Justification::left, true);

        const juce::String timeText = controlPanel.formatTime(mouseHandler.getMouseCursorTime());
        g.drawText(timeText, mouseHandler.getMouseCursorX() + Config::Layout::Glow::mouseTextOffset, mouseHandler.getMouseCursorY() + Config::Layout::Glow::mouseTextOffset, 100,
                   Config::Layout::Text::mouseCursorSize, juce::Justification::left, true);
    }

    PlaybackCursorGlow::renderGlow(g, mouseHandler.getMouseCursorX(), waveformBounds.getY(), waveformBounds.getBottom(), currentLineColor);
    g.setColour(currentLineColor);
    g.drawHorizontalLine(mouseHandler.getMouseCursorY(), (float)waveformBounds.getX(), (float)waveformBounds.getRight());
}

void WaveformRenderer::drawZoomPopup(juce::Graphics& g) const
{
    const bool zDown = controlPanel.isZKeyDown();
    const auto activePoint = controlPanel.getActiveZoomPoint();
    
    if (!zDown && activePoint == ControlPanel::ActiveZoomPoint::None)
        return;

    AudioPlayer& audioPlayer = controlPanel.getAudioPlayer();
    const double audioLength = audioPlayer.getThumbnail().getTotalLength();
    if (audioLength <= 0.0)
        return;

    const auto waveformBounds = controlPanel.getWaveformBounds();
    
    // Calculate popup bounds: 80% of waveform area, centered
    const int popupWidth = juce::roundToInt((float)waveformBounds.getWidth() * Config::Layout::Zoom::popupScale);
    const int popupHeight = juce::roundToInt((float)waveformBounds.getHeight() * Config::Layout::Zoom::popupScale);
    const juce::Rectangle<int> popupBounds(
        waveformBounds.getCentreX() - popupWidth / 2,
        waveformBounds.getCentreY() - popupHeight / 2,
        popupWidth,
        popupHeight
    );

    const auto& mouseHandler = controlPanel.getMouseHandler();
    
    // Determine the center time for the zoom window
    double zoomCenterTime = 0.0;
    // Use FocusManager to determine what time to center the zoom popup on
    zoomCenterTime = controlPanel.getFocusManager().getFocusedTime();
// Determine current time point for the indicator (the setting being adjusted)
    double indicatorTime = 0.0;
    if (activePoint == ControlPanel::ActiveZoomPoint::In)
        indicatorTime = audioPlayer.getCutIn();
    else if (activePoint == ControlPanel::ActiveZoomPoint::Out)
        indicatorTime = audioPlayer.getCutOut();
    else
        indicatorTime = audioPlayer.getTransportSource().getCurrentPosition();

    // Calculate time range for dynamic zoom
    double timeRange = audioLength / (double)controlPanel.getZoomFactor();
    
    // Ensure we don't zoom in so much that we have 0 duration (min 2 samples at 44.1k approx)
    timeRange = juce::jlimit(0.00005, audioLength, timeRange);

    const double startTime = zoomCenterTime - (timeRange / 2.0);
    const double endTime = startTime + timeRange;

    // Cache for MouseHandler
    controlPanel.setZoomPopupBounds(popupBounds);
    controlPanel.setZoomTimeRange(startTime, endTime);

    // Draw background
    g.setColour(juce::Colours::black);
    g.fillRect(popupBounds);

    // --- Draw zoomed waveform based on channel mode ---
    g.setColour(Config::Colors::waveform);
    const auto channelMode = controlPanel.getChannelViewMode();
    const int numChannels = audioPlayer.getThumbnail().getNumChannels();

    if (channelMode == AppEnums::ChannelViewMode::Mono || numChannels == 1)
    {
        // Mono: Use full height
        audioPlayer.getThumbnail().drawChannel(g, popupBounds, startTime, endTime, 0, 1.0f);
        
        // Zero Line
        g.setColour(Config::Colors::zoomPopupZeroLine);
        g.drawHorizontalLine(popupBounds.getCentreY(), (float)popupBounds.getX(), (float)popupBounds.getRight());
    }
    else
    {
        // Stereo: Split height
        auto topBounds = popupBounds.withHeight(popupBounds.getHeight() / 2);
        auto bottomBounds = popupBounds.withTop(topBounds.getBottom()).withHeight(popupBounds.getHeight() / 2);
        
        audioPlayer.getThumbnail().drawChannel(g, topBounds, startTime, endTime, 0, 1.0f);
        audioPlayer.getThumbnail().drawChannel(g, bottomBounds, startTime, endTime, 1, 1.0f);

        // Zero Lines
        g.setColour(Config::Colors::zoomPopupZeroLine);
        g.drawHorizontalLine(topBounds.getCentreY(), (float)topBounds.getX(), (float)topBounds.getRight());
        g.drawHorizontalLine(bottomBounds.getCentreY(), (float)bottomBounds.getX(), (float)bottomBounds.getRight());
    }

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

    const double cutIn = audioPlayer.getCutIn();
    const double cutOut = audioPlayer.getCutOut();

    // Shadow regions outside the cut
    drawShadow(startTime, cutIn, juce::Colours::black.withAlpha(0.5f));
    drawShadow(cutOut, endTime, juce::Colours::black.withAlpha(0.5f));

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

    bool isDraggingCutIn = mouseHandler.getDraggedHandle() == MouseHandler::CutMarkerHandle::In;
    bool isDraggingCutOut = mouseHandler.getDraggedHandle() == MouseHandler::CutMarkerHandle::Out;

    // Draw Cut Lines (Fine)
    drawFineLine(cutIn, Config::Colors::loopLine, 1.0f);
    drawFineLine(cutOut, Config::Colors::loopLine, 1.0f);
    
    // Draw Playback Cursor (Fine)
    drawFineLine(audioPlayer.getTransportSource().getCurrentPosition(), Config::Colors::playbackCursor, 1.0f);

    // Draw the "Tracking" line (the one we are centered on) with more prominence
    if (isDraggingCutIn || isDraggingCutOut)
        drawFineLine(isDraggingCutIn ? cutIn : cutOut, Config::Colors::zoomPopupTrackingLine, 2.0f); // Blue tracking
    else
        drawFineLine(audioPlayer.getTransportSource().getCurrentPosition(), Config::Colors::zoomPopupPlaybackLine, 2.0f); // Green tracking

    // Draw blue border
    g.setColour(Config::Colors::zoomPopupBorder);
    g.drawRect(popupBounds.toFloat(), Config::Layout::Zoom::borderThickness);
}
