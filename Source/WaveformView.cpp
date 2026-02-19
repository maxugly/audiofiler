#include "WaveformView.h"
#include "WaveformManager.h"
#include "Config.h"

WaveformView::WaveformView(WaveformManager& waveformManagerIn)
    : waveformManager(waveformManagerIn)
{
    waveformManager.addChangeListener(this);
    setInterceptsMouseClicks(false, false);
    setOpaque(true);
    setBufferedToImage(true);
}

WaveformView::~WaveformView()
{
    waveformManager.removeChangeListener(this);
}

void WaveformView::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &waveformManager.getThumbnail())
        repaint();
}

void WaveformView::setQuality(AppEnums::ThumbnailQuality quality)
{
    if (currentQuality == quality) return;
    currentQuality = quality;
    repaint();
}

void WaveformView::setChannelMode(AppEnums::ChannelViewMode channelMode)
{
    if (currentChannelMode == channelMode) return;
    currentChannelMode = channelMode;
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
