#ifndef AUDIOFILER_CONTROLPANEL_H
#define AUDIOFILER_CONTROLPANEL_H

/**
 * @class FocusManager
 * @brief Home: Engine.
 *
 */
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

/**
 * @class MainComponent
 * @brief Home: View.
 *
 */
class MainComponent; 
/**
 * @class LayoutManager
 * @brief Home: Engine.
 *
 */
class LayoutManager;
/**
 * @class WaveformView
 * @brief Home: View.
 *
 */
class WaveformView;
/**
 * @class CutLayerView
 * @brief Home: View.
 *
 */
class CutLayerView;
/**
 * @class CutPresenter
 * @brief Home: Presenter.
 *
 * @see CutLayerView
 */
class CutPresenter;
/**
 * @class StatsPresenter
 * @brief Home: Presenter.
 *
 * @see ControlPanel
 */
class StatsPresenter;
/**
 * @class RepeatPresenter
 * @brief Home: Presenter.
 *
 * @see PlaybackRepeatController
 */
class RepeatPresenter;
/**
 * @class ControlStatePresenter
 * @brief Home: Presenter.
 *
 * @see ControlPanel
 */
class ControlStatePresenter;
/**
 * @class TransportPresenter
 * @brief Home: Presenter.
 *
 * @see ControlPanel
 */
class TransportPresenter;
/**
 * @class SilenceDetectionPresenter
 * @brief Home: Presenter.
 *
 * @see SilenceDetector
 */
class SilenceDetectionPresenter;
/**
 * @class ControlButtonsPresenter
 * @brief Home: Presenter.
 *
 * @see ControlPanel
 */
class ControlButtonsPresenter;
/**
 * @class CutResetPresenter
 * @brief Home: Presenter.
 *
 * @see ControlPanel
 */
class CutResetPresenter;
/**
 * @class CutButtonPresenter
 * @brief Home: Presenter.
 *
 * @see RepeatButton
 */
class CutButtonPresenter;
/**
 * @class PlaybackTextPresenter
 * @brief Home: Presenter.
 *
 * @see ControlPanel
 */
class PlaybackTextPresenter;
/**
 * @class PlaybackCursorView
 * @brief Home: View.
 *
 */
class PlaybackCursorView;
/**
 * @class ZoomView
 * @brief Home: View.
 *
 */
class ZoomView;

/**
 * @file ControlPanel.h
 * @brief Strictly a Layout Manager for the application controls.
 * @ingroup Views
 */


/**
 * @class ControlPanel
 * @brief Strictly a Layout Manager for the application controls.
 */
