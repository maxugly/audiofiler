

#include "CutLayerView.h"

#include "SessionState.h"
#include "SilenceDetector.h"
#include "MouseHandler.h"
#include "WaveformManager.h"
#include "CoordinateMapper.h"
#include "ControlPanel.h"

CutLayerView::CutLayerView(ControlPanel& ownerIn,
                           SessionState& sessionStateIn,
                           SilenceDetector& silenceDetectorIn,
                           WaveformManager& waveformManagerIn,
                           std::function<float()> glowAlphaProviderIn)
    : owner(ownerIn),
      sessionState(sessionStateIn),
      silenceDetector(silenceDetectorIn),
      waveformManager(waveformManagerIn),
      glowAlphaProvider(std::move(glowAlphaProviderIn))
{

    setInterceptsMouseClicks(false, false);

    setOpaque(false);

    setBufferedToImage(true);
    waveformManager.addChangeListener(this);
}

CutLayerView::~CutLayerView()
{
    waveformManager.removeChangeListener(this);
}

void CutLayerView::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &waveformManager.getThumbnail())

        repaint();
}

void CutLayerView::setChannelMode(AppEnums::ChannelViewMode mode)
{
    if (currentChannelMode == mode) return;
    currentChannelMode = mode;

    repaint();
}

void CutLayerView::setQuality(AppEnums::ThumbnailQuality quality)
{
    if (currentQuality == quality) return;
    currentQuality = quality;

    repaint();
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

        const float xPos = (float)bounds.getX() + CoordinateMapper::secondsToPixels(cutPos, (float)bounds.getWidth(), (double)audioLength);
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

    const float inX = juce::jlimit((float)bounds.getX(), (float)bounds.getRight(), (float)bounds.getX() + CoordinateMapper::secondsToPixels(actualIn, (float)bounds.getWidth(), (double)audioLength));
    const float outX = juce::jlimit((float)bounds.getX(), (float)bounds.getRight(), (float)bounds.getX() + CoordinateMapper::secondsToPixels(actualOut, (float)bounds.getWidth(), (double)audioLength));

    const float fadeLength = bounds.getWidth() * Config::Layout::Waveform::cutRegionFadeProportion;
    const float boxHeight = (float)Config::Layout::Glow::cutMarkerBoxHeight;

    const juce::Rectangle<float> leftRegion((float)bounds.getX(), (float)bounds.getY(), juce::jmax(0.0f, inX - (float)bounds.getX()), (float)bounds.getHeight());
    if (leftRegion.getWidth() > 0.0f)
    {
        const float actualFade = juce::jmin(fadeLength, leftRegion.getWidth());

        juce::Rectangle<float> solidBlackLeft = leftRegion.withWidth(juce::jmax(0.0f, leftRegion.getWidth() - actualFade));
        g.setColour(juce::Colours::black);
        g.fillRect(solidBlackLeft);

        juce::Rectangle<float> fadeAreaLeft(inX - actualFade, (float)bounds.getY(), actualFade, (float)bounds.getHeight());
        juce::ColourGradient leftFadeGradient(Config::Colors::cutRegion, inX, leftRegion.getCentreY(),
                                              juce::Colours::black, inX - actualFade, leftRegion.getCentreY(), false);
        g.setGradientFill(leftFadeGradient);
        g.fillRect(fadeAreaLeft);
    }

    const juce::Rectangle<float> rightRegion(outX, (float)bounds.getY(), juce::jmax(0.0f, (float)bounds.getRight() - outX), (float)bounds.getHeight());
    if (rightRegion.getWidth() > 0.0f)
    {
        const float actualFade = juce::jmin(fadeLength, rightRegion.getWidth());

        float solidBlackStart = outX + actualFade;
        juce::Rectangle<float> solidBlackRight(solidBlackStart, (float)bounds.getY(), juce::jmax(0.0f, (float)bounds.getRight() - solidBlackStart), (float)bounds.getHeight());
        g.setColour(juce::Colours::black);
        g.fillRect(solidBlackRight);

        juce::Rectangle<float> fadeAreaRight(outX, (float)bounds.getY(), actualFade, (float)bounds.getHeight());
        juce::ColourGradient rightFadeGradient(Config::Colors::cutRegion, outX, rightRegion.getCentreY(),
                                               juce::Colours::black, outX + actualFade, rightRegion.getCentreY(), false);
        g.setGradientFill(rightFadeGradient);
        g.fillRect(fadeAreaRight);
    }

    const juce::Colour glowColor = Config::Colors::cutLine.withAlpha(Config::Colors::cutLine.getFloatAlpha() * (1.0f - glowAlphaProvider()));
    g.setColour(glowColor);
    g.fillRect(inX - (Config::Layout::Glow::cutLineGlowThickness * Config::Layout::Glow::offsetFactor - 0.5f), (float)bounds.getY() + boxHeight, Config::Layout::Glow::cutLineGlowThickness, (float)bounds.getHeight() - (2.0f * boxHeight));
    g.fillRect(outX - (Config::Layout::Glow::cutLineGlowThickness * Config::Layout::Glow::offsetFactor - 0.5f), (float)bounds.getY() + boxHeight, Config::Layout::Glow::cutLineGlowThickness, (float)bounds.getHeight() - (2.0f * boxHeight));

    g.setColour(Config::Colors::cutLine);
    auto drawCutMarker = [&](float x, MouseHandler::CutMarkerHandle handleType) {
        juce::Colour markerColor = Config::Colors::cutLine;

        if (handleType == MouseHandler::CutMarkerHandle::In && silenceDetector.getIsAutoCutInActive())
            markerColor = Config::Colors::cutMarkerAuto;
        else if (handleType == MouseHandler::CutMarkerHandle::Out && silenceDetector.getIsAutoCutOutActive())
            markerColor = Config::Colors::cutMarkerAuto;

        float thickness = Config::Layout::Glow::cutBoxOutlineThickness;

        if (mouseHandler != nullptr && mouseHandler->getDraggedHandle() == handleType)
        {
            markerColor = Config::Colors::cutMarkerDrag;
            thickness = Config::Layout::Glow::cutBoxOutlineThicknessInteracting;
        }
        else if (mouseHandler != nullptr && mouseHandler->getHoveredHandle() == handleType)
        {
            markerColor = Config::Colors::cutMarkerHover;
            thickness = Config::Layout::Glow::cutBoxOutlineThicknessInteracting;
        }

        const float boxWidth = Config::Layout::Glow::cutMarkerBoxWidth;
        const float halfBoxWidth = boxWidth / 2.0f;

        g.setColour(markerColor);
        g.drawRect(x - halfBoxWidth, (float)bounds.getY(), boxWidth, boxHeight, thickness);
        g.drawRect(x - halfBoxWidth, (float)bounds.getBottom() - boxHeight, boxWidth, boxHeight, thickness);

        g.setColour(markerColor);
        g.fillRect(x - Config::Layout::Glow::cutMarkerWidthThin / Config::Layout::Glow::cutMarkerCenterDivisor,
                   (float)bounds.getY() + boxHeight,
                   Config::Layout::Glow::cutMarkerWidthThin,
                   (float)bounds.getHeight() - (2.0f * boxHeight));
    };

    drawCutMarker(inX, MouseHandler::CutMarkerHandle::In);

    drawCutMarker(outX, MouseHandler::CutMarkerHandle::Out);

    juce::Colour hollowColor = Config::Colors::cutLine;
    float thickness = Config::Layout::Glow::cutBoxOutlineThickness;

    if (mouseHandler != nullptr && mouseHandler->getDraggedHandle() == MouseHandler::CutMarkerHandle::Full)
    {
        hollowColor = Config::Colors::cutMarkerDrag;
        thickness = Config::Layout::Glow::cutBoxOutlineThicknessInteracting;
    }
    else if (mouseHandler != nullptr && mouseHandler->getHoveredHandle() == MouseHandler::CutMarkerHandle::Full)
    {
        hollowColor = Config::Colors::cutMarkerHover;
        thickness = Config::Layout::Glow::cutBoxOutlineThicknessInteracting;
    }

    g.setColour(hollowColor);
    const float halfBoxWidth = Config::Layout::Glow::cutMarkerBoxWidth / 2.0f;

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
