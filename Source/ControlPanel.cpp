#include "ControlPanel.h"
#include "MainComponent.h" // Full header required for MainComponent access (e.g., getAudioPlayer)
#include "AudioPlayer.h"    // Required for AudioPlayer types in public methods
#include "Config.h"
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
ControlPanel::ControlPanel(MainComponent& ownerComponent) : owner(ownerComponent),
                                                         modernLF(),
                                                         silenceDetector(std::make_unique<SilenceDetector>(*this))
{
    initialiseLookAndFeel();
    initialiseButtons();
    initialiseLoopButtons();
    initialiseClearButtons();
    initialiseLoopEditors();
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
    modernLF.setBaseOffColor(Config::buttonBaseColour);
    modernLF.setBaseOnColor(Config::buttonOnColour);
    modernLF.setTextColor(Config::buttonTextColour);
}

/**
 * @brief Orchestrates the initialization of all individual UI buttons.
 *
 * This method calls a series of specialized `initialise` methods to create,
 * configure, and add to the visible hierarchy all the `juce::TextButton`
 * and custom `LoopButton` instances used in the control panel. It also
 * initializes the `statsDisplay` TextEditor here due to its close relation
 * to button states.
 */
void ControlPanel::initialiseButtons()
{
    initialiseOpenButton();
    initialisePlayStopButton();
    initialiseModeButton();
    initialiseChannelViewButton();
    initialiseQualityButton();
    initialiseExitButton();
    initialiseStatsButton();
    initialiseLoopButton();
    initialiseAutoplayButton();
    initialiseAutoCutInButton();
    initialiseAutoCutOutButton();
    initialiseCutButton();

    // Stats Display TextEditor setup
    addAndMakeVisible (statsDisplay);
    statsDisplay.setReadOnly (true); // User cannot type in this display
    statsDisplay.setMultiLine (true); // Allows multiple lines of text
    statsDisplay.setWantsKeyboardFocus (false); // Does not grab keyboard focus
    statsDisplay.setColour (juce::TextEditor::backgroundColourId, Config::statsDisplayBackgroundColour);
    statsDisplay.setColour (juce::TextEditor::textColourId, Config::statsDisplayTextColour);
    statsDisplay.setVisible (false); // Initially hidden
}

/**
 * @brief Initializes the "Open Directory" button.
 *
 * Sets the button's text from `Config.h` and assigns an `onClick` lambda
 * that calls `owner.openButtonClicked()`, delegating the file opening
 * functionality to the `MainComponent`.
 */
void ControlPanel::initialiseOpenButton()
{
    addAndMakeVisible(openButton);
    openButton.setButtonText(Config::openButtonText);
    openButton.onClick = [this] { owner.openButtonClicked(); };
}

/**
 * @brief Initializes the "Play/Stop" button.
 *
 * Sets the initial button text to "Play" (from `Config.h`), and assigns an
 * `onClick` lambda that toggles the `AudioPlayer`'s playback state.
 * It's initially disabled until an audio file is loaded.
 */
void ControlPanel::initialisePlayStopButton()
{
    addAndMakeVisible(playStopButton);
    playStopButton.setButtonText(Config::playButtonText); // Initialize text
    playStopButton.onClick = [this] { owner.getAudioPlayer()->togglePlayStop(); };
    playStopButton.setEnabled(false); // Disabled until an audio file is loaded
}

/**
 * @brief Initializes the "View Mode" button.
 *
 * Configures the button to toggle between `AppEnums::ViewMode::Classic` and
 * `AppEnums::ViewMode::Overlay`. The `onClick` lambda updates the internal
 * `currentMode` state, changes the button's text accordingly, and triggers
 * a `resized()` and `repaint()` to update the UI layout and appearance.
 */
void ControlPanel::initialiseModeButton()
{
    addAndMakeVisible (modeButton);
    modeButton.setButtonText (Config::viewModeClassicText);
    modeButton.setClickingTogglesState (true); // Makes it a toggle button
    modeButton.onClick = [this] {
        currentMode = modeButton.getToggleState() ? AppEnums::ViewMode::Overlay : AppEnums::ViewMode::Classic;
        modeButton.setButtonText (currentMode == AppEnums::ViewMode::Classic ? Config::viewModeClassicText : Config::viewModeOverlayText);
        resized(); // Re-layout components based on new view mode
        repaint(); // Redraw to reflect changes
    };
}

/**
 * @brief Initializes the "Channel View" button.
 *
 * Configures the button to toggle between `AppEnums::ChannelViewMode::Mono`
 * and `AppEnums::ChannelViewMode::Stereo`. The `onClick` lambda updates the
 * internal `currentChannelViewMode` state, changes the button's text, and
 * triggers a `repaint()` to update the waveform display.
 */
void ControlPanel::initialiseChannelViewButton()
{
    addAndMakeVisible(channelViewButton);
    channelViewButton.setButtonText(Config::channelViewMonoText);
    channelViewButton.setClickingTogglesState(true); // Makes it a toggle button
    channelViewButton.onClick = [this] {
        currentChannelViewMode = channelViewButton.getToggleState() ? AppEnums::ChannelViewMode::Stereo : AppEnums::ChannelViewMode::Mono;
        channelViewButton.setButtonText(currentChannelViewMode == AppEnums::ChannelViewMode::Mono ? Config::channelViewMonoText : Config::channelViewStereoText);
        repaint(); // Redraw waveform to reflect channel view change
    };
}

/**
 * @brief Initializes the "Quality" button.
 *
 * Configures the button to cycle through `AppEnums::ThumbnailQuality` settings
 * (High, Medium, Low) on each click. The `onClick` lambda updates the internal
 * `currentQuality` state, calls `updateQualityButtonText()` to update the
 * button's label, and triggers a `repaint()` to redraw the waveform with the
 * new quality setting.
 */
void ControlPanel::initialiseQualityButton()
{
    addAndMakeVisible(qualityButton);
    qualityButton.setButtonText(Config::qualityButtonText);
    qualityButton.onClick = [this] {
        if (currentQuality == AppEnums::ThumbnailQuality::High)
            currentQuality = AppEnums::ThumbnailQuality::Medium;
        else if (currentQuality == AppEnums::ThumbnailQuality::Medium)
            currentQuality = AppEnums::ThumbnailQuality::Low;
        else // currentQuality == AppEnums::ThumbnailQuality::Low
            currentQuality = AppEnums::ThumbnailQuality::High;
        updateQualityButtonText(); // Update button text to reflect new quality
        repaint(); // Redraw waveform with new quality
    };
    updateQualityButtonText(); // Set initial text
}

/**
 * @brief Initializes the "Exit" button.
 *
 * Sets the button's text and a distinct color from `Config.h`. The `onClick`
 * lambda requests the JUCE application to quit, ensuring a graceful shutdown.
 */
void ControlPanel::initialiseExitButton()
{
    addAndMakeVisible(exitButton);
    exitButton.setButtonText(Config::exitButtonText);
    exitButton.setColour(juce::TextButton::buttonColourId, Config::exitButtonColor); // Distinct color for exit
    exitButton.onClick = [] {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    };
}

/**
 * @brief Initializes the "Stats" button.
 *
 * Configures the button to toggle the visibility of the `statsDisplay` `TextEditor`.
 * The `onClick` lambda updates the `showStats` flag, triggers a `resized()`
 * to adjust layout, and calls `updateComponentStates()` to update visibility.
 */
void ControlPanel::initialiseStatsButton()
{
    addAndMakeVisible(statsButton);
    statsButton.setButtonText(Config::statsButtonText);
    statsButton.setClickingTogglesState(true); // Makes it a toggle button
    statsButton.onClick = [this] {
        showStats = statsButton.getToggleState(); // Update internal state
        resized(); // Re-layout components
        updateComponentStates(); // Update visibility based on new state
    };
}

/**
 * @brief Initializes the "Loop" button.
 *
 * Configures the button to toggle the global looping behavior of the `AudioPlayer`.
 * The `onClick` lambda updates the internal `shouldLoop` flag and then informs
 * the `AudioPlayer` about the new looping state.
 */