class ControlPanel final : public juce::Component,
                           public juce::Timer {
public:
  
  /** @name Constructors and Destructors
   *  @{
   */

  /**
   * @brief Constructs the ControlPanel.
   */
  explicit ControlPanel(MainComponent &owner, SessionState &sessionStateIn);

  /**
   * @brief Destructor.
   */
  ~ControlPanel() override;

  

  
  /** @name Zoom Popup State
   *  @{
   */
  enum class ActiveZoomPoint { None, In, Out };
  /**
   * @brief Gets the ActiveZoomPoint.
   * @return ActiveZoomPoint
   */
  ActiveZoomPoint getActiveZoomPoint() const { return m_activeZoomPoint; }
  /**
   * @brief Sets the ActiveZoomPoint.
   * @param point [in] Description for point.
   */
  void setActiveZoomPoint(ActiveZoomPoint point);

  /**
   * @brief Gets the ZoomFactor.
   * @return float
   */
  float getZoomFactor() const { return m_zoomFactor; }
  /**
   * @brief Sets the ZoomFactor.
   * @param factor [in] Description for factor.
   */
  void setZoomFactor(float factor) {
    m_zoomFactor = juce::jlimit(1.0f, 1000000.0f, factor);
    /**
     * @brief Undocumented method.
     */
    repaint();
  }

  /**
   * @brief Checks if ZKeyDown.
   * @return bool
   */
  bool isZKeyDown() const { return m_isZKeyDown; }
  /**
   * @brief Sets the ZKeyDown.
   * @param isDown [in] Description for isDown.
   */
  void setZKeyDown(bool isDown);

  /**
   * @brief Gets the ZoomPopupBounds.
   * @return juce::Rectangle<int>
   */
  juce::Rectangle<int> getZoomPopupBounds() const { return m_zoomPopupBounds; }
  /**
   * @brief Sets the ZoomPopupBounds.
   * @param bounds [in] Description for bounds.
   */
  void setZoomPopupBounds(juce::Rectangle<int> bounds) {
    m_zoomPopupBounds = bounds;
  }

  std::pair<double, double> getZoomTimeRange() const { return m_zoomTimeRange; }
  /**
   * @brief Sets the ZoomTimeRange.
   * @param start [in] Description for start.
   * @param end [in] Description for end.
   */
  void setZoomTimeRange(double start, double end) {
    m_zoomTimeRange = {start, end};
  }

  /**
   * @brief Undocumented method.
   */
  void jumpToCutIn();
  /**
   * @brief Sets the NeedsJumpToCutIn.
   * @param needs [in] Description for needs.
   */
  void setNeedsJumpToCutIn(bool needs) { m_needsJumpToCutIn = needs; }
  /**
   * @brief Undocumented method.
   */
  void performDelayedJumpIfNeeded();
  

  
  /** @name juce::Component Overrides
   *  @{
   */

  /**
   * @brief Renders the component.
   * @param g [in] Description for g.
   */
  void paint(juce::Graphics &g) override;
  /**
   * @brief Called when the component is resized.
   */
  void resized() override;

  
  void updateCursorPosition();

  

  
  /** @name State Update Methods
   *  @{
   */

  /**
   * @brief Undocumented method.
   * @param isPlaying [in] Description for isPlaying.
   */
  void updatePlayButtonText(bool isPlaying);
  /**
   * @brief Undocumented method.
   */
  void updateCutLabels();
  /**
   * @brief Undocumented method.
   */
  void updateComponentStates();
  /**
   * @brief Undocumented method.
   */
  void updateUIFromState();
  /**
   * @brief Sets the AutoCutInActive.
   * @param isActive [in] Description for isActive.
   */
  void setAutoCutInActive(bool isActive);
  /**
   * @brief Sets the AutoCutOutActive.
   * @param isActive [in] Description for isActive.
   */
  void setAutoCutOutActive(bool isActive);

  /**
   * @brief Updates the colors of the boundary buttons based on interaction.
   */
  void updateCutButtonColors();

  

  
  /** @name Playback & Repeating State Accessors and Mutators
   *  @{
   */

  
  bool getShouldRepeat() const { return shouldRepeat; }

  
  void setShouldRepeat(bool shouldRepeatParam);

  
  double getCutInPosition() const;

  
  double getCutOutPosition() const;

  
  void setCutInPosition(double pos);

  
  void setCutOutPosition(double pos);

  
  void ensureCutOrder();

  

  
  /** @name UI Action Triggers
   *  @{
   */

  /**
   * @brief Undocumented method.
   */
  void toggleStats();
  /**
   * @brief Undocumented method.
   */
  void triggerQualityButton();
  /**
   * @brief Undocumented method.
   */
  void triggerModeButton();
  /**
   * @brief Undocumented method.
   */
  void triggerChannelViewButton();

  
  void triggerRepeatButton();

  /**
   * @brief Undocumented method.
   */
  void resetIn();
  /**
   * @brief Undocumented method.
   */
  void resetOut();

  

  
  /** @name Stats Display Control
   *  @{
   */

  /**
   * @brief Sets the ShouldShowStats.
   * @param shouldShowStats [in] Description for shouldShowStats.
   */
  void setShouldShowStats(bool shouldShowStats);
  /**
   * @brief Sets the TotalTimeStaticString.
   * @param timeString [in] Description for timeString.
   */
  void setTotalTimeStaticString(const juce::String &timeString);
  void setStatsDisplayText(const juce::String &text,
                           juce::Colour color = Config::Colors::statsText);
  void logStatusMessage(const juce::String &message,
                        bool isError = false);
  /**
   * @brief Undocumented method.
   */
  void updateStatsFromAudio();

  

  
  /** @name Placement Mode & Autoplay Status
   *  @{
   */

  /**
   * @brief Gets the PlacementMode.
   * @return AppEnums::PlacementMode
   */
  AppEnums::PlacementMode getPlacementMode() const;
  /**
   * @brief Undocumented method.
   * @return bool
   */
  bool shouldAutoplay() const { return m_shouldAutoplay; }
  /**
   * @brief Checks if CutModeActive.
   * @return bool
   */
  bool isCutModeActive() const { return m_isCutModeActive; }

  

  
  /** @name Component Accessors
   *  @{
   */

  /**
   * @brief Gets the WaveformBounds.
   * @return juce::Rectangle<int>
   */
  juce::Rectangle<int> getWaveformBounds() const {
    return layoutCache.waveformBounds;
  }

  AudioPlayer &getAudioPlayer();
  AudioPlayer &getAudioPlayer() const;
  SessionState &getSessionState() { return sessionState; }
  const SessionState &getSessionState() const { return sessionState; }

  /**
   * @brief Gets the CurrentQualitySetting.
   * @return AppEnums::ThumbnailQuality
   */
  AppEnums::ThumbnailQuality getCurrentQualitySetting() const {
    return currentQuality;
  }

  /**
   * @brief Gets the ChannelViewMode.
   * @return AppEnums::ChannelViewMode
   */
  AppEnums::ChannelViewMode getChannelViewMode() const {
    return currentChannelViewMode;
  }

  /**
   * @brief Gets the GlowAlpha.
   * @return float
   */
  float getGlowAlpha() const { return glowAlpha; }

  const MouseHandler &getMouseHandler() const;
  MouseHandler &getMouseHandler();

  SilenceDetector &getSilenceDetector() { return *silenceDetector; }
  const SilenceDetector &getSilenceDetector() const { return *silenceDetector; }

  /**
   * @brief Gets the SilenceDetectionPresenter.
   * @return SilenceDetectionPresenter*
   */
  SilenceDetectionPresenter* getSilenceDetectionPresenter() { return silenceDetectionPresenter.get(); }

  /**
   * @brief Gets the BottomRowTopY.
   * @return int
   */
  int getBottomRowTopY() const { return layoutCache.bottomRowTopY; }

  std::tuple<int, int, int> getPlaybackLabelXs() const {
    return {layoutCache.playbackLeftTextX, layoutCache.playbackCenterTextX,
            layoutCache.playbackRightTextX};
  }

  juce::TextEditor &getStatsDisplay();

  /**
   * @brief Sets the CutStart.
   * @param sampleIndex [in] Description for sampleIndex.
   */
  void setCutStart(int sampleIndex);
  /**
   * @brief Sets the CutEnd.
   * @param sampleIndex [in] Description for sampleIndex.
   */
  void setCutEnd(int sampleIndex);

  /**
   * @brief Undocumented method.
   * @param seconds [in] Description for seconds.
   * @return juce::String
   */
  juce::String formatTime(double seconds) const;
  const juce::LookAndFeel &getLookAndFeel() const;
  FocusManager &getFocusManager() const { return *focusManager; }

  

  
  /** @name juce::MouseListener Overrides
   *  @{
   */

  /**
   * @brief Undocumented method.
   * @param event [in] Description for event.
   */
  void mouseMove(const juce::MouseEvent &event) override;
  /**
   * @brief Undocumented method.
   * @param event [in] Description for event.
   */
  void mouseDown(const juce::MouseEvent &event) override;
  /**
   * @brief Undocumented method.
   * @param event [in] Description for event.
   */
  void mouseDrag(const juce::MouseEvent &event) override;
  /**
   * @brief Undocumented method.
   * @param event [in] Description for event.
   */
  void mouseUp(const juce::MouseEvent &event) override;
  /**
   * @brief Undocumented method.
   * @param event [in] Description for event.
   */
  void mouseExit(const juce::MouseEvent &event) override;
  void mouseWheelMove(const juce::MouseEvent &event,
                      const juce::MouseWheelDetails &wheel) override;
  /**
   * @brief Undocumented method.
   */
  void timerCallback() override;

  

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
  SessionState &sessionState;
  ModernLookAndFeel modernLF;
  std::unique_ptr<SilenceDetector> silenceDetector;
  std::unique_ptr<CutPresenter> cutPresenter;
  std::unique_ptr<LayoutManager> layoutManager;
  std::unique_ptr<WaveformView> waveformView;
  std::unique_ptr<CutLayerView> cutLayerView;
  std::unique_ptr<PlaybackTextPresenter> playbackTextPresenter;
  std::unique_ptr<StatsPresenter> statsPresenter;
  std::unique_ptr<RepeatPresenter> repeatPresenter;
  std::unique_ptr<ControlStatePresenter> controlStatePresenter;
  std::unique_ptr<TransportPresenter> transportPresenter;
  std::unique_ptr<SilenceDetectionPresenter> silenceDetectionPresenter;
  std::unique_ptr<ControlButtonsPresenter> buttonPresenter;
  std::unique_ptr<CutButtonPresenter> cutButtonPresenter;
  std::unique_ptr<CutResetPresenter> cutResetPresenter;
  std::unique_ptr<FocusManager> focusManager;
  std::unique_ptr<PlaybackCursorView> playbackCursorView;
  std::unique_ptr<ZoomView> zoomView;

  int lastCursorX{-1};

  
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
  float glowAlpha = 0.0f;
  bool m_isCutModeActive = false;
  ActiveZoomPoint m_activeZoomPoint = ActiveZoomPoint::None;
  float m_zoomFactor = 10.0f;
  bool m_isZKeyDown = false;
  bool m_needsJumpToCutIn = false;
  juce::Rectangle<int> m_zoomPopupBounds;
  std::pair<double, double> m_zoomTimeRange;

  /**
   * @brief Undocumented method.
   */
  void initialiseLookAndFeel();
  /**
   * @brief Undocumented method.
   */
  void initialiseCutEditors();
  /**
   * @brief Undocumented method.
   */
  void invokeOwnerOpenDialog();
  /**
   * @brief Undocumented method.
   */
  void finaliseSetup();
  /**
   * @brief Undocumented method.
   */
  void updateQualityButtonText();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlPanel)
};

#endif 
