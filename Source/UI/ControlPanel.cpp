

#include "UI/ControlPanel.h"
#include "Core/AudioPlayer.h"
#include "Utils/Config.h"
#include "Presenters/ControlButtonsPresenter.h"
#include "Utils/Config.h"
#include "Presenters/ControlStatePresenter.h"
#include "UI/FocusManager.h"
#include "UI/LayoutManager.h"
#include "Presenters/CutButtonPresenter.h"
#include "Presenters/RepeatPresenter.h"
#include "Presenters/CutResetPresenter.h"
#include "MainComponent.h"
#include "UI/Views/PlaybackCursorView.h"
#include "UI/Views/PlaybackCursorGlow.h"
#include "Presenters/PlaybackTextPresenter.h"
#include "Presenters/SilenceDetectionPresenter.h"
#include "Presenters/StatsPresenter.h"
#include "Utils/TimeUtils.h"
#include "Presenters/TransportPresenter.h"
#include "UI/Views/WaveformView.h"
#include "UI/Views/CutLayerView.h"
#include "Presenters/CutPresenter.h"
#include "Utils/CoordinateMapper.h"
#include "UI/Views/ZoomView.h"
#include "Presenters/PlaybackTimerManager.h"
#include "Presenters/PlaybackRepeatController.h"
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

  playbackTimerManager = std::make_unique<PlaybackTimerManager>(sessionState, getAudioPlayer());
  playbackTimerManager->addListener(playbackCursorView.get());
  playbackTimerManager->addListener(zoomView.get());
  playbackTimerManager->addListener(cutLayerView.get());
  playbackTimerManager->addListener(this);

  playbackRepeatController = std::make_unique<PlaybackRepeatController>(getAudioPlayer(), *this);

  playbackTimerManager->setRepeatController(playbackRepeatController.get());
  playbackTimerManager->setZoomPointProvider([this]() {
    auto dragged = getMouseHandler().getDraggedHandle();
    if (dragged == MouseHandler::CutMarkerHandle::In)
      return AppEnums::ActiveZoomPoint::In;
    if (dragged == MouseHandler::CutMarkerHandle::Out)
      return AppEnums::ActiveZoomPoint::Out;
    return AppEnums::ActiveZoomPoint::None;
  });

  transportStrip = std::make_unique<TransportStrip>(getAudioPlayer(), sessionState);
  addAndMakeVisible(transportStrip.get());

  statsPresenter = std::make_unique<StatsPresenter>(*this);
  silenceDetectionPresenter = std::make_unique<SilenceDetectionPresenter>(*this, sessionState, *owner.getAudioPlayer());
  playbackTextPresenter = std::make_unique<PlaybackTextPresenter>(*this);

  buttonPresenter = std::make_unique<ControlButtonsPresenter>(*this);
  buttonPresenter->initialiseAllButtons();

  cutButtonPresenter = std::make_unique<CutButtonPresenter>(*this);
  cutResetPresenter = std::make_unique<CutResetPresenter>(*this);

  inStrip = std::make_unique<MarkerStrip>(MarkerStrip::MarkerType::In, getAudioPlayer(), sessionState, *silenceDetector);
  inStrip->onMarkerRightClick = [this] {
    getMouseHandler().setPlacementMode(AppEnums::PlacementMode::CutIn);
    updateCutButtonColors();
    repaint();
  };
  addAndMakeVisible(inStrip.get());

  outStrip = std::make_unique<MarkerStrip>(MarkerStrip::MarkerType::Out, getAudioPlayer(), sessionState, *silenceDetector);
  outStrip->onMarkerRightClick = [this] {
    getMouseHandler().setPlacementMode(AppEnums::PlacementMode::CutOut);
    updateCutButtonColors();
    repaint();
  };
  addAndMakeVisible(outStrip.get());

  repeatPresenter = std::make_unique<RepeatPresenter>(*this, *silenceDetector,
                                                  inStrip->getTimerEditor(), outStrip->getTimerEditor());
  repeatPresenter->initialiseEditors();

  inStrip->setPresenter(repeatPresenter.get());
  outStrip->setPresenter(repeatPresenter.get());

  controlStatePresenter = std::make_unique<ControlStatePresenter>(*this);
  transportPresenter = std::make_unique<TransportPresenter>(*this);

  sessionState.addListener(this);

  updateUIFromState();

  finaliseSetup();

  setMouseCursor(juce::MouseCursor::CrosshairCursor);
}