void ControlPanel::initialiseLoopButton()
{
    addAndMakeVisible(loopButton);
    loopButton.setButtonText(Config::loopButtonText);
    loopButton.setClickingTogglesState(true); // Makes it a toggle button
    loopButton.onClick = [this] {
        shouldLoop = loopButton.getToggleState(); // Update internal state
        owner.getAudioPlayer()->setLooping(shouldLoop); // Inform AudioPlayer
    };
}

/**
 * @brief Initializes the "Autoplay" button.
 *
 * Configures the button to toggle the `m_shouldAutoplay` flag.
 * The button's initial state is set from `m_shouldAutoplay`, and the `onClick`
 * lambda simply updates this flag.
 */
void ControlPanel::initialiseAutoplayButton()
{
    addAndMakeVisible(autoplayButton);
    autoplayButton.setButtonText(Config::autoplayButtonText);
    autoplayButton.setClickingTogglesState(true); // Makes it a toggle button
    autoplayButton.setToggleState(m_shouldAutoplay, juce::dontSendNotification); // Set initial state
    autoplayButton.onClick = [this] {
        m_shouldAutoplay = autoplayButton.getToggleState(); // Update internal flag
    };
}

/**
 * @brief Initializes the Auto Cut In button.
 *
 * Configures the button to toggle the `silenceDetector`'s "auto-cut in" state.
 * Upon toggling ON, it immediately triggers `detectInSilence()` if an audio file
 * is loaded, ensuring automatic loop-in calculation at activation. This provides
 * an "apply on toggle" behavior. `updateComponentStates()` is called to reflect
 * changes in enabled/disabled status of related UI elements.
 */
void ControlPanel::initialiseAutoCutInButton()
{
    addAndMakeVisible(autoCutInButton);
    autoCutInButton.setButtonText(Config::autoCutInButtonText);
    autoCutInButton.setClickingTogglesState(true); // Make it a toggle button
    autoCutInButton.onClick = [this] {
        const bool isAutoCutActive = autoCutInButton.getToggleState();
        silenceDetector->setIsAutoCutInActive(isAutoCutActive); // Inform SilenceDetector
        updateComponentStates(); // Update related component states
        // Why: If auto-cut in is activated and an audio file is loaded,
        // immediately run the detection to set the loop-in point.
        // This provides instant feedback and functionality to the user.
        if (isAutoCutActive && owner.getAudioPlayer()->getThumbnail().getTotalLength() > 0.0) {
            silenceDetector->detectInSilence();
        }
    };
}

/**
 * @brief Initializes the Auto Cut Out button.
 *
 * Configures the button to toggle the `silenceDetector`'s "auto-cut out" state.
 * Similar to `initialiseAutoCutInButton`, toggling ON immediately triggers
 * `detectOutSilence()` if audio is loaded, providing instant feedback.
 * `updateComponentStates()` is called to adjust related UI element states.
 */
void ControlPanel::initialiseAutoCutOutButton()
{
    addAndMakeVisible(autoCutOutButton);
    autoCutOutButton.setButtonText(Config::autoCutOutButtonText);
    autoCutOutButton.setClickingTogglesState(true); // Make it a toggle button
    autoCutOutButton.onClick = [this] {
        const bool isAutoCutActive = autoCutOutButton.getToggleState();
        silenceDetector->setIsAutoCutOutActive(isAutoCutActive); // Inform SilenceDetector
        updateComponentStates(); // Update related component states
        // Why: If auto-cut out is activated and an audio file is loaded,
        // immediately run the detection to set the loop-out point.
        // This provides instant feedback and functionality to the user.
        if (isAutoCutActive && owner.getAudioPlayer()->getThumbnail().getTotalLength() > 0.0) {
            silenceDetector->detectOutSilence();
        }
    };
}

/**
 * @brief Initializes the "Cut" button.
 *
 * Configures the button to toggle the global `m_isCutModeActive` flag, which
 * controls the visibility and functionality of loop-related controls and visualisations.
 * If "Cut Mode" is activated while playback is active and the current position
 * is outside the defined loop range, the playback position is immediately
 * jumped to the loop-in point. This ensures that engaging "Cut Mode" instantly
 * enforces playback within the specified boundaries.
 */
void ControlPanel::initialiseCutButton()
{
    addAndMakeVisible(cutButton);
    cutButton.setButtonText(Config::cutButtonText);
    cutButton.setClickingTogglesState(true);
    cutButton.setToggleState(m_isCutModeActive, juce::dontSendNotification); // Set initial state
    cutButton.onClick = [this] {
        m_isCutModeActive = cutButton.getToggleState(); // Update internal flag
        updateComponentStates(); // Adjust visibility/enabled states of related controls
        // Why: If Cut Mode is activated while the audio is playing outside the defined loop range,
        // the playback position should immediately jump to the loop-in point.
        // This ensures playback adheres to the cut boundaries from the moment the mode is engaged.
        if (m_isCutModeActive && owner.getAudioPlayer()->isPlaying())
        {
            double currentPosition = owner.getAudioPlayer()->getTransportSource().getCurrentPosition();
            double loopIn = loopInPosition;
            double loopOut = loopOutPosition;

            // Ensure valid loop range before checking current position against it
            if (loopOut > loopIn)
            {
                if (currentPosition < loopIn || currentPosition >= loopOut)
                {
                    owner.getAudioPlayer()->getTransportSource().setPosition(loopIn);
                }
            }
        }
    };
}

/**
 * @brief Initializes the custom `LoopButton` instances for setting loop in/out points.
 *
 * This method sets up `loopInButton` and `loopOutButton`, which are custom
 * `TextButton` derivatives allowing for different actions on left and right clicks.
 * - **Left Click:** Directly sets the corresponding loop point to the current
 *   playback position. It also disables automatic detection for that loop point.
 * - **Right Click:** Enters a `PlacementMode`, where the user can click on the
 *   waveform to precisely set the loop point.
 * The button colors and associated `SilenceDetector` states are updated accordingly.
 */
void ControlPanel::initialiseLoopButtons()
{
    addAndMakeVisible(loopInButton);
    loopInButton.setButtonText(Config::loopInButtonText);
    loopInButton.onLeftClick = [this] {
        loopInPosition = owner.getAudioPlayer()->getTransportSource().getCurrentPosition();
        ensureLoopOrder();
        updateLoopButtonColors();
        silenceDetector->setIsAutoCutInActive(false); // User manually set loop in, so auto-cut is no longer active
        repaint();
    };
    loopInButton.onRightClick = [this] {
        currentPlacementMode = AppEnums::PlacementMode::LoopIn;
        updateLoopButtonColors();
        silenceDetector->setIsAutoCutInActive(false); // User manually set loop in, so auto-cut is no longer active
        repaint();
    };

    addAndMakeVisible(loopOutButton);
    loopOutButton.setButtonText(Config::loopOutButtonText);
    loopOutButton.onLeftClick = [this] {
        loopOutPosition = owner.getAudioPlayer()->getTransportSource().getCurrentPosition();
        ensureLoopOrder();
        updateLoopButtonColors();
        repaint();
    };
    loopOutButton.onRightClick = [this] {
        currentPlacementMode = AppEnums::PlacementMode::LoopOut;
        updateLoopButtonColors();
        repaint();
    };
}

/**
 * @brief Initializes the "Clear Loop In" and "Clear Loop Out" buttons.
 *
 * These buttons provide a quick way to reset the loop points.
 * - `clearLoopInButton` resets `loopInPosition` to the beginning of the audio (0.0).
 * - `clearLoopOutButton` resets `loopOutPosition` to the total length of the audio.
 * Both actions also ensure loop order, update button colors, labels, and disable
 * any active auto-cut feature for the respective loop point.
 */
