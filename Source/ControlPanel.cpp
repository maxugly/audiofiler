#include "ControlPanel.h"
#include "MainComponent.h" // Full header required for MainComponent access (e.g., getAudioPlayer)
#include "AudioPlayer.h"    // Required for AudioPlayer types in public methods
#include "Config.h"
#include "ControlPanelCopy.h"
#include "LayoutManager.h"
#include "StatsPresenter.h"
#include "LoopPresenter.h"
#include "ControlStatePresenter.h"
#include "TransportPresenter.h"
#include "SilenceDetectionPresenter.h"
#include "ControlButtonsPresenter.h"
#include "LoopButtonPresenter.h"
#include "LoopEditorPresenter.h"
#include "LoopResetPresenter.h"
#include "WaveformRenderer.h"
#include "PlaybackTextPresenter.h"
#include <cmath> // For std::abs

/**
 * @file ControlPanel.cpp
 * @brief Implements the ControlPanel class, which manages the application's UI controls and interactions.
 */

/**
 * @brief Constructs the ControlPanel.
 * @param ownerComponent A reference to the `MainComponent` that owns this panel.
 *                       This reference is vital for inter-component communication,
 *                       allowing the `ControlPanel` to delegate core application
 *                       logic (like file opening or audio playback) to its owner.
 *
 * This constructor initializes member variables, including the `ModernLookAndFeel`
 * for custom styling and a `SilenceDetector` for automatic loop point finding.
 * It then calls various `initialise` methods to set up all UI buttons, editors,
 * and their respective callbacks, and finally performs any necessary post-initialization
 * setup with `finaliseSetup()`. The mouse cursor is set to `CrosshairCursor`
 * to provide immediate visual feedback for interactive elements.
 */
ControlPanel::ControlPanel(MainComponent& ownerComponent)
    : owner(ownerComponent),
      modernLF(),
      silenceDetector(std::make_unique<SilenceDetector>(*this)),
      mouseHandler(std::make_unique<MouseHandler>(*this)),
      layoutManager(std::make_unique<LayoutManager>(*this)),
      waveformRenderer(std::make_unique<WaveformRenderer>(*this))
{
    initialiseLookAndFeel();
    statsPresenter = std::make_unique<StatsPresenter>(*this);
    silenceDetectionPresenter = std::make_unique<SilenceDetectionPresenter>(*this);
    playbackTextPresenter = std::make_unique<PlaybackTextPresenter>(*this);
    buttonPresenter = std::make_unique<ControlButtonsPresenter>(*this);
    buttonPresenter->initialiseAllButtons();
    initialiseLoopEditors();
    loopButtonPresenter = std::make_unique<LoopButtonPresenter>(*this);
    loopPresenter = std::make_unique<LoopPresenter>(*this, *silenceDetector, loopInEditor, loopOutEditor);
    controlStatePresenter = std::make_unique<ControlStatePresenter>(*this);
    transportPresenter = std::make_unique<TransportPresenter>(*this);
    finaliseSetup();

    setMouseCursor(juce::MouseCursor::CrosshairCursor);
}

/**
 * @brief Destructor for the ControlPanel.
 *
 * Ensures that the custom `ModernLookAndFeel` instance is properly dereferenced
 * by setting the LookAndFeel to `nullptr`. This prevents potential issues if
 * the custom LookAndFeel outlives components that were using it.
 */
