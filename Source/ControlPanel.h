#ifndef AUDIOFILER_CONTROLPANEL_H
#define AUDIOFILER_CONTROLPANEL_H

#include "PlaybackTimerManager.h"

class FocusManager;
#include "AppEnums.h"
#include "AudioPlayer.h" 
#include "Config.h"
#include "ControlPanelLayoutCache.h"
#include "RepeatButton.h"
#include "ModernLookAndFeel.h" 
#include "MouseHandler.h"      
#include "SessionState.h"
#include "SilenceDetector.h"   
#include "SilenceWorkerClient.h"

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
                           public PlaybackTimerManager::Listener,
                           public SessionState::Listener {
public:

  explicit ControlPanel(MainComponent &owner, SessionState &sessionStateIn);

  ~ControlPanel() override;

  AppEnums::ActiveZoomPoint getActiveZoomPoint() const { return m_activeZoomPoint; }

  void setActiveZoomPoint(AppEnums::ActiveZoomPoint point);

  float getZoomFactor() const { return m_zoomFactor; }

  void setZoomFactor(float factor) {
    m_zoomFactor = juce::jlimit(1.0f, 1000000.0f, factor);

    repaint();
  }

  bool isZKeyDown() const;

  void playbackTimerTick() override;
  void animationUpdate (float breathingPulse) override;

  // SessionState::Listener
  void cutPreferenceChanged(const MainDomain::CutPreferences& prefs) override;
  void cutInChanged(double value) override;
  void cutOutChanged(double value) override;

  juce::Rectangle<int> getZoomPopupBounds() const { return m_zoomPopupBounds; }

  void setZoomPopupBounds(juce::Rectangle<int> bounds) {
    m_zoomPopupBounds = bounds;
  }

  std::pair<double, double> getZoomTimeRange() const { return m_zoomTimeRange; }

  void setZoomTimeRange(double start, double end) {
    m_zoomTimeRange = {start, end};
  }

  void jumpToCutIn();

  void setNeedsJumpToCutIn(bool needs) { m_needsJumpToCutIn = needs; }

  void performDelayedJumpIfNeeded();

  void paint(juce::Graphics &g) override;

  void resized() override;

  void updatePlayButtonText(bool isPlaying);

  void updateCutLabels();

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

  void triggerQualityButton();

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

  juce::TextButton& getAutoCutInButton() { return autoCutInButton; }
  juce::TextButton& getAutoCutOutButton() { return autoCutOutButton; }

  void updateStatsFromAudio();

  AppEnums::PlacementMode getPlacementMode() const;

  bool shouldAutoplay() const { return m_shouldAutoplay; }

  bool isCutModeActive() const { return m_isCutModeActive; }

  juce::Rectangle<int> getWaveformBounds() const {
    return layoutCache.waveformBounds;
  }

  AudioPlayer &getAudioPlayer();
  AudioPlayer &getAudioPlayer() const;
  SessionState &getSessionState() { return sessionState; }
  const SessionState &getSessionState() const { return sessionState; }

  AppEnums::ThumbnailQuality getCurrentQualitySetting() const {
    return currentQuality;
  }

  AppEnums::ChannelViewMode getChannelViewMode() const {
    return currentChannelViewMode;
  }

  float getGlowAlpha() const { return m_currentPulseAlpha; }

  const MouseHandler &getMouseHandler() const;
  MouseHandler &getMouseHandler();

  SilenceDetector &getSilenceDetector() { return *silenceDetector; }
  const SilenceDetector &getSilenceDetector() const { return *silenceDetector; }

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

  /** @brief Manages high-frequency updates. */
  std::unique_ptr<PlaybackTimerManager> playbackTimerManager;

  juce::TextButton openButton, playStopButton, modeButton, exitButton,
      statsButton, repeatButton, channelViewButton,
      qualityButton;
  juce::TextButton resetInButton, resetOutButton;
  juce::TextEditor cutInEditor, cutOutEditor;
  juce::TextEditor elapsedTimeEditor, remainingTimeEditor, cutLengthEditor;
  RepeatButton cutInButton, cutOutButton;
  juce::TextButton autoplayButton, autoCutInButton, autoCutOutButton, cutButton;

  ControlPanelLayoutCache layoutCache;

  AppEnums::ViewMode currentMode = AppEnums::ViewMode::Classic;
  AppEnums::ChannelViewMode currentChannelViewMode = AppEnums::ChannelViewMode::Mono;
  AppEnums::ThumbnailQuality currentQuality = AppEnums::ThumbnailQuality::Low;

  bool shouldRepeat = false;

  juce::String cutInDisplayString, cutOutDisplayString;
  int cutInTextX = 0, cutOutTextX = 0, cutTextY = 0;

  bool m_shouldAutoplay = false;
  bool m_isCutModeActive = false;
  AppEnums::ActiveZoomPoint m_activeZoomPoint = AppEnums::ActiveZoomPoint::None;
  float m_zoomFactor = 10.0f;
  float m_currentPulseAlpha = 0.0f;
  bool m_needsJumpToCutIn = false;
  juce::Rectangle<int> m_zoomPopupBounds;
  std::pair<double, double> m_zoomTimeRange;

  void initialiseLookAndFeel();

  void initialiseCutEditors();

  void invokeOwnerOpenDialog();

  void finaliseSetup();

  void updateQualityButtonText();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlPanel)
};

#endif 