void ControlPanel::initialiseClearButtons()
{
    addAndMakeVisible(clearLoopInButton);
    clearLoopInButton.setButtonText(Config::clearButtonText);
    clearLoopInButton.setColour(juce::TextButton::buttonColourId, Config::clearButtonColor);
    clearLoopInButton.onClick = [this] {
        loopInPosition = 0.0;
        ensureLoopOrder();
        updateLoopButtonColors();
        updateLoopLabels();
        silenceDetector->setIsAutoCutInActive(false); // User manually cleared loop in, so auto-cut is no longer active
        repaint();
    };

    addAndMakeVisible(clearLoopOutButton);
    clearLoopOutButton.setButtonText(Config::clearButtonText);
    clearLoopOutButton.setColour(juce::TextButton::buttonColourId, Config::clearButtonColor);
    clearLoopOutButton.onClick = [this] {
        loopOutPosition = owner.getAudioPlayer()->getThumbnail().getTotalLength();
        ensureLoopOrder();
        updateLoopButtonColors();
        updateLoopLabels();
        repaint();
    };
}

/**
 * @brief Initializes the `juce::TextEditor` instances for displaying and editing loop points and silence thresholds.
 *
 * This method sets up `loopInEditor`, `loopOutEditor` for numerical input
 * of loop start/end times, and also integrates the TextEditors managed by the
 * `silenceDetector` (`inSilenceThresholdEditor`, `outSilenceThresholdEditor`).
 * All editors are configured with consistent styling (read-only state, justification,
 * colors, font), keyboard focus behavior, and listeners (`this` for loop editors,
 * `silenceDetector` for threshold editors) to process user input.
 */
