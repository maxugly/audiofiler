#pragma once

#include <JuceHeader.h>

class ControlPanel;

/**
 * @class LayoutManager
 * @brief Centralises all ControlPanel layout calculations so resized() stays minimal.
 *
 * This helper keeps geometric logic together making it easier to tweak rows,
 * padding, or alternate modes without bloating ControlPanel itself.
 */
class LayoutManager
{
public:
    /**
     * @brief Creates a manager bound to a specific ControlPanel instance.
     * @param controlPanel Reference to the ControlPanel that owns the components being laid out.
     */
    explicit LayoutManager(ControlPanel& controlPanel);

    /**
     * @brief Recomputes every sub-layout (top/bottom rows, loop controls, waveform, stats).
     * @return void.
     */
    void performLayout();

private:
    void layoutTopRowButtons(juce::Rectangle<int>& bounds, int rowHeight);
    void layoutLoopAndCutControls(juce::Rectangle<int>& bounds, int rowHeight);
    void layoutBottomRowAndTextDisplay(juce::Rectangle<int>& bounds, int rowHeight);
    void layoutWaveformAndStats(juce::Rectangle<int>& bounds);

    ControlPanel& controlPanel;
};
