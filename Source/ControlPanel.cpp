#include "ControlPanel.h"
#include "AudioPlayer.h"
#include "Config.h"
#include "ControlButtonsPresenter.h"
#include "ControlPanelCopy.h"
#include "ControlStatePresenter.h"
#include "FocusManager.h"
#include "LayoutManager.h"
#include "CutButtonPresenter.h"
#include "RepeatPresenter.h"
#include "CutResetPresenter.h"
#include "MainComponent.h"
#include "PlaybackCursorView.h"
#include "PlaybackCursorGlow.h"
#include "PlaybackTextPresenter.h"
#include "SilenceDetectionPresenter.h"
#include "StatsPresenter.h"
#include "TimeUtils.h"
#include "TransportPresenter.h"
#include "WaveformView.h"
#include "CutLayerView.h"
#include "CutPresenter.h"
#include "CoordinateMapper.h"
#include "ZoomView.h"
#include <cmath>

ControlPanel::ControlPanel(MainComponent &ownerComponent, SessionState &sessionStateIn)
    : owner(ownerComponent),
      sessionState(sessionStateIn),
      modernLF(),
      silenceDetector(std::make_unique<SilenceDetector>(*this)),
      layoutManager(std::make_unique<LayoutManager>(*this)),
      focusManager(std::make_unique<FocusManager>(*this)) {
  initialiseLookAndFeel();

  waveformView = std::make_unique<WaveformView>(owner.getAudioPlayer()->getWaveformManager());
  addAndMakeVisible(waveformView.get());

  cutLayerView = std::make_unique<CutLayerView>(*this,
                                                sessionState,
                                                *silenceDetector,
                                                owner.getAudioPlayer()->getWaveformManager(),
                                                [this]() { return getGlowAlpha(); });
  cutPresenter = std::make_unique<CutPresenter>(*this, sessionState, *cutLayerView);
  cutLayerView->setMouseHandler(cutPresenter->getMouseHandler());
  addAndMakeVisible(cutLayerView.get());

  playbackCursorView = std::make_unique<PlaybackCursorView>(*this);
  addAndMakeVisible(playbackCursorView.get());
  playbackCursorView->setInterceptsMouseClicks(false, false);

  zoomView = std::make_unique<ZoomView>(*this);
  addAndMakeVisible(zoomView.get());
  zoomView->setVisible(true);

  statsPresenter = std::make_unique<StatsPresenter>(*this);
  silenceDetectionPresenter = std::make_unique<SilenceDetectionPresenter>(*this, sessionState, *owner.getAudioPlayer());
  owner.getAudioPlayer()->setControlPanel(this);
  playbackTextPresenter = std::make_unique<PlaybackTextPresenter>(*this);

  buttonPresenter = std::make_unique<ControlButtonsPresenter>(*this);
  buttonPresenter->initialiseAllButtons();

  cutButtonPresenter = std::make_unique<CutButtonPresenter>(*this);
  repeatPresenter = std::make_unique<RepeatPresenter>(*this, *silenceDetector,
                                                  cutInEditor, cutOutEditor);
  repeatPresenter->initialiseEditors();

  initialiseCutEditors();

  controlStatePresenter = std::make_unique<ControlStatePresenter>(*this);
  transportPresenter = std::make_unique<TransportPresenter>(*this);
  updateUIFromState();
  finaliseSetup();

  startTimerHz(60);
  setMouseCursor(juce::MouseCursor::CrosshairCursor);
}

ControlPanel::~ControlPanel() {
  stopTimer();
  setLookAndFeel(nullptr);
}

void ControlPanel::initialiseLookAndFeel() {
  setLookAndFeel(&modernLF);
  modernLF.setBaseOffColor(Config::Colors::Button::base);
  modernLF.setBaseOnColor(Config::Colors::Button::on);
  modernLF.setTextColor(Config::Colors::Button::text);
}

void ControlPanel::initialiseCutEditors() {
  cutResetPresenter = std::make_unique<CutResetPresenter>(*this);
  addAndMakeVisible(silenceDetector->getInSilenceThresholdEditor());
  addAndMakeVisible(silenceDetector->getOutSilenceThresholdEditor());
}

