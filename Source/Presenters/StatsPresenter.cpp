

#include "Presenters/StatsPresenter.h"

#include "UI/ControlPanel.h"
#include "Core/AudioPlayer.h"
#include <cmath>

StatsPresenter::StatsPresenter(ControlPanel& ownerIn)
    : owner(ownerIn)
{
    owner.addAndMakeVisible(statsOverlay);
    auto& statsDisplay = statsOverlay.statsDisplay;
    statsDisplay.setReadOnly(true);
    statsDisplay.setMultiLine(true);
    statsDisplay.setWantsKeyboardFocus(false);
    statsDisplay.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    statsDisplay.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    statsDisplay.setColour(juce::TextEditor::textColourId, Config::Colors::statsText);
    statsOverlay.setVisible(false);

    statsOverlay.onHeightChanged = [this](int newHeight) {
        currentHeight = newHeight;
    };
}

void StatsPresenter::updateStats()
{
    setDisplayText(buildStatsString(), Config::Colors::statsText);
}

void StatsPresenter::toggleVisibility()
{

    setShouldShowStats(!showStats);
}

void StatsPresenter::setShouldShowStats(bool shouldShowStats)
{
    showStats = shouldShowStats;

    updateVisibility();
    owner.resized(); 
}

juce::String StatsPresenter::getStatsText() const
{
    return statsOverlay.statsDisplay.getText();
}

void StatsPresenter::layoutWithin(const juce::Rectangle<int>& contentAreaBounds)
{
    auto statsBounds = contentAreaBounds.withHeight(currentHeight)
                                        .reduced(Config::Layout::Stats::sideMargin, 0)
                                        .translated(0, Config::Layout::Stats::topMargin);
    if (showStats)
    {
        statsOverlay.setBounds(statsBounds);
        statsOverlay.toFront(true);
    }

    updateVisibility();
}

void StatsPresenter::setDisplayText(const juce::String& text, juce::Colour color)
{
    statsOverlay.statsDisplay.setText(text, juce::dontSendNotification);
    statsOverlay.statsDisplay.setColour(juce::TextEditor::textColourId, color);
}

juce::TextEditor& StatsPresenter::getDisplay()
{
    return statsOverlay.statsDisplay;
}

void StatsPresenter::setDisplayEnabled(bool shouldEnable)
{
    statsOverlay.statsDisplay.setEnabled(shouldEnable);
}

juce::String StatsPresenter::buildStatsString() const
{
    juce::String stats;
    AudioPlayer& audioPlayer = owner.getAudioPlayer();
    auto& thumbnail = audioPlayer.getThumbnail();
    double sampleRate = 0.0;
    juce::int64 lengthInSamples = 0;

    if (thumbnail.getTotalLength() > 0.0 && audioPlayer.getReaderInfo(sampleRate, lengthInSamples))
    {
        stats << "File: " << audioPlayer.getLoadedFile().getFileName() << "\n";
        stats << "Samples Loaded: " << lengthInSamples << "\n";
        stats << "Sample Rate: " << sampleRate << " Hz\n";
        stats << "Channels: " << thumbnail.getNumChannels() << "\n";
        stats << "Length: " << owner.formatTime(thumbnail.getTotalLength()) << "\n";

        float minVal = 0.0f;
        float maxVal = 0.0f;
        thumbnail.getApproximateMinMax(0.0, thumbnail.getTotalLength(), 0, minVal, maxVal);
        stats << "Approx Peak (Ch 0): " << juce::jmax(std::abs(minVal), std::abs(maxVal)) << "\n";
        stats << "Min: " << minVal << ", Max: " << maxVal << "\n";

        if (thumbnail.getNumChannels() > 1)
        {
            thumbnail.getApproximateMinMax(0.0, thumbnail.getTotalLength(), 1, minVal, maxVal);
            stats << "Approx Peak (Ch 1): " << juce::jmax(std::abs(minVal), std::abs(maxVal)) << "\n";
            stats << "Min: " << minVal << ", Max: " << maxVal << "\n";
        }
    }
    else
    {
        stats << "No file loaded or error reading audio.";
    }

    return stats;
}

void StatsPresenter::updateVisibility()
{
    statsOverlay.setVisible(showStats);
    if (showStats)
        statsOverlay.toFront(true);
}
