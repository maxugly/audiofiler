/**
 * @file WaveformView.h
 * @brief Defines the WaveformView class.
 * @ingroup Views
 */

#ifndef AUDIOFILER_WAVEFORMVIEW_H
#define AUDIOFILER_WAVEFORMVIEW_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

#include "AppEnums.h"
#include "Config.h"

/**
 * @class WaveformManager
 * @brief Home: Engine.
 *
 */
class WaveformManager;

/**
 * @class WaveformView
 * @brief Draws the waveform thumbnail. Uses image caching for high performance.
 */
class WaveformView : public juce::Component, public juce::ChangeListener
{
public:
    /**
     * @brief Undocumented method.
     * @param waveformManager [in] Description for waveformManager.
     */
    explicit WaveformView(WaveformManager& waveformManager);
    /**
     * @brief Undocumented method.
     */
    ~WaveformView() override;

    /**
     * @brief Sets the Quality.
     * @param quality [in] Description for quality.
     */
    void setQuality(AppEnums::ThumbnailQuality quality);
    /**
     * @brief Sets the ChannelMode.
     * @param channelMode [in] Description for channelMode.
     */
    void setChannelMode(AppEnums::ChannelViewMode channelMode);

    /**
     * @brief Renders the component.
     * @param g [in] Description for g.
     */
    void paint(juce::Graphics& g) override;
    /**
     * @brief Undocumented method.
     * @param source [in] Description for source.
     */
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    /**
     * @brief Undocumented method.
     * @param g [in] Description for g.
     * @param bounds [in] Description for bounds.
     */
    void drawWaveform(juce::Graphics& g, const juce::Rectangle<int>& bounds) const;

    WaveformManager& waveformManager;
    AppEnums::ThumbnailQuality currentQuality = AppEnums::ThumbnailQuality::Low;
    AppEnums::ChannelViewMode currentChannelMode = AppEnums::ChannelViewMode::Mono;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformView)
};

#endif 
