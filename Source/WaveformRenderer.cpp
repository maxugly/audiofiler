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
}

void WaveformRenderer::drawWaveform(juce::Graphics& g, AudioPlayer& audioPlayer) const
{
    const auto numChannels = audioPlayer.getThumbnail().getNumChannels();
    if (numChannels <= 0)
        return;

    int pixelsPerSample = 1;
    if (controlPanel.getCurrentQualitySetting() == AppEnums::ThumbnailQuality::Low)
        pixelsPerSample = 4;
    else if (controlPanel.getCurrentQualitySetting() == AppEnums::ThumbnailQuality::Medium)
        pixelsPerSample = 2;

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
        const auto topY = (float)centerY - (maxVal * height * 0.5f);
        const auto bottomY = (float)centerY - (minVal * height * 0.5f);
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
            g.fillRect(lineStartX, topThresholdY - (Config::thresholdGlowThickness / 2.0f - 0.5f), currentLineWidth, Config::thresholdGlowThickness);
            g.fillRect(lineStartX, bottomThresholdY - (Config::thresholdGlowThickness / 2.0f - 0.5f), currentLineWidth, Config::thresholdGlowThickness);
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

    const juce::Rectangle<float> leftRegion((float)waveformBounds.getX(), (float)waveformBounds.getY(), inX - (float)waveformBounds.getX(), (float)waveformBounds.getHeight());
    if (leftRegion.getWidth() > 0.0f)
    {
        // 1. Black out the area beyond the fade
        juce::Rectangle<float> solidBlackLeft = leftRegion.withWidth(juce::jmax(0.0f, leftRegion.getWidth() - fadeLength));
        g.setColour(juce::Colours::black);
        g.fillRect(solidBlackLeft);

        // 2. Fade to transparent black at the loop boundary
        juce::Rectangle<float> fadeAreaLeft(inX - fadeLength, (float)waveformBounds.getY(), fadeLength, (float)waveformBounds.getHeight());
        juce::ColourGradient leftFadeGradient(juce::Colours::transparentBlack, inX, leftRegion.getCentreY(),
                                              juce::Colours::black, inX - fadeLength, leftRegion.getCentreY(), false);
        g.setGradientFill(leftFadeGradient);
        g.fillRect(fadeAreaLeft);
        
        // 3. Draw the legacy loop region color overlay
        g.setColour(Config::loopRegionColor);
        g.fillRect(leftRegion);
    }

    const juce::Rectangle<float> rightRegion(outX, (float)waveformBounds.getY(), (float)waveformBounds.getRight() - outX, (float)waveformBounds.getHeight());
    if (rightRegion.getWidth() > 0.0f)
    {
        // 1. Black out the area beyond the fade
        float solidBlackStart = outX + fadeLength;
        juce::Rectangle<float> solidBlackRight(solidBlackStart, (float)waveformBounds.getY(), (float)waveformBounds.getRight() - solidBlackStart, (float)waveformBounds.getHeight());
        g.setColour(juce::Colours::black);
        g.fillRect(solidBlackRight);

        // 2. Fade from transparent black at the loop boundary
        juce::Rectangle<float> fadeAreaRight(outX, (float)waveformBounds.getY(), fadeLength, (float)waveformBounds.getHeight());
        juce::ColourGradient rightFadeGradient(juce::Colours::transparentBlack, outX, rightRegion.getCentreY(),
                                               juce::Colours::black, outX + fadeLength, rightRegion.getCentreY(), false);
        g.setGradientFill(rightFadeGradient);
        g.fillRect(fadeAreaRight);

        // 3. Draw the legacy loop region color overlay
        g.setColour(Config::loopRegionColor);
        g.fillRect(rightRegion);
    }

    const juce::Colour glowColor = Config::loopLineColor.withAlpha(Config::loopLineColor.getFloatAlpha() * (1.0f - controlPanel.getGlowAlpha()));
    g.setColour(glowColor);
    g.fillRect(inX - (Config::loopLineGlowThickness / 2.0f - 0.5f), (float)waveformBounds.getY(), Config::loopLineGlowThickness, (float)waveformBounds.getHeight());
    g.fillRect(outX - (Config::loopLineGlowThickness / 2.0f - 0.5f), (float)waveformBounds.getY(), Config::loopLineGlowThickness, (float)waveformBounds.getHeight());

    g.setColour(Config::loopLineColor);
    auto drawLoopMarker = [&](float x, MouseHandler::LoopMarkerHandle handleType) {
        const auto& mouseHandler = controlPanel.getMouseHandler();
        
        juce::Colour capColor = Config::loopLineColor;
        if (mouseHandler.getDraggedHandle() == handleType)
            capColor = Config::loopMarkerDragColor;
        else if (mouseHandler.getHoveredHandle() == handleType)
            capColor = Config::loopMarkerHoverColor;

        // Draw the thin middle part
        g.setColour(Config::loopLineColor);
        g.fillRect(x - Config::loopMarkerWidthThin / Config::loopMarkerCenterDivisor, (float)waveformBounds.getY(), Config::loopMarkerWidthThin, (float)waveformBounds.getHeight());
        
        g.setColour(capColor);
        // Draw the thick top cap
        g.fillRect(x - Config::loopMarkerWidthThick / Config::loopMarkerCenterDivisor, (float)waveformBounds.getY(), 
                   Config::loopMarkerWidthThick, (float)Config::loopMarkerCapHeight);
        
        // Draw the thick bottom cap
        g.fillRect(x - Config::loopMarkerWidthThick / Config::loopMarkerCenterDivisor, (float)waveformBounds.getBottom() - Config::loopMarkerCapHeight, 
                   Config::loopMarkerWidthThick, (float)Config::loopMarkerCapHeight);
    };

    drawLoopMarker(inX, MouseHandler::LoopMarkerHandle::In);
    drawLoopMarker(outX, MouseHandler::LoopMarkerHandle::Out);

    const auto& mouseHandler = controlPanel.getMouseHandler();
    juce::Colour hollowColor = Config::loopLineColor;
    if (mouseHandler.getDraggedHandle() == MouseHandler::LoopMarkerHandle::Full)
        hollowColor = Config::loopMarkerDragColor;
    else if (mouseHandler.getHoveredHandle() == MouseHandler::LoopMarkerHandle::Full)
        hollowColor = Config::loopMarkerHoverColor;

    g.setColour(hollowColor);
    int hollowHeight = Config::loopMarkerCapHeight / 3;
    
    g.drawHorizontalLine(waveformBounds.getY(), (int)inX, (int)outX);
    g.drawHorizontalLine(waveformBounds.getY() + hollowHeight, (int)inX, (int)outX);
    
    g.drawHorizontalLine(waveformBounds.getBottom() - 1, (int)inX, (int)outX);
    g.drawHorizontalLine(waveformBounds.getBottom() - hollowHeight, (int)inX, (int)outX);
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

        g.setColour(currentGlowColor.withAlpha(0.3f));
        g.fillRect(mouseHandler.getMouseCursorX() - (int)(currentGlowThickness / 2) - 1, waveformBounds.getY(),
                   (int)currentGlowThickness + 2, waveformBounds.getHeight());
        g.fillRect(waveformBounds.getX(), mouseHandler.getMouseCursorY() - (int)(currentGlowThickness / 2) - 1,
                   waveformBounds.getWidth(), (int)currentGlowThickness + 2);
    }
    else
    {
        currentLineColor = Config::mouseCursorLineColor;
        currentHighlightColor = Config::mouseCursorHighlightColor;
        currentGlowColor = Config::mouseAmplitudeGlowColor;
    }

    g.setColour(currentHighlightColor);
    g.fillRect(mouseHandler.getMouseCursorX() - 2, waveformBounds.getY(), 5, waveformBounds.getHeight());
    g.fillRect(waveformBounds.getX(), mouseHandler.getMouseCursorY() - 2, waveformBounds.getWidth(), 5);

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
        const float amplitudeY = centerY - (amplitude * waveformBounds.getHeight() * 0.5f);
        const float bottomAmplitudeY = centerY + (amplitude * waveformBounds.getHeight() * 0.5f);

        juce::ColourGradient amplitudeGlowGradient(currentGlowColor.withAlpha(0.0f), (float)mouseHandler.getMouseCursorX(), amplitudeY,
                                                   currentGlowColor.withAlpha(0.7f), (float)mouseHandler.getMouseCursorX(), centerY, true);
        g.setGradientFill(amplitudeGlowGradient);
        g.fillRect(juce::Rectangle<float>((float)mouseHandler.getMouseCursorX() - Config::mouseAmplitudeGlowThickness / 2, amplitudeY,
                                          Config::mouseAmplitudeGlowThickness, centerY - amplitudeY));
        g.fillRect(juce::Rectangle<float>((float)mouseHandler.getMouseCursorX() - Config::mouseAmplitudeGlowThickness / 2, centerY,
                                          Config::mouseAmplitudeGlowThickness, centerY - amplitudeY));

        g.setColour(Config::mouseAmplitudeLineColor);
        g.drawVerticalLine(mouseHandler.getMouseCursorX(), amplitudeY, bottomAmplitudeY);

        const float halfLineLength = Config::mouseAmplitudeLineLength / 2.0f;
        const float leftExtent = (float)mouseHandler.getMouseCursorX() - halfLineLength;
        const float rightExtent = (float)mouseHandler.getMouseCursorX() + halfLineLength;
        g.drawHorizontalLine(juce::roundToInt(amplitudeY), leftExtent, rightExtent);
        g.drawHorizontalLine(juce::roundToInt(bottomAmplitudeY), leftExtent, rightExtent);

        g.setColour(Config::playbackTextColor);
        g.setFont(Config::mouseCursorTextSize);
        g.drawText(juce::String(amplitude, 2), mouseHandler.getMouseCursorX() + 5, (int)amplitudeY - Config::mouseCursorTextSize,
                   100, Config::mouseCursorTextSize, juce::Justification::left, true);
        g.drawText(juce::String(-amplitude, 2), mouseHandler.getMouseCursorX() + 5, (int)bottomAmplitudeY,
                   100, Config::mouseCursorTextSize, juce::Justification::left, true);

        const juce::String timeText = controlPanel.formatTime(mouseHandler.getMouseCursorTime());
        g.drawText(timeText, mouseHandler.getMouseCursorX() + 5, mouseHandler.getMouseCursorY() + 5, 100,
                   Config::mouseCursorTextSize, juce::Justification::left, true);
    }

    g.setColour(currentLineColor);
    g.drawVerticalLine(mouseHandler.getMouseCursorX(), (float)waveformBounds.getY(), (float)waveformBounds.getBottom());
    g.drawHorizontalLine(mouseHandler.getMouseCursorY(), (float)waveformBounds.getX(), (float)waveformBounds.getRight());
}
