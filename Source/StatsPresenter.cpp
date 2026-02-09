#include "StatsPresenter.h"

#include "ControlPanel.h"
#include "AudioPlayer.h"
#include <cmath>

StatsPresenter::StatsPresenter(ControlPanel& ownerIn)
    : owner(ownerIn)
{
    owner.addAndMakeVisible(statsDisplay);
    statsDisplay.setReadOnly(true);
    statsDisplay.setMultiLine(true);
    statsDisplay.setWantsKeyboardFocus(false);
    statsDisplay.setColour(juce::TextEditor::backgroundColourId, Config::statsDisplayBackgroundColour);
    statsDisplay.setColour(juce::TextEditor::textColourId, Config::statsDisplayTextColour);
    statsDisplay.setVisible(false);
}

void StatsPresenter::updateStats()
{
    setDisplayText(buildStatsString(), Config::statsDisplayTextColour);
}

void StatsPresenter::toggleVisibility()
{
    setShouldShowStats(!showStats);
}

void StatsPresenter::setShouldShowStats(bool shouldShowStats)
{
    showStats = shouldShowStats;
    updateVisibility();
    owner.resized(); // Reflow layout so the stats bounds get recomputed.
}

juce::String StatsPresenter::getStatsText() const
{
    return statsDisplay.getText();
}

void StatsPresenter::layoutWithin(const juce::Rectangle<int>& contentAreaBounds)
{
    statsBounds = contentAreaBounds.withHeight(100).reduced(10);
    if (showStats)
    {
        statsDisplay.setBounds(statsBounds);
        statsDisplay.toFront(true);
    }
    updateVisibility();
}

void StatsPresenter::setDisplayText(const juce::String& text, juce::Colour color)
{
    statsDisplay.setText(text, juce::dontSendNotification);
    statsDisplay.setColour(juce::TextEditor::textColourId, color);
}

juce::TextEditor& StatsPresenter::getDisplay()
{
    return statsDisplay;
}

void StatsPresenter::setDisplayEnabled(bool shouldEnable)
{
    statsDisplay.setEnabled(shouldEnable);
}

juce::String StatsPresenter::buildStatsString() const
{
    juce::String stats;
    AudioPlayer& audioPlayer = owner.getAudioPlayer();
    auto& thumbnail = audioPlayer.getThumbnail();
    auto* reader = audioPlayer.getAudioFormatReader();

    if (thumbnail.getTotalLength() > 0.0 && reader != nullptr)
    {
        stats << "File: " << audioPlayer.getLoadedFile().getFileName() << "\n";
        stats << "Samples Loaded: " << reader->lengthInSamples << "\n";
        stats << "Sample Rate: " << reader->sampleRate << " Hz\n";
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
    statsDisplay.setVisible(showStats);
    if (showStats)
        statsDisplay.toFront(true);
}
