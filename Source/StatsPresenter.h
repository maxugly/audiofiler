#pragma once

#include <JuceHeader.h>
#include "Config.h"

class ControlPanel;

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
                        juce::Colour color = Config::statsDisplayTextColour);

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
    juce::String buildStatsString() const;
    void updateVisibility();

    ControlPanel& owner;
    juce::TextEditor statsDisplay;
    bool showStats { false };
    juce::Rectangle<int> statsBounds;
};
