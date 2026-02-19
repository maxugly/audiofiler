

#ifndef AUDIOFILER_CUTLAYERVIEW_H
#define AUDIOFILER_CUTLAYERVIEW_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

#include "Config.h"
#include "AppEnums.h"

class SessionState;

class SilenceDetector;

class MouseHandler;

class WaveformManager;

class ControlPanel;

class CutLayerView : public juce::Component,
                     public juce::ChangeListener
{
public:
    CutLayerView(ControlPanel& owner,
                 SessionState& sessionState,
                 SilenceDetector& silenceDetector,
                 WaveformManager& waveformManager,
                 std::function<float()> glowAlphaProvider);

    ~CutLayerView() override;

    void setMouseHandler(MouseHandler& mouseHandlerIn) { mouseHandler = &mouseHandlerIn; }

    void setMarkersVisible(bool visible) { markersVisible = visible; repaint(); }

    void setChannelMode(AppEnums::ChannelViewMode mode);

    void setQuality(AppEnums::ThumbnailQuality quality);

    void paint(juce::Graphics& g) override;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    ControlPanel& owner;
    SessionState& sessionState;
    SilenceDetector& silenceDetector;
    MouseHandler* mouseHandler = nullptr;
    WaveformManager& waveformManager;
    std::function<float()> glowAlphaProvider;
    bool markersVisible = false;

    AppEnums::ChannelViewMode currentChannelMode = AppEnums::ChannelViewMode::Mono;
    AppEnums::ThumbnailQuality currentQuality = AppEnums::ThumbnailQuality::Low;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CutLayerView)
};

#endif 