void ControlPanel::initialiseLoopEditors()
{
    addAndMakeVisible(loopInEditor);
    loopInEditor.setReadOnly(false); // Allows user to type
    loopInEditor.setJustification(juce::Justification::centred);
    loopInEditor.setColour(juce::TextEditor::backgroundColourId, Config::textEditorBackgroundColour);
    loopInEditor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
    loopInEditor.setFont(juce::Font(juce::FontOptions(Config::playbackTextSize)));
    loopInEditor.setMultiLine(false); // Single line input
    loopInEditor.setReturnKeyStartsNewLine(false); // Enter key applies value
    loopInEditor.addListener(this); // ControlPanel listens for changes
    loopInEditor.setWantsKeyboardFocus(true); // Can receive keyboard focus

    addAndMakeVisible(loopOutEditor);
    loopOutEditor.setReadOnly(false); // Allows user to type
    loopOutEditor.setJustification(juce::Justification::centred);
    loopOutEditor.setColour(juce::TextEditor::backgroundColourId, Config::textEditorBackgroundColour);
    loopOutEditor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
    loopOutEditor.setFont(juce::Font(juce::FontOptions(Config::playbackTextSize)));
    loopOutEditor.setMultiLine(false); // Single line input
    loopOutEditor.setReturnKeyStartsNewLine(false); // Enter key applies value
    loopOutEditor.addListener(this); // ControlPanel listens for changes
    loopOutEditor.setWantsKeyboardFocus(true); // Can receive keyboard focus

    // Add TextEditors managed by SilenceDetector
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
void ControlPanel::finaliseSetup()
{
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
    DBG("ControlPanel::resized() - START");
    auto bounds = getLocalBounds();
    int rowHeight = Config::buttonHeight + Config::windowBorderMargins * 2;

    layoutTopRowButtons(bounds, rowHeight);
    layoutLoopAndCutControls(bounds, rowHeight);
    layoutBottomRowAndTextDisplay(bounds, rowHeight);
    layoutWaveformAndStats(bounds);
    DBG("ControlPanel::resized() - END");
}

void ControlPanel::paint(juce::Graphics& g)
{
    g.fillAll (Config::mainBackgroundColor);
    AudioPlayer* audioPlayer = owner.getAudioPlayer();

    // Waveform drawing
    if (audioPlayer->getThumbnail().getNumChannels() > 0)
    {
        int pixelsPerSample = 1;
        if (currentQuality == AppEnums::ThumbnailQuality::Low)
                pixelsPerSample = 4;
            else if (currentQuality == AppEnums::ThumbnailQuality::Medium)
                pixelsPerSample = 2;
        
            if (currentChannelViewMode == AppEnums::ChannelViewMode::Mono || audioPlayer->getThumbnail().getNumChannels() == 1)        {
            g.setColour (Config::waveformColor);
            if (pixelsPerSample > 1)
                drawReducedQualityWaveform(g, 0, pixelsPerSample);
            else
                audioPlayer->getThumbnail().drawChannel (g, waveformBounds, 0.0, audioPlayer->getThumbnail().getTotalLength(), 0, 1.0f);
        }
        else // Stereo
        {
            g.setColour (Config::waveformColor);
            if (pixelsPerSample > 1)
            {
                for (int ch = 0; ch < audioPlayer->getThumbnail().getNumChannels(); ++ch)
                    drawReducedQualityWaveform(g, ch, pixelsPerSample);
            }
            else
            {
                audioPlayer->getThumbnail().drawChannels (g, waveformBounds, 0.0, audioPlayer->getThumbnail().getTotalLength(), 1.0f);
            }
        }
    } // End of waveform drawing

    auto audioLength = (float)audioPlayer->getThumbnail().getTotalLength();

    if (audioLength > 0.0) // Main block for drawing elements that depend on audio length
    {
        if (m_isCutModeActive) 
        {
            auto drawThresholdVisualisation = [&](juce::Graphics& g_ref, double loopPos, float threshold)
            {
                if (audioLength <= 0.0) return; 

                float normalisedThreshold = threshold; 
                float centerY = (float)waveformBounds.getCentreY();
                float halfHeight = (float)waveformBounds.getHeight() / 2.0f;

                float topThresholdY = centerY - (normalisedThreshold * halfHeight);
                float bottomThresholdY = centerY + (normalisedThreshold * halfHeight);

                topThresholdY = juce::jlimit((float)waveformBounds.getY(), (float)waveformBounds.getBottom(), topThresholdY);
                bottomThresholdY = juce::jlimit((float)waveformBounds.getY(), (float)waveformBounds.getBottom(), bottomThresholdY);

                float xPos = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (loopPos / audioLength);
                float halfThresholdLineWidth = Config::thresholdLineWidth / 2.0f;
                float lineStartX = xPos - halfThresholdLineWidth;
                float lineEndX = xPos + halfThresholdLineWidth;

                lineStartX = juce::jmax(lineStartX, (float)waveformBounds.getX());
                lineEndX = juce::jmin(lineEndX, (float)waveformBounds.getRight());
                float currentLineWidth = lineEndX - lineStartX;

                juce::Colour lineColor = Config::thresholdLineColor;
                juce::Colour regionColor = Config::thresholdRegionColor;

                g_ref.setColour(regionColor);
                g_ref.fillRect(lineStartX, topThresholdY, currentLineWidth, bottomThresholdY - topThresholdY);

                if (m_isCutModeActive) { // Glow if cut mode is active
                    juce::Colour glowColor = lineColor.withAlpha(lineColor.getFloatAlpha() * glowAlpha);
                    g_ref.setColour(glowColor);
                    g_ref.fillRect(lineStartX, topThresholdY - (Config::thresholdGlowThickness / 2.0f - 0.5f), currentLineWidth, Config::thresholdGlowThickness);
                    g_ref.fillRect(lineStartX, bottomThresholdY - (Config::thresholdGlowThickness / 2.0f - 0.5f), currentLineWidth, Config::thresholdGlowThickness);
                }

                g_ref.setColour(lineColor);
                g_ref.drawHorizontalLine((int)topThresholdY, lineStartX, lineEndX);
                g_ref.drawHorizontalLine((int)bottomThresholdY, lineStartX, lineEndX);
            };

            drawThresholdVisualisation(g, loopInPosition, silenceDetector->getCurrentInSilenceThreshold());
            drawThresholdVisualisation(g, loopOutPosition, silenceDetector->getCurrentOutSilenceThreshold());

            auto actualIn = juce::jmin(loopInPosition, loopOutPosition);
            auto actualOut = juce::jmax(loopInPosition, loopOutPosition);
            auto inX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (actualIn / audioLength);
            auto outX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (actualOut / audioLength);
            const float fadeLength = waveformBounds.getWidth() * Config::waveBoxHaze;

            // Draw region to the left of loopIn with fade
            juce::Rectangle<float> leftRegion((float)waveformBounds.getX(), (float)waveformBounds.getY(), inX - (float)waveformBounds.getX(), (float)waveformBounds.getHeight());
            if (leftRegion.getWidth() > 0)
            {
                juce::ColourGradient leftFadeGradient(Config::loopRegionColor, inX, leftRegion.getCentreY(),
                                                      Config::hazyWaveBoxFadeColor, inX - fadeLength, leftRegion.getCentreY(), false);
                g.setGradientFill(leftFadeGradient);
                g.fillRect(leftRegion);
            }
            
            // Draw region to the right of loopOut with fade
            juce::Rectangle<float> rightRegion(outX, (float)waveformBounds.getY(), (float)waveformBounds.getRight() - outX, (float)waveformBounds.getHeight());
            if (rightRegion.getWidth() > 0)
            {
                juce::ColourGradient rightFadeGradient(Config::hazyWaveBoxFadeColor, outX + fadeLength, rightRegion.getCentreY(),
                                                       Config::loopRegionColor, outX, rightRegion.getCentreY(), false);
                g.setGradientFill(rightFadeGradient);
                g.fillRect(rightRegion);
            }

            juce::Colour glowColor = Config::loopLineColor.withAlpha(Config::loopLineColor.getFloatAlpha() * (1.0f - glowAlpha));
            g.setColour(glowColor);
            g.fillRect(inX - (Config::loopLineGlowThickness / 2.0f - 0.5f), (float)waveformBounds.getY(), Config::loopLineGlowThickness, (float)waveformBounds.getHeight());
            g.fillRect(outX - (Config::loopLineGlowThickness / 2.0f - 0.5f), (float)waveformBounds.getY(), Config::loopLineGlowThickness, (float)waveformBounds.getHeight());

            // Draw horizontal lines connecting loop in/out markers at top and bottom
            g.setColour(Config::loopLineColor); // Use the same color as vertical lines
            g.drawHorizontalLine(waveformBounds.getY(), (int)inX, (int)outX); // Top line
            g.drawHorizontalLine(waveformBounds.getBottom() - 1, (int)inX, (int)outX); // Bottom line (offset by 1 to be inside bounds)
        }

        // Playback Cursor
        auto drawPosition = (float)audioPlayer->getTransportSource().getCurrentPosition();
        auto x = (drawPosition / audioLength) * (float)waveformBounds.getWidth() + (float)waveformBounds.getX();
        if (audioPlayer->isPlaying())
        {
            juce::ColourGradient gradient (Config::playbackCursorGlowColorStart, (float)x - 10.0f, (float)waveformBounds.getCentreY(), Config::playbackCursorGlowColorEnd, (float)x, (float)waveformBounds.getCentreY(), false );
            g.setGradientFill (gradient);
            g.fillRect (juce::Rectangle<float>((int)x - 10, (float)waveformBounds.getY(), 10, (float)waveformBounds.getHeight()));
        }
        else
        {
            juce::ColourGradient glowGradient;
            glowGradient.addColour (0.0, Config::playbackCursorGlowColorStart);
            glowGradient.addColour (0.5, Config::playbackCursorGlowColorEnd);
            glowGradient.addColour (1.0, Config::playbackCursorColor.withAlpha(0.0f));
            glowGradient.point1 = { (float)x - 5.0f, (float)waveformBounds.getCentreY() };
            glowGradient.point2 = { (float)x + 5.0f, (float)waveformBounds.getCentreY() };
            g.setGradientFill (glowGradient);
            g.fillRect (juce::Rectangle<float>((int)x - 5, (float)waveformBounds.getY(), 10, (float)waveformBounds.getHeight()));
        }
        g.setColour (Config::playbackCursorColor);
        g.drawVerticalLine ((int)x, (float)waveformBounds.getY(), (float)waveformBounds.getBottom());
    } 

        if (mouseCursorX != -1)
        {
            juce::Colour currentLineColor;
            juce::Colour currentHighlightColor;
            juce::Colour currentGlowColor;
            float currentGlowThickness;
    
            // Why: Change mouse cursor color and add glow/shadow when in loop placement mode
            // to provide visual feedback that the mouse is "armed" to set a loop point.
            if (currentPlacementMode == AppEnums::PlacementMode::LoopIn || currentPlacementMode == AppEnums::PlacementMode::LoopOut)
            {
                currentLineColor = Config::placementModeCursorColor;
                currentHighlightColor = Config::placementModeCursorColor.withAlpha(0.4f);
                currentGlowColor = Config::placementModeGlowColor;
                currentGlowThickness = Config::placementModeGlowThickness;
    
                // Draw glow/shadow for the vertical line
                g.setColour(currentGlowColor.withAlpha(0.3f)); // Semi-transparent for shadow effect
                g.fillRect(mouseCursorX - (int)(currentGlowThickness / 2) - 1, waveformBounds.getY(), (int)currentGlowThickness + 2, waveformBounds.getHeight());
                // Draw glow/shadow for the horizontal line
                g.fillRect(waveformBounds.getX(), mouseCursorY - (int)(currentGlowThickness / 2) - 1, waveformBounds.getWidth(), (int)currentGlowThickness + 2);
    
            }
            else
            {
                currentLineColor = Config::mouseCursorLineColor;
                currentHighlightColor = Config::mouseCursorHighlightColor;
                currentGlowColor = Config::mouseAmplitudeGlowColor; // Reusing for amplitude glow, not general cursor glow
                currentGlowThickness = 0.0f; // No extra glow for default cursor
            }
    
            g.setColour (currentHighlightColor);
            g.fillRect (mouseCursorX - 2, waveformBounds.getY(), 5, waveformBounds.getHeight());
            g.fillRect (waveformBounds.getX(), mouseCursorY - 2, waveformBounds.getWidth(), 5);
    
            if (audioLength > 0.0)
            {
                float amplitude = 0.0f;
                if (audioPlayer->getThumbnail().getNumChannels() > 0)
                {
                    auto* reader = audioPlayer->getAudioFormatReader();
                    double sampleRate = (reader != nullptr) ? reader->sampleRate : 0.0;
                    if (sampleRate > 0)
                    {
                        float minVal, maxVal;
                        audioPlayer->getThumbnail().getApproximateMinMax(mouseCursorTime, mouseCursorTime + (1.0 / sampleRate), 0, minVal, maxVal);
                        amplitude = juce::jmax(std::abs(minVal), std::abs(maxVal));
                    }
                }
    
                            // Draw amplitude line
                            float centerY = (float)waveformBounds.getCentreY();
                            float amplitudeY = centerY - (amplitude * waveformBounds.getHeight() * 0.5f);
                            float bottomAmplitudeY = centerY + (amplitude * waveformBounds.getHeight() * 0.5f); // The bottom amplitude line Y position
                            
                            juce::ColourGradient amplitudeGlowGradient(Config::mouseAmplitudeGlowColor.withAlpha(0.0f), (float)mouseCursorX, amplitudeY,
                                                                      Config::mouseAmplitudeGlowColor.withAlpha(0.7f), (float)mouseCursorX, centerY, true);
                            g.setGradientFill(amplitudeGlowGradient);
                            g.fillRect(juce::Rectangle<float>((float)mouseCursorX - Config::mouseAmplitudeGlowThickness / 2, amplitudeY, Config::mouseAmplitudeGlowThickness, centerY - amplitudeY));
                            g.fillRect(juce::Rectangle<float>((float)mouseCursorX - Config::mouseAmplitudeGlowThickness / 2, centerY, Config::mouseAmplitudeGlowThickness, centerY - amplitudeY));
                
                            g.setColour(Config::mouseAmplitudeLineColor);
                            g.drawVerticalLine((float)mouseCursorX, amplitudeY, bottomAmplitudeY);
                            
                            // Why: Draw two short horizontal lines at the top and bottom of the amplitude visualization
                            // to clearly indicate the min/max amplitude range at the mouse cursor's X position.
                            float halfLineLength = Config::mouseAmplitudeLineLength / 2.0f;
                            g.drawHorizontalLine(amplitudeY, (float)mouseCursorX - halfLineLength, (float)mouseCursorX + halfLineLength);
                            g.drawHorizontalLine(bottomAmplitudeY, (float)mouseCursorX - halfLineLength, (float)mouseCursorX + halfLineLength);
                
                            // Draw amplitude text
                            juce::String amplitudeText = juce::String(amplitude, 2); // Positive amplitude
                            juce::String negativeAmplitudeText = juce::String(-amplitude, 2); // Negative amplitude
                            g.setColour(Config::playbackTextColor);
                            g.setFont(Config::mouseCursorTextSize);
                            g.drawText(amplitudeText, mouseCursorX + 5, (int)amplitudeY - Config::mouseCursorTextSize, 100, Config::mouseCursorTextSize, juce::Justification::left, true);
                            // Why: Display the negative amplitude value to provide complete information about the peak-to-peak amplitude.
                            g.drawText(negativeAmplitudeText, mouseCursorX + 5, (int)bottomAmplitudeY, 100, Config::mouseCursorTextSize, juce::Justification::left, true);
                
                
                            // Draw time text
                            juce::String timeText = formatTime(mouseCursorTime); // Use ControlPanel's formatTime
                            g.drawText(timeText, mouseCursorX + 5, mouseCursorY + 5, 100, Config::mouseCursorTextSize, juce::Justification::left, true);
            }
            
                    // Draw the main cursor lines on top
                    g.setColour (currentLineColor);
                    g.drawVerticalLine (mouseCursorX, (float)waveformBounds.getY(), (float)waveformBounds.getBottom());
                    g.drawHorizontalLine (mouseCursorY, (float)waveformBounds.getX(), (float)waveformBounds.getRight());        }
    if (audioLength > 0.0)
    {
        double currentTime = audioPlayer->getTransportSource().getCurrentPosition();
        double totalTime = audioPlayer->getThumbnail().getTotalLength();
        double remainingTime = totalTime - currentTime;

        juce::String currentTimeStr = formatTime(currentTime); // Use ControlPanel's formatTime
        juce::String remainingTimeStr = formatTime(remainingTime); // Use ControlPanel's formatTime

        int textY = bottomRowTopY - Config::playbackTimeTextOffsetY;

        g.setColour(Config::playbackTextColor);
        g.setFont(Config::playbackTextSize);

        g.drawText(currentTimeStr, playbackLeftTextX, textY, Config::playbackTextWidth, Config::playbackTextHeight, juce::Justification::left, false);
        g.drawText(totalTimeStaticStr, playbackCenterTextX, textY, Config::playbackTextWidth, 20, juce::Justification::centred, false);
        g.drawText(remainingTimeStr, playbackRightTextX, textY, Config::playbackTextWidth, 20, juce::Justification::right, false);
    }
}

void ControlPanel::updatePlayButtonText(bool isPlaying)
{
    playStopButton.setButtonText(isPlaying ? Config::stopButtonText : Config::playButtonText);
}

void ControlPanel::updateLoopLabels()
{
    loopInEditor.setText(formatTime(loopInPosition), juce::dontSendNotification); // Use ControlPanel's formatTime
    loopOutEditor.setText(formatTime(loopOutPosition), juce::dontSendNotification); // Use ControlPanel's formatTime
}

void ControlPanel::updateComponentStates()
{
    DBG("ControlPanel::updateComponentStates() - START");
    const bool enabled = owner.getAudioPlayer()->getThumbnail().getTotalLength() > 0.0;
    DBG("  - enabled (file loaded): " << (enabled ? "true" : "false"));
    DBG("  - m_isCutModeActive: " << (m_isCutModeActive ? "true" : "false")); // Use m_isCutModeActive
    updateGeneralButtonStates(enabled);
    updateCutModeControlStates(m_isCutModeActive, enabled); // Use SilenceDetector's states
    DBG("ControlPanel::updateComponentStates() - END");
}

/**
 * @brief Lays out the buttons for the top row of the control panel.
 * @param bounds The current bounds of the control panel.
 * @param rowHeight The calculated height for each button row.
 */
void ControlPanel::layoutTopRowButtons(juce::Rectangle<int>& bounds, int rowHeight)
{
    auto topRow = bounds.removeFromTop(rowHeight).reduced(Config::windowBorderMargins);
    openButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
    playStopButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
    autoplayButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
    loopButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
    cutButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
    exitButton.setBounds(topRow.removeFromRight(Config::buttonWidth)); topRow.removeFromRight(Config::windowBorderMargins);
}

/**
 * @brief Lays out the loop and cut control elements.
 * @param bounds The current bounds of the control panel.
 * @param rowHeight The calculated height for each button row.
 */
void ControlPanel::layoutLoopAndCutControls(juce::Rectangle<int>& bounds, int rowHeight)
{
    auto loopRow = bounds.removeFromTop(rowHeight).reduced(Config::windowBorderMargins);
    loopInButton.setBounds(loopRow.removeFromLeft(Config::buttonWidth)); loopRow.removeFromLeft(Config::windowBorderMargins);
    loopInEditor.setBounds(loopRow.removeFromLeft(Config::loopTextWidth)); loopRow.removeFromLeft(Config::windowBorderMargins / 2);
    clearLoopInButton.setBounds(loopRow.removeFromLeft(Config::clearButtonWidth)); loopRow.removeFromLeft(Config::clearButtonMargin);
    loopRow.removeFromLeft(Config::windowBorderMargins * 2);
    loopOutButton.setBounds(loopRow.removeFromLeft(Config::buttonWidth)); loopRow.removeFromLeft(Config::windowBorderMargins);
    loopOutEditor.setBounds(loopRow.removeFromLeft(Config::loopTextWidth)); loopRow.removeFromLeft(Config::windowBorderMargins / 2);
    clearLoopOutButton.setBounds(loopRow.removeFromLeft(Config::clearButtonWidth)); loopRow.removeFromLeft(Config::clearButtonMargin);
    loopRow.removeFromLeft(Config::windowBorderMargins * 2);
    silenceDetector->getInSilenceThresholdEditor().setBounds(loopRow.removeFromLeft(Config::thresholdEditorWidth)); loopRow.removeFromLeft(Config::windowBorderMargins);
    autoCutInButton.setBounds(loopRow.removeFromLeft(Config::buttonWidth)); loopRow.removeFromLeft(Config::windowBorderMargins * 2);
    silenceDetector->getOutSilenceThresholdEditor().setBounds(loopRow.removeFromLeft(Config::thresholdEditorWidth)); loopRow.removeFromLeft(Config::windowBorderMargins);
    autoCutOutButton.setBounds(loopRow.removeFromLeft(Config::buttonWidth));
}

/**
 * @brief Lays out the buttons for the bottom row and the text display area.
 * @param bounds The current bounds of the control panel.
 * @param rowHeight The calculated height for each button row.
 */
void ControlPanel::layoutBottomRowAndTextDisplay(juce::Rectangle<int>& bounds, int rowHeight)
{
    auto bottomRow = bounds.removeFromBottom(rowHeight).reduced(Config::windowBorderMargins);
    bottomRowTopY = bottomRow.getY();
    contentAreaBounds = bounds.reduced(Config::windowBorderMargins);
    qualityButton.setBounds(bottomRow.removeFromRight(Config::buttonWidth)); bottomRow.removeFromRight(Config::windowBorderMargins);
    channelViewButton.setBounds(bottomRow.removeFromRight(Config::buttonWidth)); bottomRow.removeFromRight(Config::windowBorderMargins);
    statsButton.setBounds(bottomRow.removeFromRight(Config::buttonWidth)); bottomRow.removeFromRight(Config::windowBorderMargins);
    modeButton.setBounds(bottomRow.removeFromRight(Config::buttonWidth));

    playbackLeftTextX = getLocalBounds().getX() + Config::windowBorderMargins;
    playbackCenterTextX = (getLocalBounds().getWidth() / 2) - (Config::playbackTextWidth / 2);
    playbackRightTextX = getLocalBounds().getRight() - Config::windowBorderMargins - Config::playbackTextWidth;
}

void ControlPanel::layoutWaveformAndStats(juce::Rectangle<int>& bounds)
{
    if (currentMode == AppEnums::ViewMode::Overlay) {
        waveformBounds = getLocalBounds();
    } else {
        waveformBounds = bounds.reduced(Config::windowBorderMargins);
    }

    if (showStats) {
        statsBounds = contentAreaBounds.withHeight(100).reduced(10);
        statsDisplay.setBounds(statsBounds);
        statsDisplay.setVisible(true);
        statsDisplay.toFront(true);
    } else {
        statsDisplay.setVisible(false);
    }
}

void ControlPanel::updateGeneralButtonStates(bool enabled)
{
    DBG("ControlPanel::updateGeneralButtonStates() - START (parent enabled: " << (enabled ? "true" : "false") << ")");
    openButton.setEnabled(true); DBG("  - openButton enabled: " << (openButton.isEnabled() ? "true" : "false"));
    exitButton.setEnabled(true); DBG("  - exitButton enabled: " << (exitButton.isEnabled() ? "true" : "false"));
    loopButton.setEnabled(true); DBG("  - loopButton enabled: " << (loopButton.isEnabled() ? "true" : "false"));
    autoplayButton.setEnabled(true); DBG("  - autoplayButton enabled: " << (autoplayButton.isEnabled() ? "true" : "false"));
    cutButton.setEnabled(true); DBG("  - cutButton enabled: " << (cutButton.isEnabled() ? "true" : "false"));

    playStopButton.setEnabled(enabled); DBG("  - playStopButton enabled: " << (playStopButton.isEnabled() ? "true" : "false"));
    modeButton.setEnabled(enabled); DBG("  - modeButton enabled: " << (modeButton.isEnabled() ? "true" : "false"));
    statsButton.setEnabled(enabled); DBG("  - statsButton enabled: " << (statsButton.isEnabled() ? "true" : "false"));
    channelViewButton.setEnabled(enabled); DBG("  - channelViewButton enabled: " << (channelViewButton.isEnabled() ? "true" : "false"));
    qualityButton.setEnabled(enabled); DBG("  - qualityButton enabled: " << (qualityButton.isEnabled() ? "true" : "false"));
    statsDisplay.setEnabled(enabled); DBG("  - statsDisplay enabled: " << (statsDisplay.isEnabled() ? "true" : "false"));
    DBG("ControlPanel::updateGeneralButtonStates() - END");
}

void ControlPanel::updateCutModeControlStates(bool isCutModeActive, bool enabled)
{
    // The parameter isCutModeActive is now correct, as it's passed from updateComponentStates where m_isCutModeActive is used.
    DBG("ControlPanel::updateCutModeControlStates() - START (isCutModeActive parameter: " << (isCutModeActive ? "true" : "false") << ", parent enabled: " << (enabled ? "true" : "false") << ")");
    // Manual Loop In controls (now always enabled if cut mode is active)
    loopInButton.setEnabled(enabled && isCutModeActive && !silenceDetector->getIsAutoCutInActive()); DBG("  - loopInButton enabled: " << (loopInButton.isEnabled() ? "true" : "false"));
    loopInEditor.setEnabled(enabled && isCutModeActive && !silenceDetector->getIsAutoCutInActive()); DBG("  - loopInEditor enabled: " << (loopInEditor.isEnabled() ? "true" : "false"));
    clearLoopInButton.setEnabled(enabled && isCutModeActive && !silenceDetector->getIsAutoCutInActive()); DBG("  - clearLoopInButton enabled: " << (clearLoopInButton.isEnabled() ? "true" : "false"));

    // Manual Loop Out controls (now always enabled if cut mode is active)
    loopOutButton.setEnabled(enabled && isCutModeActive && !silenceDetector->getIsAutoCutOutActive()); DBG("  - loopOutButton enabled: " << (loopOutButton.isEnabled() ? "true" : "false"));
    loopOutEditor.setEnabled(enabled && isCutModeActive && !silenceDetector->getIsAutoCutOutActive()); DBG("  - loopOutEditor enabled: " << (loopOutEditor.isEnabled() ? "true" : "false"));
    clearLoopOutButton.setEnabled(enabled && isCutModeActive && !silenceDetector->getIsAutoCutOutActive()); DBG("  - clearLoopOutButton enabled: " << (clearLoopOutButton.isEnabled() ? "true" : "false"));

    // Auto Cut In/Out Buttons (now always enabled if cut mode is active)
    autoCutInButton.setEnabled(enabled && isCutModeActive); DBG("  - autoCutInButton enabled: " << (autoCutInButton.isEnabled() ? "true" : "false"));
    autoCutOutButton.setEnabled(enabled && isCutModeActive); DBG("  - autoCutOutOutton enabled: " << (autoCutOutButton.isEnabled() ? "true" : "false"));

    // Threshold Editors (now always enabled if cut mode is active)
    silenceDetector->getInSilenceThresholdEditor().setEnabled(true); DBG("  - inSilenceThresholdEditor enabled: true");
    silenceDetector->getOutSilenceThresholdEditor().setEnabled(true); DBG("  - outSilenceThresholdEditor enabled: true");

    // Visibility remains the same
    loopInButton.setVisible(isCutModeActive); DBG("  - loopInButton visible: " << (loopInButton.isVisible() ? "true" : "false"));
    loopOutButton.setVisible(isCutModeActive); DBG("  - loopOutButton visible: " << (loopOutButton.isVisible() ? "true" : "false"));
    loopInEditor.setVisible(isCutModeActive); DBG("  - loopInEditor visible: " << (loopInEditor.isVisible() ? "true" : "false"));
    loopOutEditor.setVisible(isCutModeActive); DBG("  - loopOutEditor visible: " << (loopOutEditor.isVisible() ? "true" : "false"));
    clearLoopInButton.setVisible(isCutModeActive); DBG("  - clearLoopInButton visible: " << (clearLoopInButton.isVisible() ? "true" : "false"));
    clearLoopOutButton.setVisible(isCutModeActive); DBG("  - clearLoopOutButton visible: " << (clearLoopOutButton.isEnabled() ? "true" : "false")); // Fixed typo: was isVisible()
    silenceDetector->getInSilenceThresholdEditor().setVisible(isCutModeActive); DBG("  - inSilenceThresholdEditor visible: " << (silenceDetector->getInSilenceThresholdEditor().isVisible() ? "true" : "false"));
    silenceDetector->getOutSilenceThresholdEditor().setVisible(isCutModeActive); DBG("  - outSilenceThresholdEditor visible: " << (silenceDetector->getOutSilenceThresholdEditor().isVisible() ? "true" : "false"));
    autoCutInButton.setVisible(isCutModeActive); DBG("  - autoCutInButton visible: " << (autoCutInButton.isVisible() ? "true" : "false"));
    autoCutOutButton.setVisible(isCutModeActive); DBG("  - autoCutOutButton visible: " << (autoCutOutButton.isVisible() ? "true" : "false"));
    DBG("ControlPanel::updateCutModeControlStates() - END");
}

void ControlPanel::updateQualityButtonText()
{
    if (currentQuality == AppEnums::ThumbnailQuality::High)
        qualityButton.setButtonText(Config::qualityHighText);
    else if (currentQuality == AppEnums::ThumbnailQuality::Medium)
        qualityButton.setButtonText(Config::qualityMediumText);
    else
        qualityButton.setButtonText(Config::qualityLowText);
}

void ControlPanel::drawReducedQualityWaveform(juce::Graphics& g, int channel, int pixelsPerSample)
{
    auto* audioPlayer = owner.getAudioPlayer();
    auto audioLength = audioPlayer->getThumbnail().getTotalLength();
    if (audioLength <= 0.0) return;

    auto width = waveformBounds.getWidth();
    auto height = waveformBounds.getHeight();
    auto centerY = waveformBounds.getCentreY();
    for (int x = 0; x < width; x += pixelsPerSample)
    {
        auto proportion = (double)x / (double)width;
        auto time = proportion * audioLength;
        float minVal, maxVal;
        audioPlayer->getThumbnail().getApproximateMinMax(time, time + (audioLength / width) * pixelsPerSample, channel, minVal, maxVal);
        auto topY = centerY - (maxVal * height * 0.5f);
        auto bottomY = centerY - (minVal * height * 0.5f);
        g.drawVerticalLine(waveformBounds.getX() + x, topY, bottomY);
    }
}

void ControlPanel::textEditorTextChanged (juce::TextEditor& editor) {
    DBG("Text Editor Text Changed.");
    // Handles loop editors
    DBG("  Loop Editor Text Changed: " << editor.getText());
    double totalLength = owner.getAudioPlayer()->getThumbnail().getTotalLength();
    double newPosition = parseTime(editor.getText());

    if (newPosition >= 0.0 && newPosition <= totalLength) {
        editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Valid and in range
    } else if (newPosition == -1.0) {
        editor.setColour(juce::TextEditor::textColourId, Config::textEditorErrorColor); // Completely invalid format
    } else {
        editor.setColour(juce::TextEditor::textColourId, Config::textEditorWarningColor); // Valid format but out of range
    }
}

void ControlPanel::textEditorReturnKeyPressed (juce::TextEditor& editor) {
    DBG("Text Editor Return Key Pressed.");
    if (&editor == &loopInEditor) {
        DBG("  Loop In Editor: Return Key Pressed");

        double newPosition = parseTime(editor.getText());
        if (newPosition >= 0.0 && newPosition <= owner.getAudioPlayer()->getThumbnail().getTotalLength()) {
            if (loopOutPosition > -1.0 && newPosition > loopOutPosition) {
                            editor.setText(formatTime(loopInPosition), juce::dontSendNotification);
                            editor.setColour(juce::TextEditor::textColourId, Config::textEditorWarningColor);
                        } else {
                            loopInPosition = newPosition;
                            DBG("    Loop In position set to: " << loopInPosition);
                            updateLoopButtonColors();
                            silenceDetector->setIsAutoCutInActive(false); // User manually set loop in, so auto-cut is no longer active
                            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
                            repaint();
                        }
                    } else {
                        editor.setText(formatTime(loopInPosition), juce::dontSendNotification);
                        editor.setColour(juce::TextEditor::textColourId, Config::textEditorErrorColor);
                    }
                } else if (&editor == &loopOutEditor) {
                    DBG("  Loop Out Editor: Return Key Pressed");

                    double newPosition = parseTime(editor.getText());
                    if (newPosition >= 0.0 && newPosition <= owner.getAudioPlayer()->getThumbnail().getTotalLength()) {
                        if (shouldLoop && owner.getAudioPlayer()->getTransportSource().getCurrentPosition() >= loopOutPosition)
                        {
                            owner.getAudioPlayer()->getTransportSource().setPosition(loopInPosition);
                        }
                        if (loopInPosition > -1.0 && newPosition < loopInPosition) {
                            editor.setText(formatTime(loopOutPosition), juce::dontSendNotification);
                            editor.setColour(juce::TextEditor::textColourId, Config::textEditorWarningColor);
                        } else {
                            loopOutPosition = newPosition;
                            DBG("    Loop Out position set to: " << loopOutPosition);
                            updateLoopButtonColors();
                            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
                            repaint();
                        }
                    } else {
                        editor.setText(formatTime(loopOutPosition), juce::dontSendNotification);
                        editor.setColour(juce::TextEditor::textColourId, Config::textEditorErrorColor);
                    }
                }
    editor.giveAwayKeyboardFocus();
}

void ControlPanel::textEditorEscapeKeyPressed (juce::TextEditor& editor) {
    DBG("Text Editor Escape Key Pressed.");
    if (&editor == &loopInEditor) {
        DBG("  Loop In Editor: Escape Key Pressed");
        editor.setText(formatTime(loopInPosition), juce::dontSendNotification);
    } else if (&editor == &loopOutEditor) {
        DBG("  Loop Out Editor: Escape Key Pressed");
        editor.setText(formatTime(loopOutPosition), juce::dontSendNotification);
    }
    editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
    editor.giveAwayKeyboardFocus();
}

void ControlPanel::textEditorFocusLost (juce::TextEditor& editor) {
    DBG("Text Editor Focus Lost.");
    if (&editor == &loopInEditor) {
        DBG("  Loop In Editor: Focus Lost");

        double newPosition = parseTime(editor.getText());
        if (newPosition >= 0.0 && newPosition <= owner.getAudioPlayer()->getThumbnail().getTotalLength()) {
            if (loopOutPosition > -1.0 && newPosition > loopOutPosition) {
                            editor.setText(formatTime(loopInPosition), juce::dontSendNotification);
                            editor.setColour(juce::TextEditor::textColourId, Config::textEditorWarningColor);
                        } else {
                            loopInPosition = newPosition;
                            DBG("    Loop In position set to: " << loopInPosition);
                            updateLoopButtonColors();
                            silenceDetector->setIsAutoCutInActive(false); // User manually set loop in, so auto-cut is no longer active
                            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
                            repaint();
                        }
                    } else {
                        editor.setText(formatTime(loopInPosition), juce::dontSendNotification);
                        editor.setColour(juce::TextEditor::textColourId, Config::textEditorErrorColor);
                        repaint();
                    }
                } else if (&editor == &loopOutEditor) {
                    DBG("  Loop Out Editor: Focus Lost");

                    double newPosition = parseTime(editor.getText());
                    if (newPosition >= 0.0 && newPosition <= owner.getAudioPlayer()->getThumbnail().getTotalLength()) {
                        if (loopInPosition > -1.0 && newPosition < loopInPosition) {
                            editor.setText(formatTime(loopOutPosition), juce::dontSendNotification);
                            editor.setColour(juce::TextEditor::textColourId, Config::textEditorWarningColor);
                        } else {
                            loopOutPosition = newPosition;
                            DBG("    Loop Out position set to: " << loopOutPosition);
                            updateLoopButtonColors();
                            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
                            repaint();
                        }
                    } else {
                        editor.setText(formatTime(loopOutPosition), juce::dontSendNotification);
                        editor.setColour(juce::TextEditor::textColourId, Config::textEditorErrorColor);
                    }
                }
}
/**
 * @brief Toggles the visibility of the statistics display.
 */
void ControlPanel::toggleStats() { statsButton.triggerClick(); }

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
double ControlPanel::parseTime(const juce::String& timeString) {
    auto parts = juce::StringArray::fromTokens(timeString, ":", "");
    if (parts.size() != 4) return -1.0;
    return parts[0].getIntValue() * 3600.0 + parts[1].getIntValue() * 60.0 + parts[2].getIntValue() + parts[3].getIntValue() / 1000.0;
}
void ControlPanel::ensureLoopOrder() { if (loopInPosition > loopOutPosition) std::swap(loopInPosition, loopOutPosition); }
void ControlPanel::setShouldShowStats(bool shouldShowStats) { showStats = shouldShowStats; resized(); }
void ControlPanel::setTotalTimeStaticString(const juce::String& timeString) { totalTimeStaticStr = timeString; }
/**
 * @brief Sets the text in the stats display box.
 * @param text The text to display.
 * @param color The color of the text.
 */
void ControlPanel::setStatsDisplayText(const juce::String& text, juce::Colour color) {
    statsDisplay.setText(text, juce::dontSendNotification);
    statsDisplay.setColour(juce::TextEditor::textColourId, color);
}

/**
 * @brief Updates the stats display with dynamic audio statistics.
 * @param statsText The formatted string containing the dynamic statistics.
 */
void ControlPanel::updateStatsDisplay(const juce::String& statsText) {
    statsDisplay.setText(statsText, juce::dontSendNotification);
    statsDisplay.setColour(juce::TextEditor::textColourId, Config::statsDisplayTextColour);
}
void ControlPanel::updateLoopButtonColors() {
    if (currentPlacementMode == AppEnums::PlacementMode::LoopIn) {
        loopInButton.setColour(juce::TextButton::buttonColourId, Config::loopButtonPlacementModeColor);
    } else {
        loopInButton.setColour(juce::TextButton::buttonColourId, Config::loopButtonActiveColor);
    }
    if (currentPlacementMode == AppEnums::PlacementMode::LoopOut) {
        loopOutButton.setColour(juce::TextButton::buttonColourId, Config::loopButtonPlacementModeColor);
    } else {
        loopOutButton.setColour(juce::TextButton::buttonColourId, Config::loopButtonActiveColor);
    }
    updateLoopLabels();
}

// Public accessors for SilenceDetector and other classes to interact with ControlPanel
AudioPlayer& ControlPanel::getAudioPlayer() const
{
    return *owner.getAudioPlayer();
}

// juce::TextEditor& ControlPanel::getStatsDisplay() { return statsDisplay; } // This was the inline definition

void ControlPanel::setLoopStart(int sampleIndex)
{
    AudioPlayer& audioPlayer = *owner.getAudioPlayer();
    if (audioPlayer.getAudioFormatReader() != nullptr)
    {
        double sampleRate = audioPlayer.getAudioFormatReader()->sampleRate;
        loopInPosition = (double)sampleIndex / sampleRate;
        ensureLoopOrder();
        updateLoopLabels();
        repaint();
    }
}

void ControlPanel::setLoopEnd(int sampleIndex)
{
    AudioPlayer& audioPlayer = *owner.getAudioPlayer();
    if (audioPlayer.getAudioFormatReader() != nullptr)
    {
        double sampleRate = audioPlayer.getAudioFormatReader()->sampleRate;
        loopOutPosition = (double)sampleIndex / sampleRate;
        ensureLoopOrder();
        updateLoopLabels();
        repaint();
    }
}

juce::String ControlPanel::formatTime(double seconds) const
{
    return owner.formatTime(seconds);
}

const juce::LookAndFeel& ControlPanel::getLookAndFeel() const
{
    return modernLF;
}


/**
 * @brief Handles mouse movement events.
 *        Updates the mouse cursor position and triggers repaint for visual feedback.
 * @param event The mouse event details.
 */
void ControlPanel::mouseMove(const juce::MouseEvent& event)
{
    if (waveformBounds.contains(event.getPosition()))
    {
        mouseCursorX = event.x;
        mouseCursorY = event.y;
        AudioPlayer* audioPlayer = owner.getAudioPlayer(); // Use pointer here
        auto audioLength = audioPlayer->getThumbnail().getTotalLength();
        if (audioLength > 0.0)
        {
            float proportion = (float)(mouseCursorX - waveformBounds.getX()) / (float)waveformBounds.getWidth();
            mouseCursorTime = proportion * audioLength;
        }
        else
        {
            mouseCursorTime = 0.0;
        }
    }
    else
    {
        mouseCursorX = -1;
        mouseCursorY = -1;
        mouseCursorTime = 0.0;
    }
    repaint();
}

/**
 * @brief Handles mouse down events.
 *        Initiates dragging for seeking or handles right-click for loop placement.
 * @param event The mouse event details.
 */
void ControlPanel::mouseDown(juce::MouseEvent const& event)
{
    // Explicitly handle focus loss for any TextEditor children
    // If a TextEditor has focus and the click is not on it, it should lose focus.
    for (auto* child : getChildren())
    {
        juce::TextEditor* editorChild = dynamic_cast<juce::TextEditor*>(child);
        if (editorChild != nullptr && editorChild->hasKeyboardFocus(false))
        {
            if (! editorChild->getBoundsInParent().contains(event.getPosition()))
            {
                editorChild->giveAwayKeyboardFocus();
            }
        }
    }

    if (waveformBounds.contains(event.getPosition()))
    {
        if (event.mods.isLeftButtonDown())
        {
            isDragging = true;
            mouseDragStartX = event.x;
            currentPlaybackPosOnDragStart = owner.getAudioPlayer()->getTransportSource().getCurrentPosition();
            seekToMousePosition(event.x);
        }
        else if (event.mods.isRightButtonDown())
        {
            handleRightClickForLoopPlacement(event.x);
        }
    }
}

/**
 * @brief Handles mouse drag events.
 *        Updates playback position if dragging is active.
 * @param event The mouse event details.
 */
void ControlPanel::mouseDrag(const juce::MouseEvent& event)
{
    if (isDragging && event.mods.isLeftButtonDown() && waveformBounds.contains(event.getPosition()))
    {
        seekToMousePosition(event.x);
    }
}

/**
 * @brief Handles mouse up events.
 *        Stops dragging and finalizes seek operation, or handles left-click for seeking.
 * @param event The mouse event details.
 */
void ControlPanel::mouseUp(const juce::MouseEvent& event)
{
    isDragging = false;
    if (waveformBounds.contains(event.getPosition()) && event.mods.isLeftButtonDown())
    {
        if (currentPlacementMode != AppEnums::PlacementMode::None)
        {
            AudioPlayer* audioPlayer = owner.getAudioPlayer(); // Use pointer here
            auto audioLength = audioPlayer->getThumbnail().getTotalLength();
            if (audioLength > 0.0)
            {
                float proportion = (float)(event.x - waveformBounds.getX()) / (float)waveformBounds.getWidth();
                double time = proportion * audioLength;

                if (currentPlacementMode == AppEnums::PlacementMode::LoopIn)
                {
                    loopInPosition = time;
                }
                else if (currentPlacementMode == AppEnums::PlacementMode::LoopOut)
                {
                    loopOutPosition = time;
                }
                ensureLoopOrder();
                updateLoopLabels();
            }
            currentPlacementMode = AppEnums::PlacementMode::None; // Reset placement mode
            updateLoopButtonColors(); // Update button colours
            repaint();
        }
        // If it was a click (not a drag) then seek to position.
        // If it was a drag, seekToMousePosition would have already been called.
        else if (mouseDragStartX == event.x)
        {
            seekToMousePosition(event.x);
        }
    }
}

/**
 * @brief Handles mouse exit events from the component.
 *        Resets mouse cursor position to hide visual feedback.
 * @param event The mouse event details.
 */
void ControlPanel::mouseExit(const juce::MouseEvent& event)
{
    mouseCursorX = -1;
    mouseCursorY = -1;
    mouseCursorTime = 0.0;
    repaint();
}

/**
 * @brief Handles right-click events for placing loop in/out points.
 * @param x The x-coordinate of the mouse click relative to the component.
 */
void ControlPanel::handleRightClickForLoopPlacement(int x)
{
    AudioPlayer* audioPlayer = owner.getAudioPlayer(); // Use pointer here
    auto audioLength = audioPlayer->getThumbnail().getTotalLength();
    if (audioLength <= 0.0) return;

    float proportion = (float)(x - waveformBounds.getX()) / (float)waveformBounds.getWidth();
    double time = proportion * audioLength;

    if (currentPlacementMode == AppEnums::PlacementMode::LoopIn)
    {
        loopInPosition = time;
    }
    else if (currentPlacementMode == AppEnums::PlacementMode::LoopOut)
    {
        loopOutPosition = time;
    }
    ensureLoopOrder();
    updateLoopButtonColors();
    updateLoopLabels();
    repaint();
}

/**
 * @brief Seeks the audio player to the position corresponding to the given x-coordinate.
 * @param x The x-coordinate of the mouse position relative to the component.
 */
void ControlPanel::seekToMousePosition(int x)
{
    AudioPlayer* audioPlayer = owner.getAudioPlayer(); // Use pointer here
    auto audioLength = audioPlayer->getThumbnail().getTotalLength();
    if (audioLength <= 0.0) return;

    float proportion = (float)(x - waveformBounds.getX()) / (float)waveformBounds.getWidth();
    double time = proportion * audioLength;
    audioPlayer->getTransportSource().setPosition(time);
}