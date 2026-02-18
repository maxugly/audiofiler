#ifndef AUDIOFILER_WAVEFORMRENDERER_H
#define AUDIOFILER_WAVEFORMRENDERER_H

#include <JuceHeader.h>
#include "AppEnums.h"
#include "Config.h"

class SessionState;
class WaveformManager;

/**
 * @class WaveformRenderer
 * @brief Renders the waveform, loop overlays, playback cursor, and mouse feedback for the ControlPanel.
 *
 * Extracting this class keeps ControlPanel::paint concise while making it easier to reason about
 * future visual tweaks or alternate render modes.
 */
class WaveformRenderer : public juce::ChangeListener
{
public:
    void invalidateWaveformCache();
    /**
     * @brief Constructs a renderer bound to a ControlPanel.
     * @param sessionState Reference to the session state for Cut points.
     * @param waveformManager Reference to the waveform manager for thumbnail access.
     */
    explicit WaveformRenderer(SessionState& sessionState, WaveformManager& waveformManager);
    ~WaveformRenderer() override;

    /**
     * @brief Paints the waveform and overlays for the ControlPanel.
     * @param g Graphics context supplied by ControlPanel::paint.
     */
    /**
     * @brief Paints the cached waveform (static).
     * @param g Graphics context supplied by ControlPanel::paint.
     */
    void renderWaveform(juce::Graphics& g,
                        const juce::Rectangle<int>& bounds,
                        AppEnums::ThumbnailQuality quality,
                        AppEnums::ChannelViewMode channelMode);

private:
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    mutable juce::Image waveformCache;
    mutable juce::Rectangle<int> lastBounds;
    mutable double lastAudioLength = -1.0;
    mutable float lastScale = 1.0f;
    mutable int lastQuality = -1;
    mutable int lastChannelMode = -1;
    void drawWaveform(juce::Graphics& g,
                      const juce::Rectangle<int>& bounds,
                      AppEnums::ThumbnailQuality quality,
                      AppEnums::ChannelViewMode channelMode) const;
    void drawReducedQualityWaveform(juce::Graphics& g,
                                    juce::AudioThumbnail& thumbnail,
                                    const juce::Rectangle<int>& bounds,
                                    int channel,
                                    int pixelsPerSample) const;
    SessionState& sessionState;
    WaveformManager& waveformManager;
};

#endif // AUDIOFILER_WAVEFORMRENDERER_H
