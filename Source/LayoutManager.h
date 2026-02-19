/**
 * @file LayoutManager.h
 * @brief Defines the LayoutManager class.
 * @ingroup Engine
 */

#ifndef AUDIOFILER_LAYOUTMANAGER_H
#define AUDIOFILER_LAYOUTMANAGER_H

#include <JuceHeader.h>

/**
 * @class ControlPanel
 * @brief Home: View.
 *
 */
class ControlPanel;

/**
 * @class LayoutManager
 * @brief Handles the positioning and sizing of all child components within the ControlPanel.
 */
class LayoutManager final
{
public:
    /**
     * @brief Undocumented method.
     * @param controlPanelIn [in] Description for controlPanelIn.
     */
    explicit LayoutManager(ControlPanel& controlPanelIn);

    /**
     * @brief Recomputes every sub-layout (top/bottom rows, cut controls, waveform, stats).
     */
    void performLayout();

private:
    /**
     * @brief Undocumented method.
     * @param bounds [in] Description for bounds.
     * @param rowHeight [in] Description for rowHeight.
     */
    void layoutTopRowButtons(juce::Rectangle<int>& bounds, int rowHeight);
    /**
     * @brief Undocumented method.
     * @param bounds [in] Description for bounds.
     * @param rowHeight [in] Description for rowHeight.
     */
    void layoutCutControls(juce::Rectangle<int>& bounds, int rowHeight);
    /**
     * @brief Undocumented method.
     * @param bounds [in] Description for bounds.
     * @param rowHeight [in] Description for rowHeight.
     */
    void layoutBottomRowAndTextDisplay(juce::Rectangle<int>& bounds, int rowHeight);
    /**
     * @brief Undocumented method.
     * @param bounds [in] Description for bounds.
     */
    void layoutWaveformAndStats(juce::Rectangle<int>& bounds);

    ControlPanel& controlPanel;
};

#endif // AUDIOFILER_LAYOUTMANAGER_H
