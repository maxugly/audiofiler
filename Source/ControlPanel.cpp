#include "ControlPanel.h"
#include "AudioPlayer.h" // Required for AudioPlayer types in public methods
#include "Config.h"
#include "ControlButtonsPresenter.h"
#include "ControlPanelCopy.h"
#include "ControlStatePresenter.h"
#include "FocusManager.h"
#include "LayoutManager.h"
#include "CutButtonPresenter.h"

#include "LoopPresenter.h"
#include "CutResetPresenter.h"
#include "MainComponent.h" // Full header required for MainComponent access (e.g., getAudioPlayer)
#include "PlaybackOverlay.h"
#include "PlaybackCursorGlow.h"
#include "PlaybackTextPresenter.h"
#include "SilenceDetectionPresenter.h"
#include "StatsPresenter.h"
#include "TimeUtils.h"
#include "TransportPresenter.h"
#include "WaveformRenderer.h"
#include <cmath> // For std::abs

/**
 * @file ControlPanel.cpp
 * @brief Implements the ControlPanel class, which manages the application's UI
 * controls and interactions.
 */

/**
 * @brief Constructs the ControlPanel.
 * @param ownerComponent A reference to the `MainComponent` that owns this
 * panel. This reference is vital for inter-component communication, allowing
 * the `ControlPanel` to delegate core application logic (like file opening or
 * audio playback) to its owner.
 *
 * This constructor initializes member variables, including the
 * `ModernLookAndFeel` for custom styling and a `SilenceDetector` for automatic
 * loop point finding. It then calls various `initialise` methods to set up all
 * UI buttons, editors, and their respective callbacks, and finally performs any
 * necessary post-initialization setup with `finaliseSetup()`. The mouse cursor
 * is set to `CrosshairCursor` to provide immediate visual feedback for
 * interactive elements.
 */
ControlPanel::ControlPanel(MainComponent &ownerComponent, SessionState &sessionStateIn)
    : owner(ownerComponent),
      sessionState(sessionStateIn),
      modernLF(),
      silenceDetector(std::make_unique<SilenceDetector>(*this)),
      mouseHandler(std::make_unique<MouseHandler>(*this)),
      layoutManager(std::make_unique<LayoutManager>(*this)),
      waveformRenderer(std::make_unique<WaveformRenderer>(sessionState, owner.getAudioPlayer()->getWaveformManager())),
      focusManager(std::make_unique<FocusManager>(*this)) {
  initialiseLookAndFeel();
  statsPresenter = std::make_unique<StatsPresenter>(*this);
  silenceDetectionPresenter =
      std::make_unique<SilenceDetectionPresenter>(*this);
  playbackTextPresenter = std::make_unique<PlaybackTextPresenter>(*this);
  playbackOverlay = std::make_unique<PlaybackOverlay>(*this);
  addAndMakeVisible(playbackOverlay.get());
  playbackOverlay->setInterceptsMouseClicks(false, false);
  buttonPresenter = std::make_unique<ControlButtonsPresenter>(*this);
  buttonPresenter->initialiseAllButtons();

  cutButtonPresenter = std::make_unique<CutButtonPresenter>(*this);
  loopPresenter = std::make_unique<LoopPresenter>(*this, *silenceDetector,
                                                  cutInEditor, cutOutEditor);
  loopPresenter->initialiseEditors();

  initialiseCutEditors(); // Contains remaining init logic (reset presenter,
                           // thresholds)

  controlStatePresenter = std::make_unique<ControlStatePresenter>(*this);
  transportPresenter = std::make_unique<TransportPresenter>(*this);
  updateUIFromState();
  finaliseSetup();

  startTimerHz(60);
  setMouseCursor(juce::MouseCursor::CrosshairCursor);
}

/**
 * @brief Destructor for the ControlPanel.
 *
 * Ensures that the custom `ModernLookAndFeel` instance is properly dereferenced
 * by setting the LookAndFeel to `nullptr`. This prevents potential issues if
 * the custom LookAndFeel outlives components that were using it.
 */
ControlPanel::~ControlPanel() {
  stopTimer();
  setLookAndFeel(nullptr);
}