void ControlPanel::invokeOwnerOpenDialog() { owner.openButtonClicked(); }

void ControlPanel::finaliseSetup() {
  if (playbackTextPresenter != nullptr)
    playbackTextPresenter->initialiseEditors();
  updateCutLabels();
  updateComponentStates();
}

void ControlPanel::resized() {
  if (layoutManager != nullptr)
    layoutManager->performLayout();

  if (playbackTextPresenter != nullptr)
    playbackTextPresenter->layoutEditors();

  if (waveformView != nullptr)
    waveformView->setBounds(layoutCache.waveformBounds);

  if (cutLayerView != nullptr)
    cutLayerView->setBounds(layoutCache.waveformBounds);

  if (playbackCursorView != nullptr)
    playbackCursorView->setBounds(layoutCache.waveformBounds);

  if (zoomView != nullptr)
    zoomView->setBounds(layoutCache.waveformBounds);
}

void ControlPanel::paint(juce::Graphics &g) {
  g.fillAll(Config::Colors::Window::background);
  if (playbackTextPresenter != nullptr)
    playbackTextPresenter->render(g);
}

void ControlPanel::updatePlayButtonText(bool isPlaying) {
  playStopButton.setButtonText(isPlaying ? ControlPanelCopy::stopButtonText()
                                         : ControlPanelCopy::playButtonText());
}

void ControlPanel::setZKeyDown(bool isDown) {
  if (m_isZKeyDown == isDown)
    return;

  m_isZKeyDown = isDown;

  if (m_isZKeyDown) {
    auto dragged = getMouseHandler().getDraggedHandle();
    if (dragged == MouseHandler::CutMarkerHandle::In)
      m_activeZoomPoint = ActiveZoomPoint::In;
    else if (dragged == MouseHandler::CutMarkerHandle::Out)
      m_activeZoomPoint = ActiveZoomPoint::Out;
  } else {
    m_activeZoomPoint = ActiveZoomPoint::None;
    performDelayedJumpIfNeeded();
  }

  if (zoomView != nullptr) {
    zoomView->repaint();
  }
  repaint();
}

void ControlPanel::setActiveZoomPoint(ActiveZoomPoint point) {
  if (m_activeZoomPoint != point) {
    m_activeZoomPoint = point;
    if (zoomView != nullptr) {
      zoomView->repaint();
    }
    repaint();
  }
}

void ControlPanel::jumpToCutIn() {
  getAudioPlayer().setPlayheadPosition(getCutInPosition());
  m_needsJumpToCutIn = false;
}

void ControlPanel::performDelayedJumpIfNeeded() {
  if (m_needsJumpToCutIn)
    jumpToCutIn();
}

double ControlPanel::getCutInPosition() const {
  return sessionState.getCutIn();
}

double ControlPanel::getCutOutPosition() const {
  return sessionState.getCutOut();
}

void ControlPanel::setCutInPosition(double pos) {
  sessionState.setCutIn(pos);
  if (repeatPresenter != nullptr)
    repeatPresenter->updateCutLabels();
}

void ControlPanel::setCutOutPosition(double pos) {
  sessionState.setCutOut(pos);
  if (repeatPresenter != nullptr)
    repeatPresenter->updateCutLabels();
}

void ControlPanel::updateCutLabels() {
  if (repeatPresenter != nullptr)
    repeatPresenter->updateCutLabels();

  if (playbackTextPresenter != nullptr)
    playbackTextPresenter->updateEditors();
}

void ControlPanel::updateComponentStates() {
  if (controlStatePresenter != nullptr)
    controlStatePresenter->refreshStates();
}

