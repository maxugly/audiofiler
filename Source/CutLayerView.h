/**
 * @file CutLayerView.h
 * @brief Defines the CutLayerView class.
 * @ingroup Views
 */

#ifndef AUDIOFILER_CUTLAYERVIEW_H
#define AUDIOFILER_CUTLAYERVIEW_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

#include "Config.h"
#include "AppEnums.h"

/**
 * @class SessionState
 * @brief Home: Engine.
 *
 */
class SessionState;
/**
 * @class SilenceDetector
 * @brief Home: Engine.
 *
 */
class SilenceDetector;
/**
 * @class MouseHandler
 * @brief Home: Engine.
 *
 */
class MouseHandler;
/**
 * @class WaveformManager
 * @brief Home: Engine.
 *
 */
class WaveformManager;
/**
 * @class ControlPanel
 * @brief Home: View.
 *
 */
class ControlPanel;

/**
 * @class CutLayerView
 * @brief Home: View.
 *
 */
class CutLayerView : public juce::Component,
                     public juce::ChangeListener
{
public:
    CutLayerView(ControlPanel& owner,
                 SessionState& sessionState,
                 SilenceDetector& silenceDetector,
                 WaveformManager& waveformManager,
                 std::function<float()> glowAlphaProvider);

    /**
     * @brief Undocumented method.
     */
    ~CutLayerView() override;

    /**
     * @brief Sets the MouseHandler.
     * @param mouseHandlerIn [in] Description for mouseHandlerIn.
     */
    void setMouseHandler(MouseHandler& mouseHandlerIn) { mouseHandler = &mouseHandlerIn; }
    /**
     * @brief Sets the MarkersVisible.
     * @param visible [in] Description for visible.
     */
    void setMarkersVisible(bool visible) { markersVisible = visible; repaint(); }

    /**
     * @brief Sets the ChannelMode.
     * @param mode [in] Description for mode.
     */
    void setChannelMode(AppEnums::ChannelViewMode mode);
    /**
     * @brief Sets the Quality.
     * @param quality [in] Description for quality.
     */
    void setQuality(AppEnums::ThumbnailQuality quality);

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

#endif // AUDIOFILER_CUTLAYERVIEW_H