/**
 * @brief Initializes and applies the custom `ModernLookAndFeel`.
 *
 * This method sets the `ModernLookAndFeel` instance as the active look and feel
 * for this component and its children. It also configures the base and active
 * colors for buttons, as well as text colors, using values from `Config.h`
 * to ensure a consistent visual theme across the application.
 */
void ControlPanel::initialiseLookAndFeel() {
  setLookAndFeel(&modernLF);
  modernLF.setBaseOffColor(Config::Colors::Button::base);
  modernLF.setBaseOnColor(Config::Colors::Button::on);
  modernLF.setTextColor(Config::Colors::Button::text);
}

/**
 * @brief Initializes the cut editors (`cutInEditor`, `cutOutEditor`) and
 * threshold editors.
 */
void ControlPanel::initialiseCutEditors() {

  cutResetPresenter = std::make_unique<CutResetPresenter>(*this);

  addAndMakeVisible(silenceDetector->getInSilenceThresholdEditor());
  addAndMakeVisible(silenceDetector->getOutSilenceThresholdEditor());
}
/**
 * @brief Performs final setup steps after all components are initialized.
 *
 * This method ensures that the loop time labels are correctly displayed
 * and that the enabled/disabled and visible states of all UI components
 * are updated to reflect the initial application state (e.g., no audio loaded).
 */
void ControlPanel::invokeOwnerOpenDialog() { owner.openButtonClicked(); }

void ControlPanel::finaliseSetup() {
  if (playbackTextPresenter != nullptr)
    playbackTextPresenter->initialiseEditors();
  updateCutLabels();
  updateComponentStates();
}

/**
 * @brief Recalculates the layout of all child components when the ControlPanel
 * is resized.
 *
 * This method is central to the responsive design of the UI. It divides the
 * available area into logical sections (top row, loop/cut controls, bottom row,
 * and the main waveform/stats area) and calls specialized layout helper methods
 * to position buttons, editors, and display areas dynamically.
 */
void ControlPanel::resized() {
  if (layoutManager != nullptr)
    layoutManager->performLayout();

  if (playbackTextPresenter != nullptr)
    playbackTextPresenter->layoutEditors();

  if (playbackOverlay != nullptr)
    playbackOverlay->setBounds(layoutCache.waveformBounds);
}

void ControlPanel::paint(juce::Graphics &g) {
  g.fillAll(Config::Colors::Window::background);
  if (waveformRenderer != nullptr)
    waveformRenderer->renderWaveform(g, layoutCache.waveformBounds, currentQuality, currentChannelViewMode);
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
    // If we are dragging a handle, zoom to that handle
    auto dragged = mouseHandler->getDraggedHandle();
    if (dragged == MouseHandler::CutMarkerHandle::In)
      m_activeZoomPoint = ActiveZoomPoint::In;
    else if (dragged == MouseHandler::CutMarkerHandle::Out)
      m_activeZoomPoint = ActiveZoomPoint::Out;
  } else {
    // On release, we only stop zooming if we aren't hovering/focusing a loop
    // editor But for now, let's keep it simple: release 'z' -> no zoom unless
    // we want to keep it on hover. The user said 'z' is a momentary switch, so
    // let's prioritise that.
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
  if (loopPresenter != nullptr)
    loopPresenter->updateCutLabels();
}

void ControlPanel::setCutOutPosition(double pos) {
  sessionState.setCutOut(pos);
  if (loopPresenter != nullptr)
    loopPresenter->updateCutLabels();
}

