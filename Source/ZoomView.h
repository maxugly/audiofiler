/**
 * @file ZoomView.h
 * @brief Defines the ZoomView class.
 * @ingroup Views
 */

#ifndef AUDIOFILER_ZOOMVIEW_H
#define AUDIOFILER_ZOOMVIEW_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

#include "AppEnums.h"

/**
 * @class ControlPanel
 * @brief Home: View.
 *
 */
class ControlPanel;

/**
 * @class ZoomView
 * @brief Dedicated view for rendering the zoomed-in waveform popup.
 */
class ZoomView : public juce::Component
{
public:
    /**
     * @brief Undocumented method.
     * @param owner [in] Description for owner.
     */
    explicit ZoomView(ControlPanel& owner);
    ~ZoomView() override = default;

    /**
     * @brief Renders the component.
     * @param g [in] Description for g.
     */
    void paint(juce::Graphics& g) override;

    
    void updateZoomState();

private:
    ControlPanel& owner;

    juce::Rectangle<int> lastPopupBounds;
    int lastMouseX{-1};
    int lastMouseY{-1};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZoomView)
};

#endif 
