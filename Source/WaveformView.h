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
 * @brief Draws the waveform thumbnail.
 */
class WaveformView : public juce::Component, public juce::ChangeListener
{
public:
    void invalidateWaveformCache();

    /**
     * @brief Constructs a waveform view.
     * @param waveformManager Reference to the waveform manager for thumbnail access.
     */
    explicit WaveformView(WaveformManager& waveformManager);
    ~WaveformView() override;

    void paint(juce::Graphics& g) override;
    void setQuality(AppEnums::ThumbnailQuality quality);
    void setChannelMode(AppEnums::ChannelViewMode channelMode);

private:
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    mutable juce::Image waveformCache;
    mutable juce::Rectangle<int> lastBounds;
    mutable double lastAudioLength = -1.0;
    mutable float lastScale = 1.0f;
    mutable int lastQuality = -1;
    mutable int lastChannelMode = -1;

    void drawWaveform(juce::Graphics& g,
                      const juce::Rectangle<int>& bounds) const;
    void drawReducedQualityWaveform(juce::Graphics& g,
                                    juce::AudioThumbnail& thumbnail,
                                    const juce::Rectangle<int>& bounds,
                                    int channel,
                                    int pixelsPerSample) const;

    WaveformManager& waveformManager;
    AppEnums::ThumbnailQuality currentQuality = AppEnums::ThumbnailQuality::Low;
    AppEnums::ChannelViewMode currentChannelMode = AppEnums::ChannelViewMode::Mono;
};

#endif // AUDIOFILER_WAVEFORMVIEW_H