void ControlPanel::updateUIFromState() {
  const auto &autoCut = sessionState.getCutPrefs().autoCut;
  autoCutInButton.setToggleState(autoCut.inActive, juce::dontSendNotification);
  autoCutOutButton.setToggleState(autoCut.outActive, juce::dontSendNotification);
  silenceDetector->setIsAutoCutInActive(autoCut.inActive);
  silenceDetector->setIsAutoCutOutActive(autoCut.outActive);

  const int inPercent = static_cast<int>(autoCut.thresholdIn * 100.0f);
  const int outPercent = static_cast<int>(autoCut.thresholdOut * 100.0f);
  silenceDetector->getInSilenceThresholdEditor()
      .setText(juce::String(inPercent), juce::dontSendNotification);
  silenceDetector->getOutSilenceThresholdEditor()
      .setText(juce::String(outPercent), juce::dontSendNotification);

  updateComponentStates();
  updateCutLabels();
  if (zoomView != nullptr)
    zoomView->updateZoomState();
  repaint();
}

void ControlPanel::setAutoCutInActive(bool isActive) {
  sessionState.setAutoCutInActive(isActive);
  autoCutInButton.setToggleState(isActive, juce::dontSendNotification);
  silenceDetector->setIsAutoCutInActive(isActive);
  updateComponentStates();
  if (isActive && getAudioPlayer().getThumbnail().getTotalLength() > 0.0)
    silenceDetector->detectInSilence();
}

void ControlPanel::setAutoCutOutActive(bool isActive) {
  sessionState.setAutoCutOutActive(isActive);
  autoCutOutButton.setToggleState(isActive, juce::dontSendNotification);
  silenceDetector->setIsAutoCutOutActive(isActive);
  updateComponentStates();
  if (isActive && getAudioPlayer().getThumbnail().getTotalLength() > 0.0)
    silenceDetector->detectOutSilence();
}

void ControlPanel::updateQualityButtonText() {
  if (currentQuality == AppEnums::ThumbnailQuality::High)
    qualityButton.setButtonText(ControlPanelCopy::qualityHighText());
  else if (currentQuality == AppEnums::ThumbnailQuality::Medium)
    qualityButton.setButtonText(ControlPanelCopy::qualityMediumText());
  else
    qualityButton.setButtonText(ControlPanelCopy::qualityLowText());
}

void ControlPanel::toggleStats() {
  if (statsPresenter == nullptr)
    return;

  statsPresenter->toggleVisibility();
  statsButton.setToggleState(statsPresenter->isShowingStats(),
                             juce::dontSendNotification);
  updateComponentStates();
}

void ControlPanel::triggerQualityButton() { qualityButton.triggerClick(); }
void ControlPanel::triggerModeButton() { modeButton.triggerClick(); }
void ControlPanel::triggerChannelViewButton() { channelViewButton.triggerClick(); }
void ControlPanel::triggerRepeatButton() { repeatButton.triggerClick(); }

void ControlPanel::resetIn() { resetInButton.triggerClick(); }
void ControlPanel::resetOut() { resetOutButton.triggerClick(); }

void ControlPanel::setStatsDisplayText(const juce::String &text,
                                       juce::Colour color) {
  if (statsPresenter != nullptr)
    statsPresenter->setDisplayText(text, color);
}

void ControlPanel::logStatusMessage(const juce::String &message, bool isError) {
  const auto color = isError ? Config::Colors::statsErrorText : Config::Colors::statsText;
  setStatsDisplayText(message, color);
}

void ControlPanel::updateStatsFromAudio() {
  if (statsPresenter != nullptr)
    statsPresenter->updateStats();
}

void ControlPanel::ensureCutOrder() {
  if (repeatPresenter != nullptr)
    repeatPresenter->ensureCutOrder();
}

void ControlPanel::setShouldShowStats(bool shouldShowStatsParam) {
  if (statsPresenter != nullptr)
    statsPresenter->setShouldShowStats(shouldShowStatsParam);
}

void ControlPanel::setTotalTimeStaticString(const juce::String &timeString) {
  if (playbackTextPresenter != nullptr)
    playbackTextPresenter->setTotalTimeStaticString(timeString);
}

void ControlPanel::setShouldRepeat(bool shouldRepeatParam) {
  shouldRepeat = shouldRepeatParam;
}

void ControlPanel::updateCutButtonColors() {
  if (cutButtonPresenter != nullptr)
    cutButtonPresenter->updateColours();
}