ControlPanel::~ControlPanel()
{
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
void ControlPanel::initialiseLookAndFeel()
{
    setLookAndFeel (&modernLF);
    modernLF.setBaseOffColor(Config::Colors::buttonBase);
    modernLF.setBaseOnColor(Config::Colors::buttonOn);
    modernLF.setTextColor(Config::Colors::buttonText);
}

/**
 * @brief Initializes the loop editors (`loopInEditor`, `loopOutEditor`) and threshold editors.
 */
void ControlPanel::initialiseLoopEditors()
{
    loopEditorPresenter = std::make_unique<LoopEditorPresenter>(*this);
    loopEditorPresenter->initialiseEditors();
    loopResetPresenter = std::make_unique<LoopResetPresenter>(*this);

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
void ControlPanel::invokeOwnerOpenDialog()
{
    owner.openButtonClicked();
}

void ControlPanel::finaliseSetup()
{
    if (playbackTextPresenter != nullptr)
        playbackTextPresenter->initialiseEditors();
    updateLoopLabels();
    updateComponentStates();
}

/**
 * @brief Recalculates the layout of all child components when the ControlPanel is resized.
 *
 * This method is central to the responsive design of the UI. It divides the
 * available area into logical sections (top row, loop/cut controls, bottom row,
 * and the main waveform/stats area) and calls specialized layout helper methods
 * to position buttons, editors, and display areas dynamically. Debugging statements
 * (`DBG`) are included to help trace layout changes during development.
 */
void ControlPanel::resized()
{
    if (layoutManager != nullptr)
        layoutManager->performLayout();

    if (playbackTextPresenter != nullptr)
        playbackTextPresenter->layoutEditors();
}

void ControlPanel::paint(juce::Graphics& g)
{
    g.fillAll (Config::Colors::background);
    if (waveformRenderer != nullptr)
        waveformRenderer->render(g);
    if (playbackTextPresenter != nullptr)
        playbackTextPresenter->render(g);
}

void ControlPanel::updatePlayButtonText(bool isPlaying)
{
    playStopButton.setButtonText(isPlaying ? ControlPanelCopy::stopButtonText() : ControlPanelCopy::playButtonText());
}

void ControlPanel::setZKeyDown(bool isDown)
{
    if (m_isZKeyDown == isDown)
        return;

    m_isZKeyDown = isDown;
    
    if (m_isZKeyDown)
    {
        // If we are dragging a handle, zoom to that handle
        auto dragged = mouseHandler->getDraggedHandle();
        if (dragged == MouseHandler::LoopMarkerHandle::In)
            m_activeZoomPoint = ActiveZoomPoint::In;
        else if (dragged == MouseHandler::LoopMarkerHandle::Out)
            m_activeZoomPoint = ActiveZoomPoint::Out;
    }
    else
    {
        // On release, we only stop zooming if we aren't hovering/focusing a loop editor
        // But for now, let's keep it simple: release 'z' -> no zoom unless we want to keep it on hover.
        // The user said 'z' is a momentary switch, so let's prioritise that.
        m_activeZoomPoint = ActiveZoomPoint::None;
        performDelayedJumpIfNeeded();
    }
    repaint();
}

void ControlPanel::jumpToLoopIn()
{
    getAudioPlayer().getTransportSource().setPosition(getLoopInPosition());
    m_needsJumpToLoopIn = false;
}

void ControlPanel::performDelayedJumpIfNeeded()
{
    if (m_needsJumpToLoopIn)
        jumpToLoopIn();
}

double ControlPanel::getLoopInPosition() const
{
    return loopPresenter != nullptr ? loopPresenter->getLoopInPosition() : -1.0;
}

double ControlPanel::getLoopOutPosition() const
{
    return loopPresenter != nullptr ? loopPresenter->getLoopOutPosition() : -1.0;
}

void ControlPanel::setLoopInPosition(double pos)
{
    if (loopPresenter != nullptr)
        loopPresenter->setLoopInPosition(pos);
}

void ControlPanel::setLoopOutPosition(double pos)
{
    if (loopPresenter != nullptr)
        loopPresenter->setLoopOutPosition(pos);
}

void ControlPanel::updateLoopLabels()
{
    if (loopPresenter != nullptr)
        loopPresenter->updateLoopLabels();

    if (playbackTextPresenter != nullptr)
        playbackTextPresenter->updateEditors();
}

void ControlPanel::updateComponentStates()
{
    if (controlStatePresenter != nullptr)
        controlStatePresenter->refreshStates();
}

/**
 * @brief Lays out the buttons for the top row of the control panel.
 * @param bounds The current bounds of the control panel.
 * @param rowHeight The calculated height for each button row.
 */

void ControlPanel::updateQualityButtonText()
{
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
void ControlPanel::toggleStats()
{
    if (statsPresenter == nullptr)
        return;

    statsPresenter->toggleVisibility();
    statsButton.setToggleState(statsPresenter->isShowingStats(), juce::dontSendNotification);
    updateComponentStates();
}

/**
 * @brief Triggers the quality button's action, cycling through quality settings.
 */
void ControlPanel::triggerQualityButton() { qualityButton.triggerClick(); }

/**
 * @brief Triggers the mode button's action, cycling through view modes.
 */
void ControlPanel::triggerModeButton() { modeButton.triggerClick(); }

/**
 * @brief Triggers the channel view button's action, cycling through channel view modes.
 */
void ControlPanel::triggerChannelViewButton() { channelViewButton.triggerClick(); }

/**
 * @brief Triggers the loop button's action, toggling looping on/off.
 */
void ControlPanel::triggerLoopButton() { loopButton.triggerClick(); }

/**
 * @brief Triggers the clear loop in button's action, resetting the loop in position.
 */
void ControlPanel::clearLoopIn() { clearLoopInButton.triggerClick(); }

/**
 * @brief Triggers the clear loop out button's action, resetting the loop out position.
 */
void ControlPanel::clearLoopOut() { clearLoopOutButton.triggerClick(); }

/**
 * @brief Parses a time string (HH:MM:SS:mmm) into a double representing seconds.
 * @param timeString The time string to parse.
 * @return The time in seconds, or -1.0 if parsing fails.
 */
/**
 * @brief Sets the text in the stats display box.
 * @param text The text to display.
 * @param color The color of the text.
 */
void ControlPanel::setStatsDisplayText(const juce::String& text, juce::Colour color) {
    if (statsPresenter != nullptr)
        statsPresenter->setDisplayText(text, color);
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

void ControlPanel::setTotalTimeStaticString(const juce::String& timeString) {
    if (playbackTextPresenter != nullptr)
        playbackTextPresenter->setTotalTimeStaticString(timeString);
}

void ControlPanel::setShouldLoop(bool shouldLoopParam) {
    shouldLoop = shouldLoopParam;
}
void ControlPanel::updateLoopButtonColors() {
    if (loopButtonPresenter != nullptr)
        loopButtonPresenter->updateColours();
}

// Public accessors for SilenceDetector and other classes to interact with ControlPanel
AudioPlayer& ControlPanel::getAudioPlayer() const
{
    return *owner.getAudioPlayer();
}

juce::TextEditor& ControlPanel::getStatsDisplay()
{
    jassert(statsPresenter != nullptr);
    return statsPresenter->getDisplay();
}

void ControlPanel::setLoopStart(int sampleIndex)
{
    if (loopPresenter != nullptr)
        loopPresenter->setLoopStartFromSample(sampleIndex);
}

void ControlPanel::setLoopEnd(int sampleIndex)
{
    if (loopPresenter != nullptr)
        loopPresenter->setLoopEndFromSample(sampleIndex);
}

juce::String ControlPanel::formatTime(double seconds) const
{
    return owner.formatTime(seconds);
}

const juce::LookAndFeel& ControlPanel::getLookAndFeel() const
{
    return modernLF;
}

AppEnums::PlacementMode ControlPanel::getPlacementMode() const
{
    return mouseHandler->getCurrentPlacementMode();
}

void ControlPanel::mouseMove(const juce::MouseEvent& event)
{
    mouseHandler->mouseMove(event);
}

void ControlPanel::mouseDown(const juce::MouseEvent& event)
{
    mouseHandler->mouseDown(event);
}

void ControlPanel::mouseDrag(const juce::MouseEvent& event)
{
    mouseHandler->mouseDrag(event);
}

void ControlPanel::mouseUp(const juce::MouseEvent& event)
{
    mouseHandler->mouseUp(event);
}

void ControlPanel::mouseExit(const juce::MouseEvent& event)
{
    mouseHandler->mouseExit(event);
}

void ControlPanel::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    mouseHandler->mouseWheelMove(event, wheel);
}
