#ifndef AUDIOFILER_CONTROLPANEL_H
#define AUDIOFILER_CONTROLPANEL_H

#include "Presenters/PlaybackTimerManager.h"
#include "UI/InteractionCoordinator.h"

class FocusManager;
#include "Core/AppEnums.h"
#include "Core/AudioPlayer.h" 
#include "Utils/Config.h"
#include "UI/Components/TransportButton.h"
#include "UI/Components/TransportStrip.h"
#include "UI/Components/MarkerStrip.h"
#include "UI/LookAndFeel/ModernLookAndFeel.h" 
#include "UI/MouseHandler.h"      
#include "Core/SessionState.h"
#include "Workers/SilenceDetector.h"   
#include "Workers/SilenceWorkerClient.h"

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
    #include <juce_gui_basics/juce_gui_basics.h>
    #include <juce_events/juce_events.h>
#else
    #include <JuceHeader.h>
#endif

#include <memory> 
#include <tuple>

class MainComponent; 

class LayoutManager;

class WaveformView;

class CutLayerView;

class CutPresenter;

class StatsPresenter;

class RepeatPresenter;

class ControlStatePresenter;

class TransportPresenter;

class SilenceDetectionPresenter;

class ControlButtonsPresenter;

class CutResetPresenter;

class CutButtonPresenter;

class PlaybackTextPresenter;

class PlaybackCursorView;

class ZoomView;

class PlaybackRepeatController;

class TransportStrip;

class MarkerStrip;

class OverlayView;

/**
 * @ingroup UI
 * @class ControlPanel
 * @brief The main container for UI controls and waveform visualization.
 * @details This component instantiates and manages all the presenters and views (e.g.,
 *          `WaveformView`, `TransportPresenter`). It handles the layout logic and
 *          coordinates communication between the UI and the `SessionState`.
 *
 *          It acts as the "Glue" layer, ensuring presenters have access to the models they need.
 *
 * @see MainComponent
 * @see SessionState
 */
