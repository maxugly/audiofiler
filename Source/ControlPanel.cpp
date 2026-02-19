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
  waveformView->setQuality(currentQuality);
  waveformView->setChannelMode(currentChannelViewMode);
  addAndMakeVisible(waveformView.get());

  cutLayerView = std::make_unique<CutLayerView>(sessionState,
                                                *silenceDetector,
                                                owner.getAudioPlayer()->getWaveformManager(),
                                                [this]() { return getGlowAlpha(); });
  cutPresenter = std::make_unique<CutPresenter>(*this, sessionState, *cutLayerView);
  cutLayerView->setMouseHandler(cutPresenter->getMouseHandler());
  addAndMakeVisible(cutLayerView.get());

  playbackCursorView = std::make_unique<PlaybackCursorView>(*this);
  addAndMakeVisible(playbackCursorView.get());
  playbackCursorView->setInterceptsMouseClicks(false, false);

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
  repaint();
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

void ControlPanel::renderOverlays(juce::Graphics &g) {
  drawMouseCursorOverlays(g);
  drawZoomPopup(g);
}

void ControlPanel::drawMouseCursorOverlays(juce::Graphics& g) {
  const auto waveformBounds = getWaveformBounds();
  const auto& mouse = getMouseHandler();
  if (mouse.getMouseCursorX() == -1)
    return;

  juce::Colour currentLineColor;
  juce::Colour currentHighlightColor;
  juce::Colour currentGlowColor;
  float currentGlowThickness = 0.0f;

  if (mouse.getCurrentPlacementMode() == AppEnums::PlacementMode::CutIn
      || mouse.getCurrentPlacementMode() == AppEnums::PlacementMode::CutOut)
  {
    currentLineColor = Config::Colors::mousePlacementMode;
    currentHighlightColor = Config::Colors::mousePlacementMode.withAlpha(0.4f);
    currentGlowColor = Config::Colors::placementModeGlow;
    currentGlowThickness = Config::Layout::Glow::placementModeGlowThickness;

    g.setColour(currentGlowColor.withAlpha(Config::Layout::Glow::mouseAlpha));
    g.fillRect(mouse.getMouseCursorX() - (int)(currentGlowThickness * Config::Layout::Glow::offsetFactor) - 1, waveformBounds.getY(),
               (int)currentGlowThickness + Config::Layout::Glow::mousePadding, waveformBounds.getHeight());
    g.fillRect(waveformBounds.getX(), mouse.getMouseCursorY() - (int)(currentGlowThickness * Config::Layout::Glow::offsetFactor) - 1,
               waveformBounds.getWidth(), (int)currentGlowThickness + Config::Layout::Glow::mousePadding);
  }
  else
  {
    if (isZKeyDown())
    {
      currentLineColor = Config::Colors::mousePlacementMode;
      currentHighlightColor = Config::Colors::mousePlacementMode.withAlpha(0.4f);
    }
    else
    {
      currentLineColor = Config::Colors::mouseCursorLine;
      currentHighlightColor = Config::Colors::mouseCursorHighlight;
    }
    currentGlowColor = Config::Colors::mouseAmplitudeGlow;
  }

  g.setColour(currentHighlightColor);
  g.fillRect(mouse.getMouseCursorX() - Config::Layout::Glow::mouseHighlightOffset, waveformBounds.getY(), Config::Layout::Glow::mouseHighlightSize, waveformBounds.getHeight());
  g.fillRect(waveformBounds.getX(), mouse.getMouseCursorY() - Config::Layout::Glow::mouseHighlightOffset, waveformBounds.getWidth(), Config::Layout::Glow::mouseHighlightSize);

  auto& audioPlayer = getAudioPlayer();
  const double audioLength = audioPlayer.getWaveformManager().getThumbnail().getTotalLength();
  if (audioLength > 0.0f)
  {
    float amplitude = 0.0f;
    if (audioPlayer.getWaveformManager().getThumbnail().getNumChannels() > 0)
    {
      double sampleRate = 0.0;
      juce::int64 length = 0;
      if (audioPlayer.getReaderInfo(sampleRate, length) && sampleRate > 0.0)
      {
        float minVal = 0.0f, maxVal = 0.0f;
        audioPlayer.getWaveformManager().getThumbnail().getApproximateMinMax(mouse.getMouseCursorTime(),
                                                                             mouse.getMouseCursorTime() + (1.0 / sampleRate),
                                                                             0, minVal, maxVal);
        amplitude = juce::jmax(std::abs(minVal), std::abs(maxVal));
      }
    }

    const float centerY = (float)waveformBounds.getCentreY();
    const float amplitudeY = centerY - (amplitude * waveformBounds.getHeight() * Config::Layout::Waveform::heightScale);
    const float bottomAmplitudeY = centerY + (amplitude * waveformBounds.getHeight() * Config::Layout::Waveform::heightScale);

    juce::ColourGradient amplitudeGlowGradient(currentGlowColor.withAlpha(0.0f), (float)mouse.getMouseCursorX(), amplitudeY,
                                               currentGlowColor.withAlpha(Config::Layout::Glow::mouseAmplitudeAlpha), (float)mouse.getMouseCursorX(), centerY, true);
    g.setGradientFill(amplitudeGlowGradient);
    g.fillRect(juce::Rectangle<float>((float)mouse.getMouseCursorX() - Config::Layout::Glow::mouseAmplitudeGlowThickness * Config::Layout::Glow::offsetFactor, amplitudeY,
                                      Config::Layout::Glow::mouseAmplitudeGlowThickness, centerY - amplitudeY));
    g.fillRect(juce::Rectangle<float>((float)mouse.getMouseCursorX() - Config::Layout::Glow::mouseAmplitudeGlowThickness * Config::Layout::Glow::offsetFactor, centerY,
                                      Config::Layout::Glow::mouseAmplitudeGlowThickness, centerY - amplitudeY));

    g.setColour(Config::Colors::mouseAmplitudeLine);
    g.drawVerticalLine(mouse.getMouseCursorX(), amplitudeY, bottomAmplitudeY);

    const float halfLineLength = Config::Animation::mouseAmplitudeLineLength * Config::Layout::Glow::offsetFactor;
    const float leftExtent = (float)mouse.getMouseCursorX() - halfLineLength;
    const float rightExtent = (float)mouse.getMouseCursorX() + halfLineLength;
    g.drawHorizontalLine(juce::roundToInt(amplitudeY), leftExtent, rightExtent);
    g.drawHorizontalLine(juce::roundToInt(bottomAmplitudeY), leftExtent, rightExtent);

    g.setColour(Config::Colors::playbackText);
    g.setFont(Config::Layout::Text::mouseCursorSize);
    g.drawText(juce::String(amplitude, 2), mouse.getMouseCursorX() + Config::Layout::Glow::mouseTextOffset, (int)amplitudeY - Config::Layout::Text::mouseCursorSize,
               100, Config::Layout::Text::mouseCursorSize, juce::Justification::left, true);
    g.drawText(juce::String(-amplitude, 2), mouse.getMouseCursorX() + Config::Layout::Glow::mouseTextOffset, (int)bottomAmplitudeY,
               100, Config::Layout::Text::mouseCursorSize, juce::Justification::left, true);

    const juce::String timeText = formatTime(mouse.getMouseCursorTime());
    g.drawText(timeText, mouse.getMouseCursorX() + Config::Layout::Glow::mouseTextOffset, mouse.getMouseCursorY() + Config::Layout::Glow::mouseTextOffset, 100,
               Config::Layout::Text::mouseCursorSize, juce::Justification::left, true);
  }

  PlaybackCursorGlow::renderGlow(g, mouse.getMouseCursorX(), waveformBounds.getY(), waveformBounds.getBottom(), currentLineColor);
  g.setColour(currentLineColor);
  g.drawHorizontalLine(mouse.getMouseCursorY(), (float)waveformBounds.getX(), (float)waveformBounds.getRight());
}

