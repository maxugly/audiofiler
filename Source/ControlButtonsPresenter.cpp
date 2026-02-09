#include "ControlButtonsPresenter.h"

#include "ControlPanel.h"
#include "ControlPanelCopy.h"
#include "TransportPresenter.h"
#include "SilenceDetectionPresenter.h"
#include "LoopPresenter.h"
#include "MouseHandler.h"
#include "SilenceDetector.h"

ControlButtonsPresenter::ControlButtonsPresenter(ControlPanel& ownerPanel)
    : owner(ownerPanel)
{
}

void ControlButtonsPresenter::initialiseAllButtons()
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
    initialiseLoopButtons();
    initialiseClearButtons();
}

void ControlButtonsPresenter::initialiseOpenButton()
{
    owner.addAndMakeVisible(owner.openButton);
    owner.openButton.setButtonText(ControlPanelCopy::openButtonText());
    owner.openButton.onClick = [this] { owner.invokeOwnerOpenDialog(); };
}

void ControlButtonsPresenter::initialisePlayStopButton()
{
    owner.addAndMakeVisible(owner.playStopButton);
    owner.playStopButton.setButtonText(ControlPanelCopy::playButtonText());
    owner.playStopButton.onClick = [this] { owner.getAudioPlayer().togglePlayStop(); };
    owner.playStopButton.setEnabled(false);
}

void ControlButtonsPresenter::initialiseModeButton()
{
    owner.addAndMakeVisible(owner.modeButton);
    owner.modeButton.setButtonText(ControlPanelCopy::viewModeClassicText());
    owner.modeButton.setClickingTogglesState(true);
    owner.modeButton.onClick = [this] {
        owner.currentMode = owner.modeButton.getToggleState() ? AppEnums::ViewMode::Overlay : AppEnums::ViewMode::Classic;
        owner.modeButton.setButtonText(owner.currentMode == AppEnums::ViewMode::Classic
                                       ? ControlPanelCopy::viewModeClassicText()
                                       : ControlPanelCopy::viewModeOverlayText());
        owner.resized();
        owner.repaint();
    };
}

void ControlButtonsPresenter::initialiseChannelViewButton()
{
    owner.addAndMakeVisible(owner.channelViewButton);
    owner.channelViewButton.setButtonText(ControlPanelCopy::channelViewMonoText());
    owner.channelViewButton.setClickingTogglesState(true);
    owner.channelViewButton.onClick = [this] {
        owner.currentChannelViewMode = owner.channelViewButton.getToggleState()
            ? AppEnums::ChannelViewMode::Stereo
            : AppEnums::ChannelViewMode::Mono;
        owner.channelViewButton.setButtonText(owner.currentChannelViewMode == AppEnums::ChannelViewMode::Mono
                                              ? ControlPanelCopy::channelViewMonoText()
                                              : ControlPanelCopy::channelViewStereoText());
        owner.repaint();
    };
}

void ControlButtonsPresenter::initialiseQualityButton()
{
    owner.addAndMakeVisible(owner.qualityButton);
    owner.qualityButton.setButtonText(ControlPanelCopy::qualityButtonText());
    owner.qualityButton.onClick = [this] {
        if (owner.currentQuality == AppEnums::ThumbnailQuality::High)
            owner.currentQuality = AppEnums::ThumbnailQuality::Medium;
        else if (owner.currentQuality == AppEnums::ThumbnailQuality::Medium)
            owner.currentQuality = AppEnums::ThumbnailQuality::Low;
        else
            owner.currentQuality = AppEnums::ThumbnailQuality::High;
        owner.updateQualityButtonText();
        owner.repaint();
    };
    owner.updateQualityButtonText();
}

void ControlButtonsPresenter::initialiseExitButton()
{
    owner.addAndMakeVisible(owner.exitButton);
    owner.exitButton.setButtonText(ControlPanelCopy::exitButtonText());
    owner.exitButton.setColour(juce::TextButton::buttonColourId, Config::exitButtonColor);
    owner.exitButton.onClick = [] {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    };
}

void ControlButtonsPresenter::initialiseStatsButton()
{
    owner.addAndMakeVisible(owner.statsButton);
    owner.statsButton.setButtonText(ControlPanelCopy::statsButtonText());
    owner.statsButton.setClickingTogglesState(true);
    owner.statsButton.onClick = [this] {
        owner.setShouldShowStats(owner.statsButton.getToggleState());
        owner.updateComponentStates();
    };
}

void ControlButtonsPresenter::initialiseLoopButton()
{
    owner.addAndMakeVisible(owner.loopButton);
    owner.loopButton.setButtonText(ControlPanelCopy::loopButtonText());
    owner.loopButton.setClickingTogglesState(true);
    owner.loopButton.onClick = [this] {
        if (owner.transportPresenter != nullptr)
            owner.transportPresenter->handleLoopToggle(owner.loopButton.getToggleState());
    };
}

