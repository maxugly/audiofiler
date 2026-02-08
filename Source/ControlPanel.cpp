#include "ControlPanel.h"
#include "MainComponent.h" // Include MainComponent for access to AudioPlayer etc.
#include "Config.h"
#include <cmath> // For std::abs

ControlPanel::ControlPanel(MainComponent& ownerComponent) : owner(ownerComponent)
{
    initialiseLookAndFeel();
    initialiseButtons();
    initialiseLoopButtons();
    initialiseClearButtons();
    initialiseLoopEditors();
    finaliseSetup();
    setMouseCursor(juce::MouseCursor::CrosshairCursor);
}

ControlPanel::~ControlPanel()
{
    setLookAndFeel(nullptr);
}

void ControlPanel::initialiseLookAndFeel()
{
    setLookAndFeel (&modernLF);
    modernLF.setBaseOffColor(Config::buttonBaseColour);
    modernLF.setBaseOnColor(Config::buttonOnColour);
    modernLF.setTextColor(Config::buttonTextColour);
}

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

    addAndMakeVisible (statsDisplay);
    statsDisplay.setReadOnly (true);
    statsDisplay.setMultiLine (true);
    statsDisplay.setWantsKeyboardFocus (false);
    statsDisplay.setColour (juce::TextEditor::backgroundColourId, Config::statsDisplayBackgroundColour);
    statsDisplay.setColour (juce::TextEditor::textColourId, Config::statsDisplayTextColour);
    statsDisplay.setVisible (false);
}

void ControlPanel::initialiseOpenButton()
{
    addAndMakeVisible(openButton);
    openButton.setButtonText(Config::openButtonText);
    openButton.onClick = [this] { owner.openButtonClicked(); };
}

void ControlPanel::initialisePlayStopButton()
{
    addAndMakeVisible(playStopButton);
    playStopButton.setButtonText(Config::playButtonText); // Initialize text
    playStopButton.onClick = [this] { owner.getAudioPlayer()->togglePlayStop(); };
    playStopButton.setEnabled(false);
}

void ControlPanel::initialiseModeButton()
{
    addAndMakeVisible (modeButton);
    modeButton.setButtonText (Config::viewModeClassicText);
    modeButton.setClickingTogglesState (true);
    modeButton.onClick = [this] {
        currentMode = modeButton.getToggleState() ? AppEnums::ViewMode::Overlay : AppEnums::ViewMode::Classic;
        modeButton.setButtonText (currentMode == AppEnums::ViewMode::Classic ? Config::viewModeClassicText : Config::viewModeOverlayText);
        resized();
        repaint();
    };
}

void ControlPanel::initialiseChannelViewButton()
{
    addAndMakeVisible(channelViewButton);
    channelViewButton.setButtonText(Config::channelViewMonoText);
    channelViewButton.setClickingTogglesState(true);
    channelViewButton.onClick = [this] {
        currentChannelViewMode = channelViewButton.getToggleState() ? AppEnums::ChannelViewMode::Stereo : AppEnums::ChannelViewMode::Mono;
        channelViewButton.setButtonText(currentChannelViewMode == AppEnums::ChannelViewMode::Mono ? Config::channelViewMonoText : Config::channelViewStereoText);
        repaint();
    };
}

void ControlPanel::initialiseQualityButton()
{
    addAndMakeVisible(qualityButton);
    qualityButton.setButtonText(Config::qualityButtonText);
    qualityButton.onClick = [this] {
        if (currentQuality == AppEnums::ThumbnailQuality::High)
            currentQuality = AppEnums::ThumbnailQuality::Medium;
        else if (currentQuality == AppEnums::ThumbnailQuality::Medium)
            currentQuality = AppEnums::ThumbnailQuality::Low;
        else
            currentQuality = AppEnums::ThumbnailQuality::High;
        updateQualityButtonText();
        repaint();
    };
    updateQualityButtonText();
}

void ControlPanel::initialiseExitButton()
{
    addAndMakeVisible(exitButton);
    exitButton.setButtonText(Config::exitButtonText);
    exitButton.setColour(juce::TextButton::buttonColourId, Config::exitButtonColor);
    exitButton.onClick = [] {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    };
}

void ControlPanel::initialiseStatsButton()
{
    addAndMakeVisible(statsButton);
    statsButton.setButtonText(Config::statsButtonText);
    statsButton.setClickingTogglesState(true);
    statsButton.onClick = [this] {
        showStats = statsButton.getToggleState();
        resized();
        updateComponentStates();
    };
}

void ControlPanel::initialiseLoopButton()
{
    addAndMakeVisible(loopButton);
    loopButton.setButtonText(Config::loopButtonText);
    loopButton.setClickingTogglesState(true);
    loopButton.onClick = [this] {
        shouldLoop = loopButton.getToggleState();
        owner.getAudioPlayer()->setLooping(shouldLoop);
    };
}