void ControlPanel::drawZoomPopup(juce::Graphics& g)
{
  const bool zDown = isZKeyDown();
  const auto activePoint = getActiveZoomPoint();

  if (!zDown && activePoint == ControlPanel::ActiveZoomPoint::None)
    return;

  auto& audioPlayer = getAudioPlayer();
  const double audioLength = audioPlayer.getWaveformManager().getThumbnail().getTotalLength();
  if (audioLength <= 0.0)
    return;

  const auto waveformBounds = getWaveformBounds();

  const int popupWidth = juce::roundToInt((float)waveformBounds.getWidth() * Config::Layout::Zoom::popupScale);
  const int popupHeight = juce::roundToInt((float)waveformBounds.getHeight() * Config::Layout::Zoom::popupScale);
  const juce::Rectangle<int> popupBounds(
      waveformBounds.getCentreX() - popupWidth / 2,
      waveformBounds.getCentreY() - popupHeight / 2,
      popupWidth,
      popupHeight
  );

  const auto& mouse = getMouseHandler();

  double zoomCenterTime = getFocusManager().getFocusedTime();
  double timeRange = audioLength / (double)getZoomFactor();
  timeRange = juce::jlimit(0.00005, audioLength, timeRange);

  const double startTime = zoomCenterTime - (timeRange / 2.0);
  const double endTime = startTime + timeRange;

  setZoomPopupBounds(popupBounds);
  setZoomTimeRange(startTime, endTime);

  g.setColour(juce::Colours::black);
  g.fillRect(popupBounds);

  g.setColour(Config::Colors::waveform);
  const auto channelMode = getChannelViewMode();
  const int numChannels = audioPlayer.getWaveformManager().getThumbnail().getNumChannels();

  if (channelMode == AppEnums::ChannelViewMode::Mono || numChannels == 1)
  {
    audioPlayer.getWaveformManager().getThumbnail().drawChannel(g, popupBounds, startTime, endTime, 0, 1.0f);
    g.setColour(Config::Colors::zoomPopupZeroLine);
    g.drawHorizontalLine(popupBounds.getCentreY(), (float)popupBounds.getX(), (float)popupBounds.getRight());
  }
  else
  {
    auto topBounds = popupBounds.withHeight(popupBounds.getHeight() / 2);
    auto bottomBounds = popupBounds.withTop(topBounds.getBottom()).withHeight(popupBounds.getHeight() / 2);

    audioPlayer.getWaveformManager().getThumbnail().drawChannel(g, topBounds, startTime, endTime, 0, 1.0f);
    audioPlayer.getWaveformManager().getThumbnail().drawChannel(g, bottomBounds, startTime, endTime, 1, 1.0f);

    g.setColour(Config::Colors::zoomPopupZeroLine);
    g.drawHorizontalLine(topBounds.getCentreY(), (float)topBounds.getX(), (float)topBounds.getRight());
    g.drawHorizontalLine(bottomBounds.getCentreY(), (float)bottomBounds.getX(), (float)bottomBounds.getRight());
  }

  auto drawShadow = [&](double startT, double endT, juce::Colour color) {
    if (endT <= startTime || startT >= endTime) return;
    double vStart = juce::jmax(startT, startTime);
    double vEnd = juce::jmin(endT, endTime);
    float x1 = (float)popupBounds.getX() + (float)popupBounds.getWidth() * (float)((vStart - startTime) / timeRange);
    float x2 = (float)popupBounds.getX() + (float)popupBounds.getWidth() * (float)((vEnd - startTime) / timeRange);
    g.setColour(color);
    g.fillRect(x1, (float)popupBounds.getY(), x2 - x1, (float)popupBounds.getHeight());
  };

  const double cutIn = getCutInPosition();
  const double cutOut = getCutOutPosition();

  drawShadow(startTime, cutIn, juce::Colours::black.withAlpha(0.5f));
  drawShadow(cutOut, endTime, juce::Colours::black.withAlpha(0.5f));

  if (startTime < 0.0)
    drawShadow(startTime, 0.0, juce::Colours::black);
  if (endTime > audioLength)
    drawShadow(audioLength, endTime, juce::Colours::black);

  auto drawFineLine = [&](double time, juce::Colour color, float thickness) {
    if (time >= startTime && time <= endTime) {
      float proportion = (float)((time - startTime) / timeRange);
      float x = (float)popupBounds.getX() + proportion * (float)popupBounds.getWidth();
      g.setColour(color);
      g.drawLine(x, (float)popupBounds.getY(), x, (float)popupBounds.getBottom(), thickness);
    }
  };

  bool isDraggingCutIn = mouse.getDraggedHandle() == MouseHandler::CutMarkerHandle::In;
  bool isDraggingCutOut = mouse.getDraggedHandle() == MouseHandler::CutMarkerHandle::Out;

  drawFineLine(cutIn, Config::Colors::cutLine, 1.0f);
  drawFineLine(cutOut, Config::Colors::cutLine, 1.0f);
  drawFineLine(audioPlayer.getTransportSource().getCurrentPosition(), Config::Colors::playbackCursor, 1.0f);

  if (isDraggingCutIn || isDraggingCutOut)
    drawFineLine(isDraggingCutIn ? cutIn : cutOut, Config::Colors::zoomPopupTrackingLine, 2.0f);
  else
    drawFineLine(audioPlayer.getTransportSource().getCurrentPosition(), Config::Colors::zoomPopupPlaybackLine, 2.0f);

  g.setColour(Config::Colors::zoomPopupBorder);
  g.drawRect(popupBounds.toFloat(), Config::Layout::Zoom::borderThickness);
}

void ControlPanel::updateCursorPosition() {
  if (playbackCursorView != nullptr)
    playbackCursorView->repaint();
}

void ControlPanel::timerCallback() {
  const bool isZDown = juce::KeyPress::isKeyCurrentlyDown('z') || juce::KeyPress::isKeyCurrentlyDown('Z');
  setZKeyDown(isZDown);

  updateCutLabels();
  updateCursorPosition();
  if (cutLayerView != nullptr)
    cutLayerView->repaint();
}
