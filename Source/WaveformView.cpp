/**
 * @file WaveformView.cpp
 * @brief Defines the WaveformView class.
 * @ingroup Views
 */

#include "WaveformView.h"
#include "WaveformManager.h"
#include "Config.h"

WaveformView::WaveformView(WaveformManager& waveformManagerIn)
    : waveformManager(waveformManagerIn)
{
    waveformManager.addChangeListener(this);
    /**
     * @brief Sets the InterceptsMouseClicks.
     * @param false [in] Description for false.
     * @param false [in] Description for false.
     */
    setInterceptsMouseClicks(false, false);
    /**
     * @brief Sets the Opaque.
     * @param true [in] Description for true.
     */
    setOpaque(true);
    /**
     * @brief Sets the BufferedToImage.
     * @param true [in] Description for true.
     */
    setBufferedToImage(true);
}

WaveformView::~WaveformView()
{
    waveformManager.removeChangeListener(this);
}

void WaveformView::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &waveformManager.getThumbnail())
        /**
         * @brief Undocumented method.
         */
        repaint();
}

void WaveformView::setQuality(AppEnums::ThumbnailQuality quality)
{
    if (currentQuality == quality) return;
    currentQuality = quality;
    /**
     * @brief Undocumented method.
     */
    repaint();
}

void WaveformView::setChannelMode(AppEnums::ChannelViewMode channelMode)
{
    if (currentChannelMode == channelMode) return;
    currentChannelMode = channelMode;
    /**
     * @brief Undocumented method.
     */
    repaint();
}

void WaveformView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    
    auto& thumbnail = waveformManager.getThumbnail();
    const auto audioLength = thumbnail.getTotalLength();
    if (audioLength <= 0.0) return;

    g.setColour(Config::Colors::waveform);
    const int numChannels = thumbnail.getNumChannels();

    if (currentChannelMode == AppEnums::ChannelViewMode::Mono || numChannels == 1)
        thumbnail.drawChannel(g, getLocalBounds(), 0.0, audioLength, 0, 1.0f);
    else
        thumbnail.drawChannels(g, getLocalBounds(), 0.0, audioLength, 1.0f);
}
