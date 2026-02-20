

#ifndef AUDIOFILER_STATSPRESENTER_H
#define AUDIOFILER_STATSPRESENTER_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

#include "Utils/Config.h"

class ControlPanel;

class StatsOverlay : public juce::Component
{
public:
    StatsOverlay()
        : resizer (this, &constrainer, juce::ResizableEdgeComponent::bottomEdge)
    {

        setInterceptsMouseClicks(false, true);

        addAndMakeVisible(statsDisplay);

        addAndMakeVisible(resizer);

        statsDisplay.setInterceptsMouseClicks(false, false);

        constrainer.setMinimumHeight(Config::Layout::Stats::minHeight);
        constrainer.setMaximumHeight(Config::Layout::Stats::maxHeight);
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(Config::Colors::statsBackground);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), Config::Layout::Stats::cornerRadius);

        auto handleArea = getLocalBounds().removeFromBottom(Config::Layout::Stats::handleAreaHeight);
        g.setColour(juce::Colours::white.withAlpha(Config::Layout::Stats::handleAlpha));

        g.fillRect(handleArea.withSizeKeepingCentre(Config::Layout::Stats::handleWidth, Config::Layout::Stats::handleLineHeight).translated(0, -1));
        g.fillRect(handleArea.withSizeKeepingCentre(Config::Layout::Stats::handleWidth, Config::Layout::Stats::handleLineHeight).translated(0, 2));
    }

    void resized() override
    {
        auto b = getLocalBounds();
        auto handleArea = b.removeFromBottom(Config::Layout::Stats::handleAreaHeight);
        statsDisplay.setBounds(b.reduced(Config::Layout::Stats::internalPadding));
        resizer.setBounds(handleArea);
        if (onHeightChanged)
            onHeightChanged(getHeight());
    }

    std::function<void(int)> onHeightChanged;
    juce::TextEditor statsDisplay;
    juce::ResizableEdgeComponent resizer;

private:
    juce::ComponentBoundsConstrainer constrainer;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatsOverlay)
};

class StatsPresenter final
{
public:

    explicit StatsPresenter(ControlPanel& owner);

    ~StatsPresenter() = default;

    void updateStats();

    void toggleVisibility();

    void setShouldShowStats(bool shouldShowStats);

    bool isShowingStats() const noexcept { return showStats; }

    juce::String getStatsText() const;

    void layoutWithin(const juce::Rectangle<int>& contentAreaBounds);

    void setDisplayText(const juce::String& text,
                        juce::Colour color = Config::Colors::statsText);

    juce::TextEditor& getDisplay();

    void setDisplayEnabled(bool shouldEnable);

private:

    juce::String buildStatsString() const;

    void updateVisibility();

    ControlPanel& owner;
    StatsOverlay statsOverlay;
    bool showStats { false };
    int currentHeight { Config::Layout::Stats::initialHeight };
};

#endif 