void ControlButtonsPresenter::initialiseAutoplayButton()
{
    owner.addAndMakeVisible(owner.autoplayButton);
    owner.autoplayButton.setButtonText(ControlPanelCopy::autoplayButtonText());
    owner.autoplayButton.setClickingTogglesState(true);
    owner.autoplayButton.setToggleState(owner.m_shouldAutoplay, juce::dontSendNotification);
    owner.autoplayButton.onClick = [this] {
        if (owner.transportPresenter != nullptr)
            owner.transportPresenter->handleAutoplayToggle(owner.autoplayButton.getToggleState());
    };
}

void ControlButtonsPresenter::initialiseAutoCutInButton()
{
    owner.addAndMakeVisible(owner.autoCutInButton);
    owner.autoCutInButton.setButtonText(ControlPanelCopy::autoCutInButtonText());
    owner.autoCutInButton.setClickingTogglesState(true);
    owner.autoCutInButton.onClick = [this] {
        if (owner.silenceDetectionPresenter != nullptr)
            owner.silenceDetectionPresenter->handleAutoCutInToggle(owner.autoCutInButton.getToggleState());
    };
}

void ControlButtonsPresenter::initialiseAutoCutOutButton()
{
    owner.addAndMakeVisible(owner.autoCutOutButton);
    owner.autoCutOutButton.setButtonText(ControlPanelCopy::autoCutOutButtonText());
    owner.autoCutOutButton.setClickingTogglesState(true);
    owner.autoCutOutButton.onClick = [this] {
        if (owner.silenceDetectionPresenter != nullptr)
            owner.silenceDetectionPresenter->handleAutoCutOutToggle(owner.autoCutOutButton.getToggleState());
    };
}

void ControlButtonsPresenter::initialiseCutButton()
{
    owner.addAndMakeVisible(owner.cutButton);
    owner.cutButton.setButtonText(ControlPanelCopy::cutButtonText());
    owner.cutButton.setClickingTogglesState(true);
    owner.cutButton.setToggleState(owner.m_isCutModeActive, juce::dontSendNotification);
    owner.cutButton.onClick = [this] {
        if (owner.transportPresenter != nullptr)
            owner.transportPresenter->handleCutModeToggle(owner.cutButton.getToggleState());
    };
}

void ControlButtonsPresenter::initialiseLoopButtons()
{
    owner.addAndMakeVisible(owner.loopInButton);
    owner.loopInButton.setButtonText(ControlPanelCopy::loopInButtonText());
    owner.loopInButton.onLeftClick = [this] {
        owner.setLoopInPosition(owner.getAudioPlayer().getTransportSource().getCurrentPosition());
        owner.ensureLoopOrder();
        owner.updateLoopButtonColors();
        owner.silenceDetector->setIsAutoCutInActive(false);
        owner.repaint();
    };
    owner.loopInButton.onRightClick = [this] {
        owner.mouseHandler->setCurrentPlacementMode(AppEnums::PlacementMode::LoopIn);
        owner.updateLoopButtonColors();
        owner.repaint();
    };

    owner.addAndMakeVisible(owner.loopOutButton);
    owner.loopOutButton.setButtonText(ControlPanelCopy::loopOutButtonText());
    owner.loopOutButton.onLeftClick = [this] {
        owner.setLoopOutPosition(owner.getAudioPlayer().getTransportSource().getCurrentPosition());
        owner.ensureLoopOrder();
        owner.updateLoopButtonColors();
        owner.repaint();
    };
    owner.loopOutButton.onRightClick = [this] {
        owner.mouseHandler->setCurrentPlacementMode(AppEnums::PlacementMode::LoopOut);
        owner.updateLoopButtonColors();
        owner.repaint();
    };
}

void ControlButtonsPresenter::initialiseClearButtons()
{
    owner.addAndMakeVisible(owner.clearLoopInButton);
    owner.clearLoopInButton.setButtonText(ControlPanelCopy::clearButtonText());
    owner.clearLoopInButton.setColour(juce::TextButton::buttonColourId, Config::clearButtonColor);
    owner.clearLoopInButton.onClick = [this] {
        owner.setLoopInPosition(0.0);
        owner.ensureLoopOrder();
        owner.updateLoopButtonColors();
        owner.updateLoopLabels();
        owner.silenceDetector->setIsAutoCutInActive(false);
        owner.repaint();
    };

    owner.addAndMakeVisible(owner.clearLoopOutButton);
    owner.clearLoopOutButton.setButtonText(ControlPanelCopy::clearButtonText());
    owner.clearLoopOutButton.setColour(juce::TextButton::buttonColourId, Config::clearButtonColor);
    owner.clearLoopOutButton.onClick = [this] {
        owner.setLoopOutPosition(owner.getAudioPlayer().getThumbnail().getTotalLength());
        owner.ensureLoopOrder();
        owner.updateLoopButtonColors();
        owner.updateLoopLabels();
        owner.repaint();
    };
}

