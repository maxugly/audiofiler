#include "CutLayerView.h"

#include "SessionState.h"
#include "SilenceDetector.h"
#include "MouseHandler.h"
#include "WaveformManager.h"

CutLayerView::CutLayerView(SessionState& sessionStateIn,
                           SilenceDetector& silenceDetectorIn,
                           WaveformManager& waveformManagerIn,
                           std::function<float()> glowAlphaProviderIn)
    : sessionState(sessionStateIn),
      silenceDetector(silenceDetectorIn),
      waveformManager(waveformManagerIn),
      glowAlphaProvider(std::move(glowAlphaProviderIn))
{
    setInterceptsMouseClicks(false, false);
    setOpaque(false);
}

void CutLayerView::paint(juce::Graphics& g)
{
    if (!markersVisible)
        return;

    const auto bounds = getLocalBounds();
    const float audioLength = (float)waveformManager.getThumbnail().getTotalLength();
    if (audioLength <= 0.0f)
        return;

    auto drawThresholdVisualisation = [&](double cutPos, float threshold)
    {
        const float normalisedThreshold = threshold;
        const float centerY = (float)bounds.getCentreY();
        const float halfHeight = (float)bounds.getHeight() / 2.0f;

        float topThresholdY = centerY - (normalisedThreshold * halfHeight);
        float bottomThresholdY = centerY + (normalisedThreshold * halfHeight);

        topThresholdY = juce::jlimit((float)bounds.getY(), (float)bounds.getBottom(), topThresholdY);
        bottomThresholdY = juce::jlimit((float)bounds.getY(), (float)bounds.getBottom(), bottomThresholdY);

        const float xPos = (float)bounds.getX() + (float)bounds.getWidth() * (float)(cutPos / audioLength);
        const float halfThresholdLineWidth = Config::Animation::thresholdLineWidth / 2.0f;
        float lineStartX = xPos - halfThresholdLineWidth;
        float lineEndX = xPos + halfThresholdLineWidth;

        lineStartX = juce::jmax(lineStartX, (float)bounds.getX());
        lineEndX = juce::jmin(lineEndX, (float)bounds.getRight());
        const float currentLineWidth = lineEndX - lineStartX;

        g.setColour(Config::Colors::thresholdRegion);
        g.fillRect(lineStartX, topThresholdY, currentLineWidth, bottomThresholdY - topThresholdY);

        const juce::Colour glowColor = Config::Colors::thresholdLine.withAlpha(Config::Colors::thresholdLine.getFloatAlpha() * glowAlphaProvider());
        g.setColour(glowColor);
        g.fillRect(lineStartX, topThresholdY - (Config::Layout::Glow::thresholdGlowThickness * Config::Layout::Glow::offsetFactor - 0.5f), currentLineWidth, Config::Layout::Glow::thresholdGlowThickness);
        g.fillRect(lineStartX, bottomThresholdY - (Config::Layout::Glow::thresholdGlowThickness * Config::Layout::Glow::offsetFactor - 0.5f), currentLineWidth, Config::Layout::Glow::thresholdGlowThickness);

        g.setColour(Config::Colors::thresholdLine);
        g.drawHorizontalLine((int)topThresholdY, lineStartX, lineEndX);
        g.drawHorizontalLine((int)bottomThresholdY, lineStartX, lineEndX);
    };

    const double cutIn = sessionState.getCutIn();
    const double cutOut = sessionState.getCutOut();
    drawThresholdVisualisation(cutIn, silenceDetector.getCurrentInSilenceThreshold());
    drawThresholdVisualisation(cutOut, silenceDetector.getCurrentOutSilenceThreshold());

    const double actualIn = juce::jmin(cutIn, cutOut);
    const double actualOut = juce::jmax(cutIn, cutOut);
    const float inX = (float)bounds.getX() + (float)bounds.getWidth() * (float)(actualIn / audioLength);
    const float outX = (float)bounds.getX() + (float)bounds.getWidth() * (float)(actualOut / audioLength);
    const float fadeLength = bounds.getWidth() * Config::Layout::Waveform::loopRegionFadeProportion;
    const float boxHeight = (float)Config::Layout::Glow::loopMarkerBoxHeight;

    const juce::Rectangle<float> leftRegion((float)bounds.getX(), (float)bounds.getY(), inX - (float)bounds.getX(), (float)bounds.getHeight());
    if (leftRegion.getWidth() > 0.0f)
    {
        const float actualFade = juce::jmin(fadeLength, leftRegion.getWidth());

        juce::Rectangle<float> solidBlackLeft = leftRegion.withWidth(leftRegion.getWidth() - actualFade);
        g.setColour(juce::Colours::black);
        g.fillRect(solidBlackLeft);

        juce::Rectangle<float> fadeAreaLeft(inX - actualFade, (float)bounds.getY(), actualFade, (float)bounds.getHeight());
        juce::ColourGradient leftFadeGradient(Config::Colors::loopRegion, inX, leftRegion.getCentreY(),
                                              juce::Colours::black, inX - actualFade, leftRegion.getCentreY(), false);
        g.setGradientFill(leftFadeGradient);
        g.fillRect(fadeAreaLeft);
    }

    const juce::Rectangle<float> rightRegion(outX, (float)bounds.getY(), (float)bounds.getRight() - outX, (float)bounds.getHeight());
    if (rightRegion.getWidth() > 0.0f)
    {
        const float actualFade = juce::jmin(fadeLength, rightRegion.getWidth());

        float solidBlackStart = outX + actualFade;
        juce::Rectangle<float> solidBlackRight(solidBlackStart, (float)bounds.getY(), (float)bounds.getRight() - solidBlackStart, (float)bounds.getHeight());
        g.setColour(juce::Colours::black);
        g.fillRect(solidBlackRight);

        juce::Rectangle<float> fadeAreaRight(outX, (float)bounds.getY(), actualFade, (float)bounds.getHeight());
        juce::ColourGradient rightFadeGradient(Config::Colors::loopRegion, outX, rightRegion.getCentreY(),
                                               juce::Colours::black, outX + actualFade, rightRegion.getCentreY(), false);
        g.setGradientFill(rightFadeGradient);
        g.fillRect(fadeAreaRight);
    }

    const juce::Colour glowColor = Config::Colors::loopLine.withAlpha(Config::Colors::loopLine.getFloatAlpha() * (1.0f - glowAlphaProvider()));
    g.setColour(glowColor);
    g.fillRect(inX - (Config::Layout::Glow::loopLineGlowThickness * Config::Layout::Glow::offsetFactor - 0.5f), (float)bounds.getY() + boxHeight, Config::Layout::Glow::loopLineGlowThickness, (float)bounds.getHeight() - (2.0f * boxHeight));
    g.fillRect(outX - (Config::Layout::Glow::loopLineGlowThickness * Config::Layout::Glow::offsetFactor - 0.5f), (float)bounds.getY() + boxHeight, Config::Layout::Glow::loopLineGlowThickness, (float)bounds.getHeight() - (2.0f * boxHeight));

    g.setColour(Config::Colors::loopLine);
    auto drawCutMarker = [&](float x, MouseHandler::CutMarkerHandle handleType) {
        juce::Colour markerColor = Config::Colors::loopLine;

        if (handleType == MouseHandler::CutMarkerHandle::In && silenceDetector.getIsAutoCutInActive())
            markerColor = Config::Colors::loopMarkerAuto;
        else if (handleType == MouseHandler::CutMarkerHandle::Out && silenceDetector.getIsAutoCutOutActive())
            markerColor = Config::Colors::loopMarkerAuto;

        float thickness = Config::Layout::Glow::loopBoxOutlineThickness;

        if (mouseHandler != nullptr && mouseHandler->getDraggedHandle() == handleType)
        {
            markerColor = Config::Colors::loopMarkerDrag;
            thickness = Config::Layout::Glow::loopBoxOutlineThicknessInteracting;
        }
        else if (mouseHandler != nullptr && mouseHandler->getHoveredHandle() == handleType)
        {
            markerColor = Config::Colors::loopMarkerHover;
            thickness = Config::Layout::Glow::loopBoxOutlineThicknessInteracting;
        }

        const float boxWidth = Config::Layout::Glow::loopMarkerBoxWidth;
        const float halfBoxWidth = boxWidth / 2.0f;

        g.setColour(markerColor);
        g.drawRect(x - halfBoxWidth, (float)bounds.getY(), boxWidth, boxHeight, thickness);
        g.drawRect(x - halfBoxWidth, (float)bounds.getBottom() - boxHeight, boxWidth, boxHeight, thickness);

        g.setColour(markerColor);
        g.fillRect(x - Config::Layout::Glow::loopMarkerWidthThin / Config::Layout::Glow::loopMarkerCenterDivisor,
                   (float)bounds.getY() + boxHeight,
                   Config::Layout::Glow::loopMarkerWidthThin,
                   (float)bounds.getHeight() - (2.0f * boxHeight));
    };

    drawCutMarker(inX, MouseHandler::CutMarkerHandle::In);
    drawCutMarker(outX, MouseHandler::CutMarkerHandle::Out);

    juce::Colour hollowColor = Config::Colors::loopLine;
    float thickness = Config::Layout::Glow::loopBoxOutlineThickness;

    if (mouseHandler != nullptr && mouseHandler->getDraggedHandle() == MouseHandler::CutMarkerHandle::Full)
    {
        hollowColor = Config::Colors::loopMarkerDrag;
        thickness = Config::Layout::Glow::loopBoxOutlineThicknessInteracting;
    }
    else if (mouseHandler != nullptr && mouseHandler->getHoveredHandle() == MouseHandler::CutMarkerHandle::Full)
    {
        hollowColor = Config::Colors::loopMarkerHover;
        thickness = Config::Layout::Glow::loopBoxOutlineThicknessInteracting;
    }

    g.setColour(hollowColor);
    const float halfBoxWidth = Config::Layout::Glow::loopMarkerBoxWidth / 2.0f;

    const float startX = inX + halfBoxWidth;
    const float endX = outX - halfBoxWidth;

    if (startX < endX)
    {
        g.drawLine(startX, (float)bounds.getY(), endX, (float)bounds.getY(), thickness);
        g.drawLine(startX, (float)bounds.getY() + boxHeight, endX, (float)bounds.getY() + boxHeight, thickness);
        g.drawLine(startX, (float)bounds.getBottom() - 1.0f, endX, (float)bounds.getBottom() - 1.0f, thickness);
        g.drawLine(startX, (float)bounds.getBottom() - boxHeight, endX, (float)bounds.getBottom() - boxHeight, thickness);
    }
}
