#include "ControlPanel.h"
#include "MainComponent.h" // Include MainComponent for access to AudioPlayer etc.
#include "Config.h"

ControlPanel::ControlPanel(MainComponent& ownerComponent) : owner(ownerComponent)
{
    initialiseLookAndFeel();
    initialiseButtons();
    initialiseLoopButtons();
    initialiseClearButtons();
    initialiseLoopEditors();
    finaliseSetup();
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
    autoplayButton.setToggleState(shouldAutoplay, juce::dontSendNotification);
    autoplayButton.onClick = [this] {
        shouldAutoplay = autoplayButton.getToggleState();
    };
}

void ControlPanel::initialiseAutoCutInButton()
{
    addAndMakeVisible(autoCutInButton);
    autoCutInButton.setButtonText(Config::autoCutInButtonText);
    autoCutInButton.setClickingTogglesState(true);
    autoCutInButton.setToggleState(shouldAutoCutIn, juce::dontSendNotification);
    autoCutInButton.onClick = [this] {
        shouldAutoCutIn = autoCutInButton.getToggleState();
        if (shouldAutoCutIn && owner.getAudioPlayer()->getThumbnail().getTotalLength() > 0.0) {
            detectInSilence();
        }
        updateComponentStates();
    };
}

void ControlPanel::initialiseAutoCutOutButton()
{
    addAndMakeVisible(autoCutOutButton);
    autoCutOutButton.setButtonText(Config::autoCutOutButtonText);
    autoCutOutButton.setClickingTogglesState(true);
    autoCutOutButton.setToggleState(shouldAutoCutOut, juce::dontSendNotification);
    autoCutOutButton.onClick = [this] {
        shouldAutoCutOut = autoCutOutButton.getToggleState();
        if (shouldAutoCutOut && owner.getAudioPlayer()->getThumbnail().getTotalLength() > 0.0) {
            detectOutSilence();
        }
        updateComponentStates();
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
    auto bounds = getLocalBounds();
    int rowHeight = Config::buttonHeight + Config::windowBorderMargins * 2;

    layoutTopRowButtons(bounds, rowHeight);
    layoutLoopAndCutControls(bounds, rowHeight);
    layoutBottomRowAndTextDisplay(bounds, rowHeight);
    layoutWaveformAndStats(bounds);
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

            drawThresholdVisualisation(g, loopInPosition, currentInSilenceThreshold, shouldAutoCutIn);
            drawThresholdVisualisation(g, loopOutPosition, currentOutSilenceThreshold, shouldAutoCutOut);

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
          // ... drawing amplitude lines and text at mouse cursor ...
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
    const bool enabled = owner.getAudioPlayer()->getThumbnail().getTotalLength() > 0.0;
    updateGeneralButtonStates(enabled);
    updateCutModeControlStates(isCutModeActive, enabled, shouldAutoCutIn, shouldAutoCutOut);
}

void ControlPanel::layoutTopRowButtons(juce::Rectangle<int>& bounds, int rowHeight)
{
    auto topRow = bounds.removeFromTop(rowHeight).reduced(Config::windowBorderMargins);
    openButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
    playStopButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
    autoplayButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
    loopButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
    cutButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
    modeButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
    statsButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
    exitButton.setBounds(topRow.removeFromRight(Config::buttonWidth)); topRow.removeFromRight(Config::windowBorderMargins);
}

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

void ControlPanel::layoutBottomRowAndTextDisplay(juce::Rectangle<int>& bounds, int rowHeight)
{
    auto bottomRow = bounds.removeFromBottom(rowHeight).reduced(Config::windowBorderMargins);
    bottomRowTopY = bottomRow.getY();
    contentAreaBounds = bounds.reduced(Config::windowBorderMargins);
    qualityButton.setBounds(bottomRow.removeFromRight(Config::buttonWidth)); bottomRow.removeFromRight(Config::windowBorderMargins);
    channelViewButton.setBounds(bottomRow.removeFromRight(Config::buttonWidth));

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
    openButton.setEnabled(true);
    exitButton.setEnabled(true);
    loopButton.setEnabled(true);
    autoplayButton.setEnabled(true);
    cutButton.setEnabled(true);

    playStopButton.setEnabled(enabled);
    modeButton.setEnabled(enabled);
    statsButton.setEnabled(enabled);
    channelViewButton.setEnabled(enabled);
    qualityButton.setEnabled(enabled);
    statsDisplay.setEnabled(enabled);
}

void ControlPanel::updateCutModeControlStates(bool isCutModeActive, bool enabled, bool shouldAutoCutIn, bool shouldAutoCutOut)
{
    loopInButton.setEnabled(isCutModeActive && !shouldAutoCutIn);
    loopOutButton.setEnabled(isCutModeActive && !shouldAutoCutOut);
    loopInEditor.setEnabled(isCutModeActive && !shouldAutoCutIn);
    loopOutEditor.setEnabled(isCutModeActive && !shouldAutoCutOut);
    clearLoopInButton.setEnabled(isCutModeActive && !shouldAutoCutIn);
    clearLoopOutButton.setEnabled(isCutModeActive && !shouldAutoCutOut);

    inSilenceThresholdEditor.setEnabled(isCutModeActive);
    outSilenceThresholdEditor.setEnabled(isCutModeActive);

    autoCutInButton.setEnabled(isCutModeActive);
    autoCutOutButton.setEnabled(isCutModeActive);

    loopInButton.setVisible(isCutModeActive);
    loopOutButton.setVisible(isCutModeActive);
    loopInEditor.setVisible(isCutModeActive);
    loopOutEditor.setVisible(isCutModeActive);
    clearLoopInButton.setVisible(isCutModeActive);
    clearLoopOutButton.setVisible(isCutModeActive);
    inSilenceThresholdEditor.setVisible(isCutModeActive);
    outSilenceThresholdEditor.setVisible(isCutModeActive);
    autoCutInButton.setVisible(isCutModeActive);
    autoCutOutButton.setVisible(isCutModeActive);
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
        if (shouldAutoCutIn) {
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
                    if (shouldAutoCutOut) {
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
        DBG("  In Silence Threshold Editor: Return Key Pressed");
        int newPercentage = editor.getText().getIntValue();
        if (newPercentage >= 1 && newPercentage <= 99)
        {
            currentInSilenceThreshold = static_cast<float>(newPercentage) / 100.0f;
            DBG("    In Silence threshold set to: " << currentInSilenceThreshold);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
            detectInSilence();
        }
        else
        {
            editor.setText(juce::String(static_cast<int>(currentInSilenceThreshold * 100.0f)), juce::dontSendNotification);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
        }
    } else if (&editor == &outSilenceThresholdEditor)
    {
        DBG("  Out Silence Threshold Editor: Return Key Pressed");
        int newPercentage = editor.getText().getIntValue();
        if (newPercentage >= 1 && newPercentage <= 99)
        {
            currentOutSilenceThreshold = static_cast<float>(newPercentage) / 100.0f;
            DBG("    Out Silence threshold set to: " << currentOutSilenceThreshold);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
            detectOutSilence();
        }
        else
        {
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
        if (shouldAutoCutIn) {
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
                    if (shouldAutoCutOut) {
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
        DBG("  In Silence Threshold Editor: Focus Lost");
        int newPercentage = editor.getText().getIntValue();
        if (newPercentage >= 1 && newPercentage <= 99)
        {
            currentInSilenceThreshold = static_cast<float>(newPercentage) / 100.0f;
            DBG("    In Silence threshold set to: " << currentInSilenceThreshold);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
            detectInSilence();
        }
        else
        {
            editor.setText(juce::String(static_cast<int>(currentInSilenceThreshold * 100.0f)), juce::dontSendNotification);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
        }
    } else if (&editor == &outSilenceThresholdEditor) {
        DBG("  Out Silence Threshold Editor: Focus Lost");
        int newPercentage = editor.getText().getIntValue();
        if (newPercentage >= 1 && newPercentage <= 99)
        {
            currentOutSilenceThreshold = static_cast<float>(newPercentage) / 100.0f;
            DBG("    Out Silence threshold set to: " << currentOutSilenceThreshold);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
            detectOutSilence();
        }
        else
        {
            editor.setText(juce::String(static_cast<int>(currentOutSilenceThreshold * 100.0f)), juce::dontSendNotification);
            editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
        }
    }
}
void ControlPanel::ensureLoopOrder() { if (loopInPosition > loopOutPosition) std::swap(loopInPosition, loopOutPosition); }
void ControlPanel::setShouldShowStats(bool shouldShowStats) { showStats = shouldShowStats; resized(); }
void ControlPanel::setTotalTimeStaticString(const juce::String& timeString) { totalTimeStaticStr = timeString; }
void ControlPanel::setStatsDisplayText(const juce::String& text, juce::Colour color) { statsDisplay.setText(text, juce::dontSendNotification); statsDisplay.setColour(juce::TextEditor::textColourId, color); }
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
    DBG("detectInSilence() called.");
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
    DBG("detectOutSilence() called.");
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

void ControlPanel::toggleStats() { statsButton.triggerClick(); }
void ControlPanel::triggerQualityButton() { qualityButton.triggerClick(); }
void ControlPanel::triggerModeButton() { modeButton.triggerClick(); }
void ControlPanel::triggerChannelViewButton() { channelViewButton.triggerClick(); }
void ControlPanel::triggerLoopButton() { loopButton.triggerClick(); }
void ControlPanel::clearLoopIn() { clearLoopInButton.triggerClick(); }
void ControlPanel::clearLoopOut() { clearLoopOutButton.triggerClick(); }
double ControlPanel::parseTime(const juce::String& timeString) {
    auto parts = juce::StringArray::fromTokens(timeString, ":", "");
    if (parts.size() != 4) return -1.0;
    return parts[0].getIntValue() * 3600.0 + parts[1].getIntValue() * 60.0 + parts[2].getIntValue() + parts[3].getIntValue() / 1000.0;
}