ControlPanel::~ControlPanel() {
  if (playbackTimerManager != nullptr) {
    playbackTimerManager->stopTimer();
  }

  sessionState.removeListener(this);
  
  setLookAndFeel(nullptr);
}

void ControlPanel::initialiseLookAndFeel() {

  setLookAndFeel(&modernLF);
  modernLF.setBaseOffColor(Config::Colors::Button::base);
  modernLF.setBaseOnColor(Config::Colors::Button::on);
  modernLF.setTextColor(Config::Colors::Button::text);
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

void ControlPanel::paintOverChildren(juce::Graphics &g) {
  if (!m_showEyeCandy)
    return;

  auto audioLength = getAudioPlayer().getThumbnail().getTotalLength();
  if (audioLength <= 0.0)
    return;

  const auto bounds = layoutCache.waveformBounds;
  const double cutIn = getCutInPosition();
  const double cutOut = getCutOutPosition();

  float inX = (float)bounds.getX() +
               CoordinateMapper::secondsToPixels(cutIn, (float)bounds.getWidth(),
                                                 audioLength);
  float outX = (float)bounds.getX() +
                CoordinateMapper::secondsToPixels(cutOut, (float)bounds.getWidth(),
                                                  audioLength);

  const float pulse = getGlowAlpha();
  const juce::Colour blueColor = Config::Colors::cutLine.withAlpha(0.5f + 0.3f * pulse);

  // 1. Draw connecting lines
  auto drawConnector = [&](float x, juce::Component &btn, bool toTop) {
    auto btnBounds = btn.getBounds();
    float btnCenterX = (float)btnBounds.getCentreX();
    float btnBottomY = (float)btnBounds.getBottom();

    g.setColour(blueColor);
    g.drawLine(btnCenterX, btnBottomY, x, toTop ? (float)bounds.getY() : (float)bounds.getBottom(), 2.0f);
  };

  // In Strip Connectors
  drawConnector(inX, inStrip->getMarkerButton(), false);    // In Button -> Bottom of Marker
  drawConnector(inX, inStrip->getAutoCutButton(), true); // AutoCut In -> Top of Marker

  // Out Strip Connectors
  drawConnector(outX, outStrip->getMarkerButton(), false);    // Out Button -> Bottom of Marker
  drawConnector(outX, outStrip->getAutoCutButton(), true); // AutoCut Out -> Top of Marker

  // 2. Draw group outlines
  auto drawGroupOutline = [&](const std::vector<juce::Component*>& comps) {
      if (comps.empty()) return;
      juce::Rectangle<int> groupBounds = comps[0]->getBounds();
      for (auto* c : comps)
          groupBounds = groupBounds.getUnion(c->getBounds());
      
      g.setColour(blueColor);
      g.drawRect(groupBounds.expanded(3).toFloat(), 2.0f);
  };

  std::vector<juce::Component*> inGroup = { 
      &inStrip->getMarkerButton(), &inStrip->getTimerEditor(), &inStrip->getResetButton(), 
      &getSilenceDetector().getInSilenceThresholdEditor(), &inStrip->getAutoCutButton() 
  };
  std::vector<juce::Component*> outGroup = { 
      &outStrip->getMarkerButton(), &outStrip->getTimerEditor(), &outStrip->getResetButton(), 
      &getSilenceDetector().getOutSilenceThresholdEditor(), &outStrip->getAutoCutButton() 
  };

  drawGroupOutline(inGroup);
  drawGroupOutline(outGroup);
}

void ControlPanel::updatePlayButtonText(bool isPlaying) {
  if (transportStrip != nullptr)
    transportStrip->updatePlayButtonText(isPlaying);
}

AppEnums::ActiveZoomPoint ControlPanel::getActiveZoomPoint() const {
  if (playbackTimerManager != nullptr)
    return playbackTimerManager->getActiveZoomPoint();
  return AppEnums::ActiveZoomPoint::None;
}

bool ControlPanel::isZKeyDown() const {
  if (playbackTimerManager != nullptr)
    return playbackTimerManager->isZKeyDown();
  return false;
}

void ControlPanel::playbackTimerTick() {
  updateCutLabels();
  repaint();
}

void ControlPanel::activeZoomPointChanged(AppEnums::ActiveZoomPoint newPoint) {
  if (newPoint == AppEnums::ActiveZoomPoint::None) {
    performDelayedJumpIfNeeded();
  }

  if (zoomView != nullptr)
    zoomView->repaint();

  repaint();
}

void ControlPanel::animationUpdate(float breathingPulse) {
  m_currentPulseAlpha = breathingPulse;

  if (cutLayerView != nullptr)
    cutLayerView->repaint();
}

void ControlPanel::cutPreferenceChanged(const MainDomain::CutPreferences& prefs) {
  m_isCutModeActive = prefs.active;
  
  if (transportStrip != nullptr) {
    transportStrip->updateAutoplayState(prefs.autoplay);
    transportStrip->updateCutModeState(prefs.active);
  }

  if (inStrip != nullptr)
    inStrip->updateAutoCutState(prefs.autoCut.inActive);
  if (outStrip != nullptr)
    outStrip->updateAutoCutState(prefs.autoCut.outActive);
  
  if (silenceDetector != nullptr) {
    silenceDetector->setIsAutoCutInActive(prefs.autoCut.inActive);
    silenceDetector->setIsAutoCutOutActive(prefs.autoCut.outActive);
    
    if (prefs.autoCut.inActive && getAudioPlayer().getThumbnail().getTotalLength() > 0.0)
      silenceDetector->detectInSilence();
      
    if (prefs.autoCut.outActive && getAudioPlayer().getThumbnail().getTotalLength() > 0.0)
      silenceDetector->detectOutSilence();
  }

  updateComponentStates();
  repaint();
}

void ControlPanel::cutInChanged(double value) {
  juce::ignoreUnused(value);
  updateCutLabels();
  repaint();
}

void ControlPanel::cutOutChanged(double value) {
  juce::ignoreUnused(value);
  updateCutLabels();
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
}

void ControlPanel::setCutOutPosition(double pos) {
  sessionState.setCutOut(pos);
}

void ControlPanel::updateCutLabels() {
  if (inStrip != nullptr)
    inStrip->updateTimerText(TimeUtils::formatTime(getCutInPosition()));
  if (outStrip != nullptr)
    outStrip->updateTimerText(TimeUtils::formatTime(getCutOutPosition()));

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
  const auto &prefs = sessionState.getCutPrefs();
  const auto &autoCut = prefs.autoCut;
  
  m_isCutModeActive = prefs.active;
  
  if (transportStrip != nullptr) {
    transportStrip->updateAutoplayState(prefs.autoplay);
    transportStrip->updateCutModeState(prefs.active);
  }

  if (inStrip != nullptr)
    inStrip->updateAutoCutState(autoCut.inActive);
  if (outStrip != nullptr)
    outStrip->updateAutoCutState(autoCut.outActive);

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
    zoomView->repaint();

  repaint();
}

void ControlPanel::setAutoCutInActive(bool isActive) {
  sessionState.setAutoCutInActive(isActive);
  if (inStrip != nullptr)
    inStrip->updateAutoCutState(isActive);
}

void ControlPanel::setAutoCutOutActive(bool isActive) {
  sessionState.setAutoCutOutActive(isActive);
  if (outStrip != nullptr)
    outStrip->updateAutoCutState(isActive);
}

void ControlPanel::toggleStats() {
  if (statsPresenter == nullptr)
    return;

  statsPresenter->toggleVisibility();
  statsButton.setToggleState(statsPresenter->isShowingStats(),
                             juce::dontSendNotification);

  updateComponentStates();
}

void ControlPanel::triggerModeButton() { modeButton.triggerClick(); }

void ControlPanel::triggerChannelViewButton() { channelViewButton.triggerClick(); }

void ControlPanel::triggerRepeatButton() { 
  if (transportStrip != nullptr)
    transportStrip->getRepeatButton().triggerClick(); 
}

void ControlPanel::resetIn() { if (inStrip != nullptr) inStrip->getResetButton().triggerClick(); }

void ControlPanel::resetOut() { if (outStrip != nullptr) outStrip->getResetButton().triggerClick(); }

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