void ControlPanel::initialiseAutoplayButton()
{
    addAndMakeVisible(autoplayButton);
    autoplayButton.setButtonText(Config::autoplayButtonText);
    autoplayButton.setClickingTogglesState(true);
    autoplayButton.setToggleState(m_shouldAutoplay, juce::dontSendNotification);
    autoplayButton.onClick = [this] {
        m_shouldAutoplay = autoplayButton.getToggleState();
    };
}

void ControlPanel::initialiseAutoCutInButton()
{
    addAndMakeVisible(autoCutInButton);
    autoCutInButton.setButtonText(Config::autoCutInButtonText);
    autoCutInButton.setClickingTogglesState(true);
    autoCutInButton.setToggleState(m_shouldAutoCutIn, juce::dontSendNotification);
    autoCutInButton.onClick = [this] {
        m_shouldAutoCutIn = autoCutInButton.getToggleState();
        updateComponentStates();
        // If auto-cut in is now enabled, trigger detection
        if (m_shouldAutoCutIn && owner.getAudioPlayer()->getThumbnail().getTotalLength() > 0.0) {
            detectInSilence();
        }
    };
}

void ControlPanel::initialiseAutoCutOutButton()
{
    addAndMakeVisible(autoCutOutButton);
    autoCutOutButton.setButtonText(Config::autoCutOutButtonText);
    autoCutOutButton.setClickingTogglesState(true);
    autoCutOutButton.setToggleState(m_shouldAutoCutOut, juce::dontSendNotification);
    autoCutOutButton.onClick = [this] {
        m_shouldAutoCutOut = autoCutOutButton.getToggleState();
        updateComponentStates();
        // If auto-cut out is now enabled, trigger detection
        if (m_shouldAutoCutOut && owner.getAudioPlayer()->getThumbnail().getTotalLength() > 0.0) {
            detectOutSilence();
        }
    };
}

void ControlPanel::initialiseCutButton()
{
    addAndMakeVisible(cutButton);
    cutButton.setButtonText(Config::cutButtonText);
    cutButton.setClickingTogglesState(true);
    cutButton.setToggleState(isCutModeActive, juce::dontSendNotification);
    cutButton.onClick = [this] {
        isCutModeActive = cutButton.getToggleState();
        updateComponentStates();
    };
}

