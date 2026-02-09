#pragma once

#include <JuceHeader.h>

class ControlPanel;

/**
 * @class PlaybackTextPresenter
 * @brief Formats and renders the playback timing labels.
 */
class PlaybackTextPresenter
{
public:
    explicit PlaybackTextPresenter(ControlPanel& ownerPanel);

    void render(juce::Graphics& g) const;
    void setTotalTimeStaticString(const juce::String& text) { totalTimeStaticStr = text; }
    const juce::String& getTotalTimeStaticString() const { return totalTimeStaticStr; }

private:
    juce::String buildCurrentTime() const;
    juce::String buildRemainingTime() const;

    ControlPanel& owner;
    juce::String totalTimeStaticStr;
};
