#ifndef AUDIOFILER_WAVEFORMVIEW_H
#define AUDIOFILER_WAVEFORMVIEW_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

#include "AppEnums.h"
#include "Config.h"

class WaveformManager;

/**
 * @class WaveformView
 * @brief Draws the waveform thumbnail. Uses image caching for high performance.
 */
class WaveformView : public juce::Component, public juce::ChangeListener
{
public:
    explicit WaveformView(WaveformManager& waveformManager);
    ~WaveformView() override;

    void setQuality(AppEnums::ThumbnailQuality quality);
    void setChannelMode(AppEnums::ChannelViewMode channelMode);

    void paint(juce::Graphics& g) override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    void drawWaveform(juce::Graphics& g, const juce::Rectangle<int>& bounds) const;

    WaveformManager& waveformManager;
    AppEnums::ThumbnailQuality currentQuality = AppEnums::ThumbnailQuality::Low;
    AppEnums::ChannelViewMode currentChannelMode = AppEnums::ChannelViewMode::Mono;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformView)
};

#endif // AUDIOFILER_WAVEFORMVIEW_H
