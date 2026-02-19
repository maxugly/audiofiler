/**
 * @file PlaybackCursorView.h
 * @brief Defines the PlaybackCursorView class.
 * @ingroup Views
 */

#ifndef AUDIOFILER_PLAYBACKCURSORVIEW_H
#define AUDIOFILER_PLAYBACKCURSORVIEW_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

/**
 * @class ControlPanel
 * @brief Home: View.
 *
 */
class ControlPanel;

/**
 * @class PlaybackCursorView
 * @brief A transparent component that renders dynamic elements (cursors, overlays)
 *        on top of the waveform to avoid full repaints of the ControlPanel.
 */
class PlaybackCursorView : public juce::Component
{
public:
    /**
     * @brief Undocumented method.
     * @param owner [in] Description for owner.
     */
    explicit PlaybackCursorView(ControlPanel& owner);
    ~PlaybackCursorView() override = default;

    /**
     * @brief Renders the component.
     * @param g [in] Description for g.
     */
    void paint(juce::Graphics& g) override;

private:
    ControlPanel& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaybackCursorView)
};

#endif 