void ControlPanel::updateCutLabels() {
  if (loopPresenter != nullptr)
    loopPresenter->updateCutLabels();

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

/**
 * @brief Lays out the buttons for the top row of the control panel.
 * @param bounds The current bounds of the control panel.
 * @param rowHeight The calculated height for each button row.
 */

void ControlPanel::updateQualityButtonText() {
  if (currentQuality == AppEnums::ThumbnailQuality::High)
    qualityButton.setButtonText(ControlPanelCopy::qualityHighText());
  else if (currentQuality == AppEnums::ThumbnailQuality::Medium)
    qualityButton.setButtonText(ControlPanelCopy::qualityMediumText());
  else
    qualityButton.setButtonText(ControlPanelCopy::qualityLowText());
}

/**
 * @brief Toggles the visibility of the statistics display.
 */
void ControlPanel::toggleStats() {
  if (statsPresenter == nullptr)
    return;

  statsPresenter->toggleVisibility();
  statsButton.setToggleState(statsPresenter->isShowingStats(),
                             juce::dontSendNotification);
  updateComponentStates();
}

/**
 * @brief Triggers the quality button's action, cycling through quality
 * settings.
 */
void ControlPanel::triggerQualityButton() { qualityButton.triggerClick(); }

/**
 * @brief Triggers the mode button's action, cycling through view modes.
 */
void ControlPanel::triggerModeButton() { modeButton.triggerClick(); }

/**
 * @brief Triggers the channel view button's action, cycling through channel
 * view modes.
 */
void ControlPanel::triggerChannelViewButton() {
  channelViewButton.triggerClick();
}

/**
 * @brief Triggers the loop button's action, toggling looping on/off.
 */
void ControlPanel::triggerLoopButton() { loopButton.triggerClick(); }

/**
 * @brief Triggers the reset-in button's action, resetting the cut-in boundary.
 */
void ControlPanel::resetIn() { resetInButton.triggerClick(); }

/**
 * @brief Triggers the reset-out button's action, resetting the cut-out boundary.
 */
void ControlPanel::resetOut() { resetOutButton.triggerClick(); }


/**
 * @brief Parses a time string (HH:MM:SS:mmm) into a double representing
 * seconds.
 * @param timeString The time string to parse.
 * @return The time in seconds, or -1.0 if parsing fails.
 */
/**
 * @brief Sets the text in the stats display box.
 * @param text The text to display.
 * @param color The color of the text.
 */
void ControlPanel::setStatsDisplayText(const juce::String &text,
                                       juce::Colour color) {
  if (statsPresenter != nullptr)
    statsPresenter->setDisplayText(text, color);
}

void ControlPanel::logStatusMessage(const juce::String &message, bool isError) {
  const auto color =
      isError ? Config::Colors::statsErrorText : Config::Colors::statsText;
  setStatsDisplayText(message, color);
}

void ControlPanel::updateStatsFromAudio() {
  if (statsPresenter != nullptr)
    statsPresenter->updateStats();
}
void ControlPanel::ensureLoopOrder() {
  if (loopPresenter != nullptr)
    loopPresenter->ensureLoopOrder();
}

void ControlPanel::setShouldShowStats(bool shouldShowStatsParam) {
  if (statsPresenter != nullptr)
    statsPresenter->setShouldShowStats(shouldShowStatsParam);
}

void ControlPanel::setTotalTimeStaticString(const juce::String &timeString) {
  if (playbackTextPresenter != nullptr)
    playbackTextPresenter->setTotalTimeStaticString(timeString);
}

void ControlPanel::setShouldLoop(bool shouldLoopParam) {
  shouldLoop = shouldLoopParam;
}
void ControlPanel::updateLoopButtonColors() {
  if (cutButtonPresenter != nullptr)
    cutButtonPresenter->updateColours();
}

// Public accessors for SilenceDetector and other classes to interact with
// ControlPanel
AudioPlayer &ControlPanel::getAudioPlayer() const {
  return *owner.getAudioPlayer();
}

AudioPlayer &ControlPanel::getAudioPlayer() { return *owner.getAudioPlayer(); }

juce::TextEditor &ControlPanel::getStatsDisplay() {
  jassert(statsPresenter != nullptr);
  return statsPresenter->getDisplay();
}

void ControlPanel::setCutStart(int sampleIndex) {
  if (loopPresenter != nullptr)
    loopPresenter->setCutStartFromSample(sampleIndex);
}

void ControlPanel::setCutEnd(int sampleIndex) {
  if (loopPresenter != nullptr)
    loopPresenter->setCutEndFromSample(sampleIndex);
}

juce::String ControlPanel::formatTime(double seconds) const {
  return TimeUtils::formatTime(seconds);
}

const juce::LookAndFeel &ControlPanel::getLookAndFeel() const {
  return modernLF;
}

AppEnums::PlacementMode ControlPanel::getPlacementMode() const {
  return mouseHandler->getCurrentPlacementMode();
}

void ControlPanel::mouseMove(const juce::MouseEvent &event) {
  mouseHandler->mouseMove(event);
}

void ControlPanel::mouseDown(const juce::MouseEvent &event) {
  mouseHandler->mouseDown(event);
}

void ControlPanel::mouseDrag(const juce::MouseEvent &event) {
  mouseHandler->mouseDrag(event);
}

void ControlPanel::mouseUp(const juce::MouseEvent &event) {
  mouseHandler->mouseUp(event);
}

void ControlPanel::mouseExit(const juce::MouseEvent &event) {
  mouseHandler->mouseExit(event);
}

void ControlPanel::mouseWheelMove(const juce::MouseEvent &event,
                                  const juce::MouseWheelDetails &wheel) {
  mouseHandler->mouseWheelMove(event, wheel);
}

void ControlPanel::forceInvalidateWaveformCache() {
  if (waveformRenderer != nullptr)
    waveformRenderer->invalidateWaveformCache();
}

void ControlPanel::renderOverlays(juce::Graphics &g) {
  if (isCutModeActive())
    drawCutModeOverlays(g);
  drawMouseCursorOverlays(g);
  drawZoomPopup(g);
}

void ControlPanel::drawCutModeOverlays(juce::Graphics& g) {
  const auto waveformBounds = getWaveformBounds();
  auto& audioPlayer = getAudioPlayer();
  const float audioLength = (float)audioPlayer.getWaveformManager().getThumbnail().getTotalLength();
  if (audioLength <= 0.0f)
    return;

  const auto& silenceDetectorRef = getSilenceDetector();

  auto drawThresholdVisualisation = [&](double cutPos, float threshold)
  {
    if (audioLength <= 0.0f)
      return;

    const float normalisedThreshold = threshold;
    const float centerY = (float)waveformBounds.getCentreY();
    const float halfHeight = (float)waveformBounds.getHeight() / 2.0f;

    float topThresholdY = centerY - (normalisedThreshold * halfHeight);
    float bottomThresholdY = centerY + (normalisedThreshold * halfHeight);

    topThresholdY = juce::jlimit((float)waveformBounds.getY(), (float)waveformBounds.getBottom(), topThresholdY);
    bottomThresholdY = juce::jlimit((float)waveformBounds.getY(), (float)waveformBounds.getBottom(), bottomThresholdY);

    const float xPos = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (float)(cutPos / audioLength);
    const float halfThresholdLineWidth = Config::Animation::thresholdLineWidth / 2.0f;
    float lineStartX = xPos - halfThresholdLineWidth;
    float lineEndX = xPos + halfThresholdLineWidth;

    lineStartX = juce::jmax(lineStartX, (float)waveformBounds.getX());
    lineEndX = juce::jmin(lineEndX, (float)waveformBounds.getRight());
    const float currentLineWidth = lineEndX - lineStartX;

    g.setColour(Config::Colors::thresholdRegion);
    g.fillRect(lineStartX, topThresholdY, currentLineWidth, bottomThresholdY - topThresholdY);

    if (isCutModeActive())
    {
      const juce::Colour glowColor = Config::Colors::thresholdLine.withAlpha(Config::Colors::thresholdLine.getFloatAlpha() * getGlowAlpha());
      g.setColour(glowColor);
      g.fillRect(lineStartX, topThresholdY - (Config::Layout::Glow::thresholdGlowThickness * Config::Layout::Glow::offsetFactor - 0.5f), currentLineWidth, Config::Layout::Glow::thresholdGlowThickness);
      g.fillRect(lineStartX, bottomThresholdY - (Config::Layout::Glow::thresholdGlowThickness * Config::Layout::Glow::offsetFactor - 0.5f), currentLineWidth, Config::Layout::Glow::thresholdGlowThickness);
    }

    g.setColour(Config::Colors::thresholdLine);
    g.drawHorizontalLine((int)topThresholdY, lineStartX, lineEndX);
    g.drawHorizontalLine((int)bottomThresholdY, lineStartX, lineEndX);
  };

  const double cutIn = audioPlayer.getCutIn();
  const double cutOut = audioPlayer.getCutOut();
  drawThresholdVisualisation(cutIn, silenceDetectorRef.getCurrentInSilenceThreshold());
  drawThresholdVisualisation(cutOut, silenceDetectorRef.getCurrentOutSilenceThreshold());

  const double actualIn = juce::jmin(cutIn, cutOut);
  const double actualOut = juce::jmax(cutIn, cutOut);
  const float inX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (float)(actualIn / audioLength);
  const float outX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (float)(actualOut / audioLength);
  const float fadeLength = waveformBounds.getWidth() * Config::Layout::Waveform::loopRegionFadeProportion;
  const float boxHeight = (float)Config::Layout::Glow::loopMarkerBoxHeight;

  const juce::Rectangle<float> leftRegion((float)waveformBounds.getX(), (float)waveformBounds.getY(), inX - (float)waveformBounds.getX(), (float)waveformBounds.getHeight());
  if (leftRegion.getWidth() > 0.0f)
  {
    const float actualFade = juce::jmin(fadeLength, leftRegion.getWidth());

    juce::Rectangle<float> solidBlackLeft = leftRegion.withWidth(leftRegion.getWidth() - actualFade);
    g.setColour(juce::Colours::black);
    g.fillRect(solidBlackLeft);

    juce::Rectangle<float> fadeAreaLeft(inX - actualFade, (float)waveformBounds.getY(), actualFade, (float)waveformBounds.getHeight());
    juce::ColourGradient leftFadeGradient(Config::Colors::loopRegion, inX, leftRegion.getCentreY(),
                                          juce::Colours::black, inX - actualFade, leftRegion.getCentreY(), false);
    g.setGradientFill(leftFadeGradient);
    g.fillRect(fadeAreaLeft);
  }

  const juce::Rectangle<float> rightRegion(outX, (float)waveformBounds.getY(), (float)waveformBounds.getRight() - outX, (float)waveformBounds.getHeight());
  if (rightRegion.getWidth() > 0.0f)
  {
    const float actualFade = juce::jmin(fadeLength, rightRegion.getWidth());

    float solidBlackStart = outX + actualFade;
    juce::Rectangle<float> solidBlackRight(solidBlackStart, (float)waveformBounds.getY(), (float)waveformBounds.getRight() - solidBlackStart, (float)waveformBounds.getHeight());
    g.setColour(juce::Colours::black);
    g.fillRect(solidBlackRight);

    juce::Rectangle<float> fadeAreaRight(outX, (float)waveformBounds.getY(), actualFade, (float)waveformBounds.getHeight());
    juce::ColourGradient rightFadeGradient(Config::Colors::loopRegion, outX, rightRegion.getCentreY(),
                                           juce::Colours::black, outX + actualFade, rightRegion.getCentreY(), false);
    g.setGradientFill(rightFadeGradient);
    g.fillRect(fadeAreaRight);
  }

  const juce::Colour glowColor = Config::Colors::loopLine.withAlpha(Config::Colors::loopLine.getFloatAlpha() * (1.0f - getGlowAlpha()));
  g.setColour(glowColor);
  g.fillRect(inX - (Config::Layout::Glow::loopLineGlowThickness * Config::Layout::Glow::offsetFactor - 0.5f), (float)waveformBounds.getY() + boxHeight, Config::Layout::Glow::loopLineGlowThickness, (float)waveformBounds.getHeight() - (2.0f * boxHeight));
  g.fillRect(outX - (Config::Layout::Glow::loopLineGlowThickness * Config::Layout::Glow::offsetFactor - 0.5f), (float)waveformBounds.getY() + boxHeight, Config::Layout::Glow::loopLineGlowThickness, (float)waveformBounds.getHeight() - (2.0f * boxHeight));

  g.setColour(Config::Colors::loopLine);
  auto drawCutMarker = [&](float x, MouseHandler::CutMarkerHandle handleType) {
    const auto& mouseRef = getMouseHandler();
    const auto& silenceRef = getSilenceDetector();

    juce::Colour markerColor = Config::Colors::loopLine;

    if (handleType == MouseHandler::CutMarkerHandle::In && silenceRef.getIsAutoCutInActive())
      markerColor = Config::Colors::loopMarkerAuto;
    else if (handleType == MouseHandler::CutMarkerHandle::Out && silenceRef.getIsAutoCutOutActive())
      markerColor = Config::Colors::loopMarkerAuto;

    float thickness = Config::Layout::Glow::loopBoxOutlineThickness;

    if (mouseRef.getDraggedHandle() == handleType)
    {
      markerColor = Config::Colors::loopMarkerDrag;
      thickness = Config::Layout::Glow::loopBoxOutlineThicknessInteracting;
    }
    else if (mouseRef.getHoveredHandle() == handleType)
    {
      markerColor = Config::Colors::loopMarkerHover;
      thickness = Config::Layout::Glow::loopBoxOutlineThicknessInteracting;
    }

    const float boxWidth = Config::Layout::Glow::loopMarkerBoxWidth;
    const float halfBoxWidth = boxWidth / 2.0f;

    g.setColour(markerColor);
    g.drawRect(x - halfBoxWidth, (float)waveformBounds.getY(), boxWidth, boxHeight, thickness);
    g.drawRect(x - halfBoxWidth, (float)waveformBounds.getBottom() - boxHeight, boxWidth, boxHeight, thickness);

    g.setColour(markerColor);
    g.fillRect(x - Config::Layout::Glow::loopMarkerWidthThin / Config::Layout::Glow::loopMarkerCenterDivisor,
               (float)waveformBounds.getY() + boxHeight,
               Config::Layout::Glow::loopMarkerWidthThin,
               (float)waveformBounds.getHeight() - (2.0f * boxHeight));
  };

  drawCutMarker(inX, MouseHandler::CutMarkerHandle::In);
  drawCutMarker(outX, MouseHandler::CutMarkerHandle::Out);

  const auto& mouseRef = getMouseHandler();
  juce::Colour hollowColor = Config::Colors::loopLine;
  float thickness = Config::Layout::Glow::loopBoxOutlineThickness;

  if (mouseRef.getDraggedHandle() == MouseHandler::CutMarkerHandle::Full)
  {
    hollowColor = Config::Colors::loopMarkerDrag;
    thickness = Config::Layout::Glow::loopBoxOutlineThicknessInteracting;
  }
  else if (mouseRef.getHoveredHandle() == MouseHandler::CutMarkerHandle::Full)
  {
    hollowColor = Config::Colors::loopMarkerHover;
    thickness = Config::Layout::Glow::loopBoxOutlineThicknessInteracting;
  }

  g.setColour(hollowColor);
  const float halfBoxWidth = Config::Layout::Glow::loopMarkerBoxWidth / 2.0f;

  const float startX = inX + halfBoxWidth;
  const float endX = outX - halfBoxWidth;

  if (startX < endX)
  {
    g.drawLine(startX, (float)waveformBounds.getY(), endX, (float)waveformBounds.getY(), thickness);
    g.drawLine(startX, (float)waveformBounds.getY() + boxHeight, endX, (float)waveformBounds.getY() + boxHeight, thickness);
    g.drawLine(startX, (float)waveformBounds.getBottom() - 1.0f, endX, (float)waveformBounds.getBottom() - 1.0f, thickness);
    g.drawLine(startX, (float)waveformBounds.getBottom() - boxHeight, endX, (float)waveformBounds.getBottom() - boxHeight, thickness);
  }
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

  if (mouse.getCurrentPlacementMode() == AppEnums::PlacementMode::LoopIn
      || mouse.getCurrentPlacementMode() == AppEnums::PlacementMode::LoopOut)
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

  drawFineLine(cutIn, Config::Colors::loopLine, 1.0f);
  drawFineLine(cutOut, Config::Colors::loopLine, 1.0f);
  drawFineLine(audioPlayer.getTransportSource().getCurrentPosition(), Config::Colors::playbackCursor, 1.0f);

  if (isDraggingCutIn || isDraggingCutOut)
    drawFineLine(isDraggingCutIn ? cutIn : cutOut, Config::Colors::zoomPopupTrackingLine, 2.0f);
  else
    drawFineLine(audioPlayer.getTransportSource().getCurrentPosition(), Config::Colors::zoomPopupPlaybackLine, 2.0f);

  g.setColour(Config::Colors::zoomPopupBorder);
  g.drawRect(popupBounds.toFloat(), Config::Layout::Zoom::borderThickness);
}

void ControlPanel::updateCursorPosition() {
  if (playbackOverlay != nullptr)
    playbackOverlay->repaint();
}

void ControlPanel::timerCallback() {
  // Handle momentary 'z' zoom key
  const bool isZDown = juce::KeyPress::isKeyCurrentlyDown('z') || juce::KeyPress::isKeyCurrentlyDown('Z');
  setZKeyDown(isZDown);

  updateCutLabels();
  updateCursorPosition();
}
