#ifndef AUDIOFILER_CONTROLPANEL_H
#define AUDIOFILER_CONTROLPANEL_H

class FocusManager;
#include "AppEnums.h"
#include "AudioPlayer.h" // Added for AudioPlayer type recognition
#include "Config.h"
#include "ControlPanelLayoutCache.h"
#include "RepeatButton.h"
#include "ModernLookAndFeel.h" // Added include for ModernLookAndFeel
#include "MouseHandler.h"      // Include the new MouseHandler class
#include "SessionState.h"
#include "SilenceDetector.h"   // Include the new SilenceDetector class
#include "SilenceWorkerClient.h"

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
    #include <juce_gui_basics/juce_gui_basics.h>
    #include <juce_events/juce_events.h>
#else
    #include <JuceHeader.h>
#endif

#include <memory> // Required for std::unique_ptr
#include <tuple>

class MainComponent; // Forward declaration
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

/**
 * @file ControlPanel.h
 * @brief Defines the ControlPanel class, which manages the application's UI
 * controls.
 */

/**
 * @class ControlPanel
 * @brief A component that manages all UI controls and interactions for the
 * audiofiler application.
 */
class ControlPanel final : public juce::Component,
                           public juce::Timer {
public:
  //==============================================================================
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

  /** @} */

  //==============================================================================
  /** @name Zoom Popup State
   *  @{
   */
  enum class ActiveZoomPoint { None, In, Out };
  ActiveZoomPoint getActiveZoomPoint() const { return m_activeZoomPoint; }
  void setActiveZoomPoint(ActiveZoomPoint point) {
    m_activeZoomPoint = point;
    repaint();
  }

  float getZoomFactor() const { return m_zoomFactor; }
  void setZoomFactor(float factor) {
    m_zoomFactor = juce::jlimit(1.0f, 1000000.0f, factor);
    repaint();
  }

  bool isZKeyDown() const { return m_isZKeyDown; }
  void setZKeyDown(bool isDown);

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
  /** @} */

  //==============================================================================
  /** @name juce::Component Overrides
   *  @{
   */

  void paint(juce::Graphics &g) override;
  void resized() override;

  /** @brief Renders the dynamic overlays (cursors, etc.). Called by
   * PlaybackCursorView. */
  void renderOverlays(juce::Graphics &g);
  void drawMouseCursorOverlays(juce::Graphics &g);
  void drawZoomPopup(juce::Graphics &g);
  /** @brief Updates only the cursor/overlay part of the UI. */
  void updateCursorPosition();

  /** @} */

  //==============================================================================
  /** @name State Update Methods
   *  @{
   */

  void updatePlayButtonText(bool isPlaying);
  void updateCutLabels();
  void updateComponentStates();
  void updateUIFromState();
  void setAutoCutInActive(bool isActive);
  void setAutoCutOutActive(bool isActive);

  /**
   * @brief Updates the colors of the boundary buttons based on interaction.
   */
  void updateCutButtonColors();

  /** @} */

  //==============================================================================
  /** @name Playback & Repeating State Accessors and Mutators
   *  @{
   */

  /** @brief Checks if repeating is enabled. */
  bool getShouldRepeat() const { return shouldRepeat; }

  /** @brief Sets whether audio playback should repeat. */
  void setShouldRepeat(bool shouldRepeatParam);

  /** @brief Gets the current cut-in position. */
  double getCutInPosition() const;

  /** @brief Gets the current cut-out position. */
  double getCutOutPosition() const;

  /** @brief Sets the cut-in position. */
  void setCutInPosition(double pos);

  /** @brief Sets the cut-out position. */
  void setCutOutPosition(double pos);

  /** @brief Ensures that cut-in is logically before or at cut-out. */
  void ensureCutOrder();

  /** @} */

  //==============================================================================
  /** @name UI Action Triggers
   *  @{
   */

  void toggleStats();
  void triggerQualityButton();
  void triggerModeButton();
  void triggerChannelViewButton();

  /** @brief Triggers the main repeat toggle button. */
  void triggerRepeatButton();

  void resetIn();
  void resetOut();

  /** @} */

  //==============================================================================
  /** @name Stats Display Control
   *  @{
   */

  void setShouldShowStats(bool shouldShowStats);
  void setTotalTimeStaticString(const juce::String &timeString);
  void setStatsDisplayText(const juce::String &text,
                           juce::Colour color = Config::Colors::statsText);
  void logStatusMessage(const juce::String &message,
                        bool isError = false);
  void updateStatsFromAudio();

  /** @} */

  //==============================================================================
  /** @name Placement Mode & Autoplay Status
   *  @{
   */

  AppEnums::PlacementMode getPlacementMode() const;
  bool shouldAutoplay() const { return m_shouldAutoplay; }
  bool isCutModeActive() const { return m_isCutModeActive; }

  /** @} */

  //==============================================================================
  /** @name Component Accessors
   *  @{
   */

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

  float getGlowAlpha() const { return glowAlpha; }

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

  /** @} */

  //==============================================================================
  /** @name juce::MouseListener Overrides
   *  @{
   */

  void mouseMove(const juce::MouseEvent &event) override;
  void mouseDown(const juce::MouseEvent &event) override;
  void mouseDrag(const juce::MouseEvent &event) override;
  void mouseUp(const juce::MouseEvent &event) override;
  void mouseExit(const juce::MouseEvent &event) override;
  void mouseWheelMove(const juce::MouseEvent &event,
                      const juce::MouseWheelDetails &wheel) override;
  void timerCallback() override;

  /** @} */

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

  // --- UI Components ---
  juce::TextButton openButton, playStopButton, modeButton, exitButton,
      statsButton, repeatButton, channelViewButton,
      qualityButton;
  juce::TextButton resetInButton, resetOutButton;
  juce::TextEditor cutInEditor, cutOutEditor;
  juce::TextEditor elapsedTimeEditor, remainingTimeEditor, cutLengthEditor;
  RepeatButton cutInButton, cutOutButton;
  juce::TextButton autoplayButton, autoCutInButton, autoCutOutButton, cutButton;

  // --- Layout ---
  ControlPanelLayoutCache layoutCache;

  // --- State ---
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

  void initialiseLookAndFeel();
  void initialiseCutEditors();
  void invokeOwnerOpenDialog();
  void finaliseSetup();
  void updateQualityButtonText();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlPanel)
};

#endif // AUDIOFILER_CONTROLPANEL_H
