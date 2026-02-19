#ifndef AUDIOFILER_CUTLAYERVIEW_H
#define AUDIOFILER_CUTLAYERVIEW_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

#include "Config.h"

class SessionState;
class SilenceDetector;
class MouseHandler;
class WaveformManager;

class CutLayerView : public juce::Component
{
public:
    CutLayerView(SessionState& sessionState,
                 SilenceDetector& silenceDetector,
                 WaveformManager& waveformManager,
                 std::function<float()> glowAlphaProvider);

    void setMouseHandler(MouseHandler& mouseHandlerIn) { mouseHandler = &mouseHandlerIn; }
    void setMarkersVisible(bool visible) { markersVisible = visible; repaint(); }

    void paint(juce::Graphics& g) override;

private:
    SessionState& sessionState;
    SilenceDetector& silenceDetector;
    MouseHandler* mouseHandler = nullptr;
    WaveformManager& waveformManager;
    std::function<float()> glowAlphaProvider;
    bool markersVisible = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CutLayerView)
};

#endif // AUDIOFILER_CUTLAYERVIEW_H