void ControlPanel::initialiseLoopButtons()
{
    addAndMakeVisible(loopInButton);
    loopInButton.setButtonText(Config::loopInButtonText);
    loopInButton.onLeftClick = [this] {
        loopInPosition = owner.getAudioPlayer()->getTransportSource().getCurrentPosition();
        ensureLoopOrder();
        updateLoopButtonColors();
        repaint();
    };
    loopInButton.onRightClick = [this] {
        currentPlacementMode = AppEnums::PlacementMode::LoopIn;
        updateLoopButtonColors();
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

void ControlPanel::initialiseLoopEditors()
{
    addAndMakeVisible(loopInEditor);
    loopInEditor.setReadOnly(false);
    loopInEditor.setJustification(juce::Justification::centred);
    loopInEditor.setColour(juce::TextEditor::backgroundColourId, Config::textEditorBackgroundColour);
    loopInEditor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
    loopInEditor.setFont(juce::Font(juce::FontOptions(Config::playbackTextSize)));
    loopInEditor.setMultiLine(false);
    loopInEditor.setReturnKeyStartsNewLine(false);
    loopInEditor.addListener(this);
    loopInEditor.setWantsKeyboardFocus(true);

    addAndMakeVisible(loopOutEditor);
    loopOutEditor.setReadOnly(false);
    loopOutEditor.setJustification(juce::Justification::centred);
    loopOutEditor.setColour(juce::TextEditor::backgroundColourId, Config::textEditorBackgroundColour);
    loopOutEditor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
    loopOutEditor.setFont(juce::Font(juce::FontOptions(Config::playbackTextSize)));
    loopOutEditor.setMultiLine(false);
    loopOutEditor.setReturnKeyStartsNewLine(false);
    loopOutEditor.addListener(this);
    loopOutEditor.setWantsKeyboardFocus(true);

    addAndMakeVisible(inSilenceThresholdEditor);
    inSilenceThresholdEditor.setText(juce::String(static_cast<int>(Config::silenceThreshold * 100.0f)));
    inSilenceThresholdEditor.setInputRestrictions(0, "0123456789");
    inSilenceThresholdEditor.setJustification(juce::Justification::centred);
    inSilenceThresholdEditor.setColour(juce::TextEditor::backgroundColourId, Config::textEditorBackgroundColour);
    inSilenceThresholdEditor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
    inSilenceThresholdEditor.setFont(juce::Font(juce::FontOptions(Config::playbackTextSize)));
    inSilenceThresholdEditor.applyFontToAllText(inSilenceThresholdEditor.getFont());
    inSilenceThresholdEditor.setMultiLine(false);
    inSilenceThresholdEditor.setReturnKeyStartsNewLine(false);
    inSilenceThresholdEditor.addListener(this);
    inSilenceThresholdEditor.setWantsKeyboardFocus(true);

    addAndMakeVisible(outSilenceThresholdEditor);
    outSilenceThresholdEditor.setText(juce::String(static_cast<int>(Config::outSilenceThreshold * 100.0f)));
    outSilenceThresholdEditor.setInputRestrictions(0, "0123456789");
    outSilenceThresholdEditor.setJustification(juce::Justification::centred);
    outSilenceThresholdEditor.setColour(juce::TextEditor::backgroundColourId, Config::textEditorBackgroundColour);
    outSilenceThresholdEditor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
    outSilenceThresholdEditor.setFont(juce::Font(juce::FontOptions(Config::playbackTextSize)));
    outSilenceThresholdEditor.applyFontToAllText(outSilenceThresholdEditor.getFont());
    outSilenceThresholdEditor.setMultiLine(false);
    outSilenceThresholdEditor.setReturnKeyStartsNewLine(false);
    outSilenceThresholdEditor.addListener(this);
    outSilenceThresholdEditor.setWantsKeyboardFocus(true);
}

void ControlPanel::finaliseSetup()
{
    updateLoopLabels();
    updateComponentStates();
}

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
    auto* audioPlayer = owner.getAudioPlayer();

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
        if (isCutModeActive) 
        {
            auto drawThresholdVisualisation = [&](juce::Graphics& g_ref, double loopPos, float threshold, bool isActive)
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

                if (isActive) {
                    juce::Colour glowColor = lineColor.withAlpha(lineColor.getFloatAlpha() * glowAlpha);
                    g_ref.setColour(glowColor);
                    g_ref.fillRect(lineStartX, topThresholdY - (Config::thresholdGlowThickness / 2.0f - 0.5f), currentLineWidth, Config::thresholdGlowThickness);
                    g_ref.fillRect(lineStartX, bottomThresholdY - (Config::thresholdGlowThickness / 2.0f - 0.5f), currentLineWidth, Config::thresholdGlowThickness);
                }

                g_ref.setColour(lineColor);
                g_ref.drawHorizontalLine((int)topThresholdY, lineStartX, lineEndX);
                g_ref.drawHorizontalLine((int)bottomThresholdY, lineStartX, lineEndX);
            };

            drawThresholdVisualisation(g, loopInPosition, currentInSilenceThreshold, m_shouldAutoCutIn);
            drawThresholdVisualisation(g, loopOutPosition, currentOutSilenceThreshold, m_shouldAutoCutOut);

            auto actualIn = juce::jmin(loopInPosition, loopOutPosition);
            auto actualOut = juce::jmax(loopInPosition, loopOutPosition);
            auto inX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (actualIn / audioLength);
            auto outX = (float)waveformBounds.getX() + (float)waveformBounds.getWidth() * (actualOut / audioLength);
            g.setColour(Config::loopRegionColor);
            g.fillRect(juce::Rectangle<float>(inX, (float)waveformBounds.getY(), outX - inX, (float)waveformBounds.getHeight()));

            juce::Colour glowColor = Config::loopLineColor.withAlpha(Config::loopLineColor.getFloatAlpha() * (1.0f - glowAlpha));
            g.setColour(glowColor);
            g.fillRect(inX - (Config::loopLineGlowThickness / 2.0f - 0.5f), (float)waveformBounds.getY(), Config::loopLineGlowThickness, (float)waveformBounds.getHeight());
            g.fillRect(outX - (Config::loopLineGlowThickness / 2.0f - 0.5f), (float)waveformBounds.getY(), Config::loopLineGlowThickness, (float)waveformBounds.getHeight());
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
        g.setColour (Config::mouseCursorHighlightColor);
        g.fillRect (mouseCursorX - 2, waveformBounds.getY(), 5, waveformBounds.getHeight());
        g.fillRect (waveformBounds.getX(), mouseCursorY - 2, waveformBounds.getWidth(), 5);

        if (audioLength > 0.0) 
        {
            float amplitude = 0.0f;
            if (audioPlayer->getThumbnail().getNumChannels() > 0)
            {
                // Get the amplitude at the mouse cursor's time position
                // Note: getLevel usually takes start/end times. For a single point, 
                // we approximate by taking a very small time window around mouseCursorTime.
                // A more accurate method might involve directly sampling the AudioFormatReader,
                // but this is simpler for visualization.
                auto* reader = audioPlayer->getAudioFormatReader();
                double sampleRate = (reader != nullptr) ? reader->sampleRate : 0.0;
                // Ensure sampleIndex is within valid range
                if (sampleRate > 0) // Check to avoid division by zero
                {
                    float minVal, maxVal;
                    // Use a very small window around the mouseCursorTime to get an approximate amplitude
                    audioPlayer->getThumbnail().getApproximateMinMax(mouseCursorTime, mouseCursorTime + (1.0 / sampleRate), 0, minVal, maxVal);
                    amplitude = juce::jmax(std::abs(minVal), std::abs(maxVal));
                }
            }

            // Draw amplitude line
            float centerY = (float)waveformBounds.getCentreY();
            float amplitudeY = centerY - (amplitude * waveformBounds.getHeight() * 0.5f);
            
            juce::ColourGradient amplitudeGlowGradient(Config::mouseAmplitudeGlowColor.withAlpha(0.0f), (float)mouseCursorX, amplitudeY,
                                                      Config::mouseAmplitudeGlowColor.withAlpha(0.7f), (float)mouseCursorX, centerY, true);
            g.setGradientFill(amplitudeGlowGradient);
            g.fillRect(juce::Rectangle<float>((float)mouseCursorX - Config::mouseAmplitudeGlowThickness / 2, amplitudeY, Config::mouseAmplitudeGlowThickness, centerY - amplitudeY));
            g.fillRect(juce::Rectangle<float>((float)mouseCursorX - Config::mouseAmplitudeGlowThickness / 2, centerY, Config::mouseAmplitudeGlowThickness, centerY - amplitudeY));

            g.setColour(Config::mouseAmplitudeLineColor);
            g.drawVerticalLine((float)mouseCursorX, centerY - (amplitude * waveformBounds.getHeight() * 0.5f), centerY + (amplitude * waveformBounds.getHeight() * 0.5f));

            // Draw amplitude text
            juce::String amplitudeText = juce::String(amplitude, 2);
            g.setColour(Config::playbackTextColor);
            g.setFont(Config::mouseCursorTextSize);
            g.drawText(amplitudeText, mouseCursorX + 5, (int)amplitudeY - Config::mouseCursorTextSize, 100, Config::mouseCursorTextSize, juce::Justification::left, true);

            // Draw time text
            juce::String timeText = owner.formatTime(mouseCursorTime);
            g.drawText(timeText, mouseCursorX + 5, mouseCursorY + 5, 100, Config::mouseCursorTextSize, juce::Justification::left, true);
        }

        g.setColour (Config::mouseCursorLineColor);
        g.drawVerticalLine (mouseCursorX, (float)waveformBounds.getY(), (float)waveformBounds.getBottom());
        g.drawHorizontalLine (mouseCursorY, (float)waveformBounds.getX(), (float)waveformBounds.getRight());
    } 

    if (audioLength > 0.0)
    {
        double currentTime = audioPlayer->getTransportSource().getCurrentPosition();
        double totalTime = audioPlayer->getThumbnail().getTotalLength();
        double remainingTime = totalTime - currentTime;

        juce::String currentTimeStr = owner.formatTime(currentTime);
        juce::String remainingTimeStr = owner.formatTime(remainingTime);

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
    loopInEditor.setText(owner.formatTime(loopInPosition), juce::dontSendNotification);
    loopOutEditor.setText(owner.formatTime(loopOutPosition), juce::dontSendNotification);
}

void ControlPanel::updateComponentStates()
{
    DBG("ControlPanel::updateComponentStates() - START");
    const bool enabled = owner.getAudioPlayer()->getThumbnail().getTotalLength() > 0.0;
    DBG("  - enabled (file loaded): " << (enabled ? "true" : "false"));
    DBG("  - isCutModeActive: " << (isCutModeActive ? "true" : "false"));
    DBG("  - m_shouldAutoCutIn: " << (m_shouldAutoCutIn ? "true" : "false"));
    DBG("  - m_shouldAutoCutOut: " << (m_shouldAutoCutOut ? "true" : "false"));
    updateGeneralButtonStates(enabled);
    updateCutModeControlStates(isCutModeActive, enabled, m_shouldAutoCutIn, m_shouldAutoCutOut);
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
    inSilenceThresholdEditor.setBounds(loopRow.removeFromLeft(Config::thresholdEditorWidth)); loopRow.removeFromLeft(Config::windowBorderMargins);
    autoCutInButton.setBounds(loopRow.removeFromLeft(Config::buttonWidth)); loopRow.removeFromLeft(Config::windowBorderMargins * 2);
    outSilenceThresholdEditor.setBounds(loopRow.removeFromLeft(Config::thresholdEditorWidth)); loopRow.removeFromLeft(Config::windowBorderMargins);
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

void ControlPanel::updateCutModeControlStates(bool isCutModeActive, bool enabled, bool paramShouldAutoCutIn, bool paramShouldAutoCutOut)
{
    DBG("ControlPanel::updateCutModeControlStates() - START (isCutModeActive: " << (isCutModeActive ? "true" : "false") << ", parent enabled: " << (enabled ? "true" : "false") << ", paramShouldAutoCutIn: " << (paramShouldAutoCutIn ? "true" : "false") << ", paramShouldAutoCutOut: " << (paramShouldAutoCutOut ? "true" : "false") << ")");
    // Manual Loop In controls
    loopInButton.setEnabled(enabled && isCutModeActive && !paramShouldAutoCutIn); DBG("  - loopInButton enabled: " << (loopInButton.isEnabled() ? "true" : "false"));
    loopInEditor.setEnabled(enabled && isCutModeActive && !paramShouldAutoCutIn); DBG("  - loopInEditor enabled: " << (loopInEditor.isEnabled() ? "true" : "false"));
    clearLoopInButton.setEnabled(enabled && isCutModeActive && !paramShouldAutoCutIn); DBG("  - clearLoopInButton enabled: " << (clearLoopInButton.isEnabled() ? "true" : "false"));

    // Manual Loop Out controls
    loopOutButton.setEnabled(enabled && isCutModeActive && !paramShouldAutoCutOut); DBG("  - loopOutButton enabled: " << (loopOutButton.isEnabled() ? "true" : "false"));
    loopOutEditor.setEnabled(enabled && isCutModeActive && !paramShouldAutoCutOut); DBG("  - loopOutEditor enabled: " << (loopOutEditor.isEnabled() ? "true" : "false"));
    clearLoopOutButton.setEnabled(enabled && isCutModeActive && !paramShouldAutoCutOut); DBG("  - clearLoopOutButton enabled: " << (clearLoopOutButton.isEnabled() ? "true" : "false"));

    // Auto Cut In/Out Buttons
    autoCutInButton.setEnabled(enabled && isCutModeActive); DBG("  - autoCutInButton enabled: " << (autoCutInButton.isEnabled() ? "true" : "false"));
    autoCutOutButton.setEnabled(enabled && isCutModeActive); DBG("  - autoCutOutOutton enabled: " << (autoCutOutButton.isEnabled() ? "true" : "false"));

    // Threshold Editors - enabled only if corresponding auto-cut is active AND cut mode is active AND file is loaded
    inSilenceThresholdEditor.setEnabled(enabled && isCutModeActive && paramShouldAutoCutIn); DBG("  - inSilenceThresholdEditor enabled: " << (inSilenceThresholdEditor.isEnabled() ? "true" : "false"));
    outSilenceThresholdEditor.setEnabled(enabled && isCutModeActive && paramShouldAutoCutOut); DBG("  - outSilenceThresholdEditor enabled: " << (outSilenceThresholdEditor.isEnabled() ? "true" : "false"));

    // Visibility remains the same
    loopInButton.setVisible(isCutModeActive); DBG("  - loopInButton visible: " << (loopInButton.isVisible() ? "true" : "false"));
    loopOutButton.setVisible(isCutModeActive); DBG("  - loopOutButton visible: " << (loopOutButton.isVisible() ? "true" : "false"));
    loopInEditor.setVisible(isCutModeActive); DBG("  - loopInEditor visible: " << (loopInEditor.isVisible() ? "true" : "false"));
    loopOutEditor.setVisible(isCutModeActive); DBG("  - loopOutEditor visible: " << (loopOutEditor.isVisible() ? "true" : "false"));
    clearLoopInButton.setVisible(isCutModeActive); DBG("  - clearLoopInButton visible: " << (clearLoopInButton.isVisible() ? "true" : "false"));
    clearLoopOutButton.setVisible(isCutModeActive); DBG("  - clearLoopOutButton visible: " << (clearLoopOutButton.isVisible() ? "true" : "false"));
    inSilenceThresholdEditor.setVisible(isCutModeActive); DBG("  - inSilenceThresholdEditor visible: " << (inSilenceThresholdEditor.isVisible() ? "true" : "false"));
    outSilenceThresholdEditor.setVisible(isCutModeActive); DBG("  - outSilenceThresholdEditor visible: " << (outSilenceThresholdEditor.isVisible() ? "true" : "false"));
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
    if (&editor == &inSilenceThresholdEditor) {
        DBG("  In Silence Threshold Editor: " << editor.getText());
        int newPercentage = editor.getText().getIntValue();
        if (newPercentage >= 1 && newPercentage <= 99) {
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Valid
        } else {
            editor.setColour(juce::TextEditor::textColourId, Config::textEditorOutOfRangeColour); // Out of range
        }
    } else if (&editor == &outSilenceThresholdEditor) {
        DBG("  Out Silence Threshold Editor: " << editor.getText());
        int newPercentage = editor.getText().getIntValue();
        if (newPercentage >= 1 && newPercentage <= 99) {
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Valid
        } else {
            editor.setColour(juce::TextEditor::textColourId, Config::textEditorOutOfRangeColour); // Out of range
        }
    } else {
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
}

void ControlPanel::textEditorReturnKeyPressed (juce::TextEditor& editor) {
    DBG("Text Editor Return Key Pressed.");
    if (&editor == &loopInEditor) {
        DBG("  Loop In Editor: Return Key Pressed");
        if (m_shouldAutoCutIn) {
            DBG("  Loop In Editor: Return Key Pressed ignored: Auto Cut In is active.");
            editor.setText(owner.formatTime(loopInPosition), juce::dontSendNotification); // Revert text
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor); // Reset color
            editor.giveAwayKeyboardFocus();
            return;
        }
        double newPosition = parseTime(editor.getText());
        if (newPosition >= 0.0 && newPosition <= owner.getAudioPlayer()->getThumbnail().getTotalLength()) {
            if (loopOutPosition > -1.0 && newPosition > loopOutPosition) {
                            editor.setText(owner.formatTime(loopInPosition), juce::dontSendNotification);
                            editor.setColour(juce::TextEditor::textColourId, Config::textEditorWarningColor);
                        } else {
                            loopInPosition = newPosition;
                            DBG("    Loop In position set to: " << loopInPosition);
                            updateLoopButtonColors();
                            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
                            repaint();
                        }
                    } else {
                        editor.setText(owner.formatTime(loopInPosition), juce::dontSendNotification);
                        editor.setColour(juce::TextEditor::textColourId, Config::textEditorErrorColor);
                    }
                } else if (&editor == &loopOutEditor) {
                    DBG("  Loop Out Editor: Return Key Pressed");
                    if (m_shouldAutoCutOut) {
                        DBG("  Loop Out Editor: Return Key Pressed ignored: Auto Cut Out is active.");
                        editor.setText(owner.formatTime(loopOutPosition), juce::dontSendNotification);
                        editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
                        editor.giveAwayKeyboardFocus();
                        return;
                    }
                    double newPosition = parseTime(editor.getText());
                    if (newPosition >= 0.0 && newPosition <= owner.getAudioPlayer()->getThumbnail().getTotalLength()) {
                        if (shouldLoop && owner.getAudioPlayer()->getTransportSource().getCurrentPosition() >= loopOutPosition)
                        {
                            owner.getAudioPlayer()->getTransportSource().setPosition(loopInPosition);
                        }
                        if (loopInPosition > -1.0 && newPosition < loopInPosition) { 
                            editor.setText(owner.formatTime(loopOutPosition), juce::dontSendNotification);
                            editor.setColour(juce::TextEditor::textColourId, Config::textEditorWarningColor);
                        } else { 
                            loopOutPosition = newPosition;
                            DBG("    Loop Out position set to: " << loopOutPosition);
                            updateLoopButtonColors();
                            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
                            repaint();
                        }
                    } else { 
                        editor.setText(owner.formatTime(loopOutPosition), juce::dontSendNotification);
                        editor.setColour(juce::TextEditor::textColourId, Config::textEditorErrorColor);
                    }    } else if (&editor == &inSilenceThresholdEditor)
    {
        DBG("  In Silence Threshold Editor: Return Key Pressed - START");
        int newPercentage = editor.getText().getIntValue();
        if (newPercentage >= 1 && newPercentage <= 99)
        {
            currentInSilenceThreshold = static_cast<float>(newPercentage) / 100.0f;
            DBG("    In Silence threshold set to: " << currentInSilenceThreshold << ". Calling detectInSilence().");
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
            detectInSilence();
            DBG("  In Silence Threshold Editor: Return Key Pressed - detectInSilence() called.");
        }
        else
        {
            DBG("  In Silence Threshold Editor: Return Key Pressed - Threshold value out of range or invalid.");
            editor.setText(juce::String(static_cast<int>(currentInSilenceThreshold * 100.0f)), juce::dontSendNotification);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
        }
    } else if (&editor == &outSilenceThresholdEditor)
    {
        DBG("  Out Silence Threshold Editor: Return Key Pressed - START");
        int newPercentage = editor.getText().getIntValue();
        if (newPercentage >= 1 && newPercentage <= 99)
        {
            currentOutSilenceThreshold = static_cast<float>(newPercentage) / 100.0f;
            DBG("    Out Silence threshold set to: " << currentOutSilenceThreshold << ". Calling detectOutSilence().");
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
            detectOutSilence();
            DBG("  Out Silence Threshold Editor: Return Key Pressed - detectOutSilence() called.");
        }
        else
        {
            DBG("  Out Silence Threshold Editor: Return Key Pressed - Threshold value out of range or invalid.");
            editor.setText(juce::String(static_cast<int>(currentOutSilenceThreshold * 100.0f)), juce::dontSendNotification);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
        }
    }
    editor.giveAwayKeyboardFocus();
} 


void ControlPanel::textEditorEscapeKeyPressed (juce::TextEditor& editor) {
    DBG("Text Editor Escape Key Pressed.");
    if (&editor == &loopInEditor) {
        DBG("  Loop In Editor: Escape Key Pressed");
        editor.setText(owner.formatTime(loopInPosition), juce::dontSendNotification);
    } else if (&editor == &loopOutEditor) {
        DBG("  Loop Out Editor: Escape Key Pressed");
        editor.setText(owner.formatTime(loopOutPosition), juce::dontSendNotification);
    } else if (&editor == &inSilenceThresholdEditor) {
        DBG("  In Silence Threshold Editor: Escape Key Pressed");
        editor.setText(juce::String(static_cast<int>(currentInSilenceThreshold * 100.0f)), juce::dontSendNotification);
    } else if (&editor == &outSilenceThresholdEditor) {
        DBG("  Out Silence Threshold Editor: Escape Key Pressed");
        editor.setText(juce::String(static_cast<int>(currentOutSilenceThreshold * 100.0f)), juce::dontSendNotification);
    }
    editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
    editor.giveAwayKeyboardFocus();
}

void ControlPanel::textEditorFocusLost (juce::TextEditor& editor) {
    DBG("Text Editor Focus Lost.");
    if (&editor == &loopInEditor) {
        DBG("  Loop In Editor: Focus Lost");
        if (m_shouldAutoCutIn) {
            DBG("  Loop In Editor: Focus Lost ignored: Auto Cut In is active.");
            editor.setText(owner.formatTime(loopInPosition), juce::dontSendNotification);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
            return;
        }
        double newPosition = parseTime(editor.getText());
        if (newPosition >= 0.0 && newPosition <= owner.getAudioPlayer()->getThumbnail().getTotalLength()) {
            if (loopOutPosition > -1.0 && newPosition > loopOutPosition) {
                            editor.setText(owner.formatTime(loopInPosition), juce::dontSendNotification);
                            editor.setColour(juce::TextEditor::textColourId, Config::textEditorWarningColor);
                        } else {
                            loopInPosition = newPosition;
                            DBG("    Loop In position set to: " << loopInPosition);
                            updateLoopButtonColors();
                            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
                            repaint();
                        }
                    } else {
                        editor.setText(owner.formatTime(loopInPosition), juce::dontSendNotification);
                        editor.setColour(juce::TextEditor::textColourId, Config::textEditorErrorColor);
                        repaint();
                    }
                } else if (&editor == &loopOutEditor) {
                    DBG("  Loop Out Editor: Focus Lost");
                    if (m_shouldAutoCutOut) {
                        DBG("  Loop Out Editor: Focus Lost ignored: Auto Cut Out is active.");
                        editor.setText(owner.formatTime(loopOutPosition), juce::dontSendNotification);
                        editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
                        return;
                    }
                    double newPosition = parseTime(editor.getText());
                    if (newPosition >= 0.0 && newPosition <= owner.getAudioPlayer()->getThumbnail().getTotalLength()) {
                        if (loopInPosition > -1.0 && newPosition < loopInPosition) {
                            editor.setText(owner.formatTime(loopOutPosition), juce::dontSendNotification);
                            editor.setColour(juce::TextEditor::textColourId, Config::textEditorWarningColor);
                        } else {
                            loopOutPosition = newPosition;
                            DBG("    Loop Out position set to: " << loopOutPosition);
                            updateLoopButtonColors();
                            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
                            repaint();
                        }
                    } else { 
                        editor.setText(owner.formatTime(loopOutPosition), juce::dontSendNotification);
                        editor.setColour(juce::TextEditor::textColourId, Config::textEditorErrorColor);
                    }    } else if (&editor == &inSilenceThresholdEditor) {
        DBG("  In Silence Threshold Editor: Focus Lost - START");
        int newPercentage = editor.getText().getIntValue();
        if (newPercentage >= 1 && newPercentage <= 99)
        {
            currentInSilenceThreshold = static_cast<float>(newPercentage) / 100.0f;
            DBG("    In Silence threshold set to: " << currentInSilenceThreshold << ". Calling detectInSilence().");
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
            detectInSilence();
            DBG("  In Silence Threshold Editor: Focus Lost - detectInSilence() called.");
        }
        else
        {
            DBG("  In Silence Threshold Editor: Focus Lost - Threshold value out of range or invalid.");
            editor.setText(juce::String(static_cast<int>(currentInSilenceThreshold * 100.0f)), juce::dontSendNotification);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
        }
    } else if (&editor == &outSilenceThresholdEditor) {
        DBG("  Out Silence Threshold Editor: Focus Lost - START");
        int newPercentage = editor.getText().getIntValue();
        if (newPercentage >= 1 && newPercentage <= 99)
        {
            currentOutSilenceThreshold = static_cast<float>(newPercentage) / 100.0f;
            DBG("    Out Silence threshold set to: " << currentOutSilenceThreshold << ". Calling detectOutSilence().");
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
            detectOutSilence();
            DBG("  Out Silence Threshold Editor: Focus Lost - detectOutSilence() called.");
        }
        else
        {
            DBG("  Out Silence Threshold Editor: Focus Lost - Threshold value out of range or invalid.");
            editor.setText(juce::String(static_cast<int>(currentOutSilenceThreshold * 100.0f)), juce::dontSendNotification);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
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

void ControlPanel::detectInSilence()
{
    DBG("ControlPanel::detectInSilence() called.");
    auto* audioPlayer = owner.getAudioPlayer();
    bool wasPlaying = audioPlayer->isPlaying();
    if (wasPlaying) {
        audioPlayer->getTransportSource().stop();
        DBG("  Playback stopped for silence detection.");
    }

    juce::AudioFormatReader* reader = audioPlayer->getAudioFormatReader();

    if (!reader) {
        DBG("  AudioFormatReader is null. Returning.");
        if (wasPlaying) { audioPlayer->getTransportSource().start(); }
        return;
    }
    
    int numChannels = reader->numChannels;
    juce::int64 lengthInSamples = reader->lengthInSamples;
    DBG("  Reader: numChannels = " << numChannels << ", lengthInSamples = " << lengthInSamples);

    if (lengthInSamples <= 0) {
        if (wasPlaying) { audioPlayer->getTransportSource().start(); }
        return;
    }

    auto buffer = std::make_unique<juce::AudioBuffer<float>>(numChannels, (int)lengthInSamples);
    reader->read(buffer.get(), 0, (int)lengthInSamples, 0, true, true);

    float threshold = currentInSilenceThreshold;
    int firstSample = -1;

    for (int i = 0; i < buffer->getNumSamples(); ++i)
    {
        bool isSilent = true;
        for (int j = 0; j < buffer->getNumChannels(); ++j)
        {
            if (std::abs(buffer->getSample(j, i)) > threshold)
            {
                isSilent = false;
                break;
            }
        }
        if (!isSilent) { firstSample = i; break; }
    }

    if (firstSample != -1)
    {
        int loopInSample = 0;
        for (int i = firstSample; i > 0; --i)
        {
            if (i >= 1 && i < buffer->getNumSamples()) {
                bool sign1 = buffer->getSample(0, i) >= 0;
                bool sign2 = buffer->getSample(0, i - 1) >= 0;
                if (sign1 != sign2) { loopInSample = i; break; }
            }
        }
        loopInPosition = (double)loopInSample / reader->sampleRate;
        audioPlayer->getTransportSource().setPosition(loopInPosition);
        updateLoopButtonColors();
        updateLoopLabels();
        repaint();
    }

    if (wasPlaying) { audioPlayer->getTransportSource().start(); }
}

void ControlPanel::detectOutSilence()
{
    DBG("ControlPanel::detectOutSilence() called.");
    auto* audioPlayer = owner.getAudioPlayer();
    bool wasPlaying = audioPlayer->isPlaying();
    if (wasPlaying) {
        audioPlayer->getTransportSource().stop();
    }

    juce::AudioFormatReader* reader = audioPlayer->getAudioFormatReader();

    if (!reader || reader->lengthInSamples <= 0) {
        if (wasPlaying) audioPlayer->getTransportSource().start();
        return;
    }
    
    int numChannels = reader->numChannels;
    juce::int64 lengthInSamples = reader->lengthInSamples;
    auto buffer = std::make_unique<juce::AudioBuffer<float>>(numChannels, (int)lengthInSamples);
    reader->read(buffer.get(), 0, (int)lengthInSamples, 0, true, true);

    float threshold = currentOutSilenceThreshold;
    int lastSample = -1;

    for (int i = buffer->getNumSamples() - 1; i >= 0; --i)
    {
        bool isSilent = true;
        for (int j = 0; j < buffer->getNumChannels(); ++j)
        {
            if (std::abs(buffer->getSample(j, i)) > threshold)
            {
                isSilent = false;
                break;
            }
        }
        if (!isSilent) { lastSample = i; break; }
    }

    if (lastSample != -1)
    {
        int loopOutSample = buffer->getNumSamples() - 1;
        for (int i = lastSample; i < buffer->getNumSamples() - 1; ++i)
        {
            if (i >= 0 && i < buffer->getNumSamples() - 1) {
                bool sign1 = buffer->getSample(0, i) >= 0;
                bool sign2 = buffer->getSample(0, i + 1) >= 0;
                if (sign1 != sign2) { loopOutSample = i; break; }
            }
        }
        loopOutPosition = (double)loopOutSample / reader->sampleRate;
        audioPlayer->getTransportSource().setPosition(loopOutPosition);
        updateLoopButtonColors();
        updateLoopLabels();
        repaint();
    }

    if (wasPlaying) { audioPlayer->getTransportSource().start(); }
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
        auto* audioPlayer = owner.getAudioPlayer();
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
            auto* audioPlayer = owner.getAudioPlayer();
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
    auto* audioPlayer = owner.getAudioPlayer();
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
    auto* audioPlayer = owner.getAudioPlayer();
    auto audioLength = audioPlayer->getThumbnail().getTotalLength();
    if (audioLength <= 0.0) return;

    float proportion = (float)(x - waveformBounds.getX()) / (float)waveformBounds.getWidth();
    double time = proportion * audioLength;
    audioPlayer->getTransportSource().setPosition(time);
}