AudioPlayer &ControlPanel::getAudioPlayer() { return *owner.getAudioPlayer(); }

AudioPlayer &ControlPanel::getAudioPlayer() const {
  return *owner.getAudioPlayer();
}

const MouseHandler &ControlPanel::getMouseHandler() const {
  return cutPresenter->getMouseHandler();
}

MouseHandler &ControlPanel::getMouseHandler() {
  return cutPresenter->getMouseHandler();
}

juce::TextEditor &ControlPanel::getStatsDisplay() {
  jassert(statsPresenter != nullptr);
  return statsPresenter->getDisplay();
}

void ControlPanel::setCutStart(int sampleIndex) {
  if (silenceDetectionPresenter != nullptr)
    silenceDetectionPresenter->setCutStart(sampleIndex);
}

void ControlPanel::setCutEnd(int sampleIndex) {
  if (silenceDetectionPresenter != nullptr)
    silenceDetectionPresenter->setCutEnd(sampleIndex);
}

juce::String ControlPanel::formatTime(double seconds) const {
  return TimeUtils::formatTime(seconds);
}

const juce::LookAndFeel &ControlPanel::getLookAndFeel() const {
  return modernLF;
}

AppEnums::PlacementMode ControlPanel::getPlacementMode() const {
  return getMouseHandler().getCurrentPlacementMode();
}

void ControlPanel::mouseMove(const juce::MouseEvent &event) {
  getMouseHandler().mouseMove(event);
}

void ControlPanel::mouseDown(const juce::MouseEvent &event) {
  getMouseHandler().mouseDown(event);
}

void ControlPanel::mouseDrag(const juce::MouseEvent &event) {
  getMouseHandler().mouseDrag(event);
}

void ControlPanel::mouseUp(const juce::MouseEvent &event) {
  getMouseHandler().mouseUp(event);
}

void ControlPanel::mouseExit(const juce::MouseEvent &event) {
  getMouseHandler().mouseExit(event);
}

void ControlPanel::mouseWheelMove(const juce::MouseEvent &event,
                                  const juce::MouseWheelDetails &wheel) {
  getMouseHandler().mouseWheelMove(event, wheel);
}

void ControlPanel::updateCursorPosition() {
  if (playbackCursorView != nullptr)
  {
    const auto& audioPlayer = getAudioPlayer();
    const double audioLength = audioPlayer.getWaveformManager().getThumbnail().getTotalLength();
    if (audioLength > 0.0)
    {
      const float x = CoordinateMapper::secondsToPixels(audioPlayer.getCurrentPosition(), 
                                                        (float)layoutCache.waveformBounds.getWidth(), 
                                                        audioLength);
      const int currentX = juce::roundToInt(x);

      if (currentX != lastCursorX)
      {
          // Repaint old area
          if (lastCursorX >= 0)
              playbackCursorView->repaint(lastCursorX - 1, 0, 3, playbackCursorView->getHeight());
          
          // Repaint new area
          playbackCursorView->repaint(currentX - 1, 0, 3, playbackCursorView->getHeight());
          
          lastCursorX = currentX;
      }

      const bool zDown = isZKeyDown();
      const auto activePoint = getActiveZoomPoint();
      const bool isZooming = zDown || activePoint != ControlPanel::ActiveZoomPoint::None;
      
      // If zooming, we might want to hide the cursor if it intersects the popup area
      // popupBounds is in ControlPanel coords, PlaybackCursorView is at waveformBounds
      if (isZooming && m_zoomPopupBounds.translated(-layoutCache.waveformBounds.getX(), -layoutCache.waveformBounds.getY()).contains(currentX, 10))
          playbackCursorView->setVisible(false);
      else
          playbackCursorView->setVisible(true);
    }
  }
}

void ControlPanel::timerCallback() {
  const bool isZDown = juce::KeyPress::isKeyCurrentlyDown('z') || juce::KeyPress::isKeyCurrentlyDown('Z');
  setZKeyDown(isZDown);

  updateCutLabels();
  updateCursorPosition();
  if (zoomView != nullptr)
    zoomView->updateZoomState();
}
