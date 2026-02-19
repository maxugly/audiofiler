/**
 * @file StatsPresenter.h
 * @brief Defines the StatsPresenter class.
 * @ingroup Presenters
 */

#ifndef AUDIOFILER_STATSPRESENTER_H
#define AUDIOFILER_STATSPRESENTER_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

#include "Config.h"

/**
 * @class ControlPanel
 * @brief Home: View.
 *
 */
class ControlPanel;

/**
 * @class StatsOverlay
 * @brief A component that contains the stats display and a resize handle.
 */
class StatsOverlay : public juce::Component
{
public:
    StatsOverlay()
        : resizer (this, &constrainer, juce::ResizableEdgeComponent::bottomEdge)
    {
        /**
         * @brief Sets the InterceptsMouseClicks.
         * @param false [in] Description for false.
         * @param true [in] Description for true.
         */
        setInterceptsMouseClicks(false, true);
        /**
         * @brief Undocumented method.
         * @param statsDisplay [in] Description for statsDisplay.
         */
        addAndMakeVisible(statsDisplay);
        /**
         * @brief Undocumented method.
         * @param resizer [in] Description for resizer.
         */
        addAndMakeVisible(resizer);

        statsDisplay.setInterceptsMouseClicks(false, false);
        
        constrainer.setMinimumHeight(Config::Layout::Stats::minHeight);
        constrainer.setMaximumHeight(Config::Layout::Stats::maxHeight);
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(Config::Colors::statsBackground);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), Config::Layout::Stats::cornerRadius);

        // Draw a subtle resize handle at the bottom
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

/**
 * @class StatsPresenter
 * @brief Encapsulates the stats TextEditor along with the logic for building and updating its content.
 *
 * This helper keeps ControlPanel lean by handling the creation, layout, visibility, and content of
 * the stats display. It queries the owner ControlPanel for audio state and formats the resulting
 * information before presenting it to the user.
 */
class StatsPresenter final
{
public:
    /**
     * @brief Constructs the presenter and attaches the stats display to the owning ControlPanel.
     * @param owner The ControlPanel that owns the stats UI.
     */
    explicit StatsPresenter(ControlPanel& owner);

    /**
     * @brief Default destructor.
     */
    ~StatsPresenter() = default;

    /**
     * @brief Builds and displays the latest audio statistics.
     */
    void updateStats();

    /**
     * @brief Toggles the visibility state of the stats display.
     */
    void toggleVisibility();

    /**
     * @brief Forces a specific visibility state for the stats display.
     * @param shouldShowStats True to show the panel, false to hide it.
     */
    void setShouldShowStats(bool shouldShowStats);

    /**
     * @brief Reports whether the stats panel is currently visible.
     * @return True if visible, false otherwise.
     */
    bool isShowingStats() const noexcept { return showStats; }

    /**
     * @brief Retrieves the text currently displayed in the stats editor.
     * @return A copy of the displayed text.
     */
    juce::String getStatsText() const;

    /**
     * @brief Repositions the stats display inside the provided content bounds.
     * @param contentAreaBounds The bounds reserved for content inside the ControlPanel.
     */
    void layoutWithin(const juce::Rectangle<int>& contentAreaBounds);

    /**
     * @brief Sets the stats text directly, optionally overriding its colour.
     * @param text The text to display.
     * @param color The colour to apply to the text.
     */
    void setDisplayText(const juce::String& text,
                        juce::Colour color = Config::Colors::statsText);

    /**
     * @brief Provides access to the underlying TextEditor for log-style append operations.
     * @return Reference to the managed TextEditor.
     */
    juce::TextEditor& getDisplay();

    /**
     * @brief Updates the enabled state of the stats editor to track global control state.
     * @param shouldEnable True to enable the editor, false to disable it.
     */
    void setDisplayEnabled(bool shouldEnable);

private:
    /**
     * @brief Undocumented method.
     * @return juce::String
     */
    juce::String buildStatsString() const;
    /**
     * @brief Undocumented method.
     */
    void updateVisibility();

    ControlPanel& owner;
    StatsOverlay statsOverlay;
    bool showStats { false };
    int currentHeight { Config::Layout::Stats::initialHeight };
};

#endif // AUDIOFILER_STATSPRESENTER_H