class ControlPanel final : public juce::Component,
                           public SessionState::Listener {
public:
  struct LayoutCache {
    juce::Rectangle<int> waveformBounds;
    juce::Rectangle<int> contentAreaBounds;
    int bottomRowTopY{0};
    int playbackLeftTextX{0};
    int playbackCenterTextX{0};
    int playbackRightTextX{0};
  };

  explicit ControlPanel(MainComponent &owner, SessionState &sessionStateIn);

  ~ControlPanel() override;

  float getZoomFactor() const { return m_zoomFactor; }

  void setZoomFactor(float factor) {
    m_zoomFactor = juce::jlimit(1.0f, 1000000.0f, factor);

    repaint();
  }

  // SessionState::Listener
  void cutPreferenceChanged(const MainDomain::CutPreferences& prefs) override;
  void cutInChanged(double value) override;
  void cutOutChanged(double value) override;

  void jumpToCutIn();

  void paint(juce::Graphics &g) override;

  void resized() override;

  void updatePlayButtonText(bool isPlaying);

  void refreshLabels();

  void updateComponentStates();

  void updateUIFromState();

  void setAutoCutInActive(bool isActive);

  void setAutoCutOutActive(bool isActive);

  void updateCutButtonColors();

  bool getShouldRepeat() const { return shouldRepeat; }

  void setShouldRepeat(bool shouldRepeatParam);

  double getCutInPosition() const;

  double getCutOutPosition() const;

  void setCutInPosition(double pos);

  void setCutOutPosition(double pos);

  void ensureCutOrder();

  void toggleStats();

  void triggerModeButton();

  void triggerChannelViewButton();

  void triggerRepeatButton();

  void resetIn();

  void resetOut();

  void setShouldShowStats(bool shouldShowStats);

  void setTotalTimeStaticString(const juce::String &timeString);
  void setStatsDisplayText(const juce::String &text,
                           juce::Colour color = Config::Colors::statsText);
  void logStatusMessage(const juce::String &message,
                        bool isError = false);

  juce::TextButton& getAutoCutInButton() { return inStrip->getAutoCutButton(); }
  juce::TextButton& getAutoCutOutButton() { return outStrip->getAutoCutButton(); }

  void updateStatsFromAudio();

  AppEnums::PlacementMode getPlacementMode() const { return interactionCoordinator->getPlacementMode(); }

  void setPlacementMode(AppEnums::PlacementMode mode) { interactionCoordinator->setPlacementMode(mode); }

  bool shouldAutoplay() const { return sessionState.getCutPrefs().autoplay; }

  bool isCutModeActive() const { return m_isCutModeActive; }

  juce::Rectangle<int> getWaveformBounds() const {
    return layoutCache.waveformBounds;
  }

  AudioPlayer &getAudioPlayer();
  AudioPlayer &getAudioPlayer() const;
  SessionState &getSessionState() { return sessionState; }
  const SessionState &getSessionState() const { return sessionState; }

  InteractionCoordinator& getInteractionCoordinator() { return *interactionCoordinator; }

  AppEnums::ChannelViewMode getChannelViewMode() const {
    return currentChannelViewMode;
  }

  const MouseHandler &getMouseHandler() const;
  MouseHandler &getMouseHandler();

  SilenceDetector &getSilenceDetector() { return *silenceDetector; }
  const SilenceDetector &getSilenceDetector() const { return *silenceDetector; }

  TransportStrip* getTransportStrip() { return transportStrip.get(); }
  MarkerStrip* getInStrip() { return inStrip.get(); }
  MarkerStrip* getOutStrip() { return outStrip.get(); }
  SilenceDetectionPresenter* getSilenceDetectionPresenter() { return silenceDetectionPresenter.get(); }

  int getBottomRowTopY() const { return layoutCache.bottomRowTopY; }

  std::tuple<int, int, int> getPlaybackLabelXs() const {
    return {layoutCache.playbackLeftTextX, layoutCache.playbackCenterTextX,
            layoutCache.playbackRightTextX};
  }

  juce::TextEditor &getStatsDisplay();

  void setCutStart(int sampleIndex);

  void setCutEnd(int sampleIndex);

  juce::String formatTime(double seconds) const;
  const juce::LookAndFeel &getLookAndFeel() const;
  FocusManager &getFocusManager() const { return *focusManager; }

  void mouseMove(const juce::MouseEvent &event) override;

  void mouseDown(const juce::MouseEvent &event) override;

  void mouseDrag(const juce::MouseEvent &event) override;

  void mouseUp(const juce::MouseEvent &event) override;

  void mouseExit(const juce::MouseEvent &event) override;
  void mouseWheelMove(const juce::MouseEvent &event,
                      const juce::MouseWheelDetails &wheel) override;

  PlaybackTimerManager& getPlaybackTimerManager() { return *playbackTimerManager; }
  RepeatPresenter& getRepeatPresenter();
  PlaybackTextPresenter& getPlaybackTextPresenter();

private:
  friend class LayoutManager;
  friend class ControlStatePresenter;
  friend class TransportPresenter;
  friend class SilenceDetectionPresenter;
  friend class ControlButtonsPresenter;
  friend class CutButtonPresenter;
  friend class CutResetPresenter;
  friend class PlaybackTextPresenter;

  MainComponent &owner;
  /** @brief Reference to the shared application state. */
  SessionState &sessionState;
  ModernLookAndFeel modernLF;

  /** @brief Manages high-frequency updates. */
  std::unique_ptr<PlaybackTimerManager> playbackTimerManager;

  /** @brief Handles silence detection logic and background workers. */
  std::unique_ptr<SilenceDetector> silenceDetector;

  /** @brief Manages the logic for cut operations (in/out points). */
  std::unique_ptr<CutPresenter> cutPresenter;

  /** @brief Calculates and caches component positions. */
  std::unique_ptr<LayoutManager> layoutManager;

  /** @brief Renders the static waveform visualization. */
  std::unique_ptr<WaveformView> waveformView;

  /** @brief Renders the overlay for cut regions. */
  std::unique_ptr<CutLayerView> cutLayerView;

  /** @brief Manages playback position text display. */
  std::unique_ptr<PlaybackTextPresenter> playbackTextPresenter;

  /** @brief Manages file statistics (sample rate, bit depth, etc.). */
  std::unique_ptr<StatsPresenter> statsPresenter;

  /** @brief Manages repeat/loop functionality. */
  std::unique_ptr<RepeatPresenter> repeatPresenter;

  /** @brief Updates UI state based on SessionState changes. */
  std::unique_ptr<ControlStatePresenter> controlStatePresenter;

  /** @brief Manages transport controls (Play/Stop). */
  std::unique_ptr<TransportPresenter> transportPresenter;

  /** @brief Presenter for silence detection settings. */
  std::unique_ptr<SilenceDetectionPresenter> silenceDetectionPresenter;

  /** @brief Manages general control buttons (Open, Save, etc.). */
  std::unique_ptr<ControlButtonsPresenter> buttonPresenter;

  /** @brief Manages cut button interactions. */
  std::unique_ptr<CutButtonPresenter> cutButtonPresenter;

  /** @brief Manages reset buttons for cut points. */
  std::unique_ptr<CutResetPresenter> cutResetPresenter;

  /** @brief Handles keyboard focus navigation. */
  std::unique_ptr<FocusManager> focusManager;

  /** @brief Renders the playback cursor and glow effects. */
  std::unique_ptr<PlaybackCursorView> playbackCursorView;

  /** @brief Renders the zoom window. */
  std::unique_ptr<ZoomView> zoomView;

  /** @brief Renders eye-candy overlays. */
  std::unique_ptr<OverlayView> overlayView;

  /** @brief Manages transient UI interaction states. */
  std::unique_ptr<InteractionCoordinator> interactionCoordinator;

  /** @brief Manages repeat and autoplay logic. */
  std::unique_ptr<PlaybackRepeatController> playbackRepeatController;

  juce::TextButton openButton, modeButton, exitButton,
      statsButton, channelViewButton, eyeCandyButton;
  std::unique_ptr<TransportStrip> transportStrip;
  std::unique_ptr<MarkerStrip> inStrip, outStrip;
  juce::TextEditor elapsedTimeEditor, remainingTimeEditor, cutLengthEditor;

  LayoutCache layoutCache;

  AppEnums::ViewMode currentMode = AppEnums::ViewMode::Classic;
  AppEnums::ChannelViewMode currentChannelViewMode = AppEnums::ChannelViewMode::Mono;

  bool shouldRepeat = false;

  juce::String cutInDisplayString, cutOutDisplayString;
  int cutInTextX = 0, cutOutTextX = 0, cutTextY = 0;

  bool m_isCutModeActive = false;
  float m_zoomFactor = 10.0f;

  void initialiseLookAndFeel();

  void initialiseCutEditors();

  void invokeOwnerOpenDialog();

  void finaliseSetup();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlPanel)
};

#endif 
