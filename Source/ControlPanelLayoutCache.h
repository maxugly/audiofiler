#ifndef AUDIOFILER_CONTROLPANELLAYOUTCACHE_H
#define AUDIOFILER_CONTROLPANELLAYOUTCACHE_H

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
    #include <juce_graphics/juce_graphics.h>
#else
    #include <JuceHeader.h>
#endif

/**
 * @struct ControlPanelLayoutCache
 * @brief Stores geometry values computed during ControlPanel layout.
 */
struct ControlPanelLayoutCache
{
    juce::Rectangle<int> waveformBounds;
    juce::Rectangle<int> contentAreaBounds;
    int bottomRowTopY = 0;
    int playbackLeftTextX = 0;
    int playbackRightTextX = 0;
    int playbackCenterTextX = 0;
};


#endif // AUDIOFILER_CONTROLPANELLAYOUTCACHE_H
