#include "ControlButtonsPresenter.h"

#include "ControlPanel.h"
#include "WaveformView.h"
#include "CutLayerView.h"
#include "ControlPanelCopy.h"
#include "TransportPresenter.h"
#include "SilenceDetectionPresenter.h"
#include "RepeatPresenter.h"
#include "CutResetPresenter.h"
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
    initialiseRepeatButton();
    initialiseAutoplayButton();
    initialiseAutoCutInButton();
    initialiseAutoCutOutButton();
    initialiseCutButton();
    initialiseCutBoundaryButtons();
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
        if (owner.waveformView != nullptr)
            owner.waveformView->setChannelMode(owner.currentChannelViewMode);
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
        if (owner.waveformView != nullptr)
            owner.waveformView->setQuality(owner.currentQuality);
        owner.repaint();
    };
    owner.updateQualityButtonText();
}

void ControlButtonsPresenter::initialiseExitButton()
{
    owner.addAndMakeVisible(owner.exitButton);
    owner.exitButton.setButtonText(ControlPanelCopy::exitButtonText());
    owner.exitButton.setColour(juce::TextButton::buttonColourId, Config::Colors::Button::exit);
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

void ControlButtonsPresenter::initialiseRepeatButton()
{
    owner.addAndMakeVisible(owner.repeatButton);
    owner.repeatButton.setButtonText(ControlPanelCopy::repeatButtonText());
    owner.repeatButton.setClickingTogglesState(true);
    owner.repeatButton.onClick = [this] {
        if (owner.transportPresenter != nullptr)
            owner.transportPresenter->handleRepeatToggle(owner.repeatButton.getToggleState());
    };
}

void ControlButtonsPresenter::initialiseAutoplayButton()
{
    owner.addAndMakeVisible(owner.autoplayButton);
    owner.autoplayButton.setButtonText(ControlPanelCopy::autoplayButtonText());
    owner.autoplayButton.setClickingTogglesState(true);
    owner.autoplayButton.setToggleState(owner.shouldAutoplay(), juce::dontSendNotification);
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
    owner.cutButton.setToggleState(owner.isCutModeActive(), juce::dontSendNotification);
    owner.cutButton.onClick = [this] {
        if (owner.transportPresenter != nullptr)
            owner.transportPresenter->handleCutModeToggle(owner.cutButton.getToggleState());
    };
}

void ControlButtonsPresenter::initialiseCutBoundaryButtons()
{
    owner.addAndMakeVisible(owner.cutInButton);
    owner.cutInButton.setButtonText(ControlPanelCopy::cutInButtonText());
    owner.cutInButton.onLeftClick = [this] {
        owner.setCutInPosition(owner.getAudioPlayer().getCurrentPosition());
        owner.ensureCutOrder();
        owner.updateCutButtonColors();
        owner.setAutoCutInActive(false);
        owner.repaint();
    };
    owner.cutInButton.onRightClick = [this] {
        owner.getMouseHandler().setPlacementMode(AppEnums::PlacementMode::CutIn);
        owner.updateCutButtonColors();
        owner.repaint();
    };

    owner.addAndMakeVisible(owner.cutOutButton);
    owner.cutOutButton.setButtonText(ControlPanelCopy::cutOutButtonText());
    owner.cutOutButton.onLeftClick = [this] {
        owner.setCutOutPosition(owner.getAudioPlayer().getCurrentPosition());
        owner.ensureCutOrder();
        owner.updateCutButtonColors();
        owner.setAutoCutOutActive(false);
        owner.repaint();
    };
    owner.cutOutButton.onRightClick = [this] {
        owner.getMouseHandler().setPlacementMode(AppEnums::PlacementMode::CutOut);
        owner.updateCutButtonColors();
        owner.repaint();
    };
}

void ControlButtonsPresenter::initialiseClearButtons()
{
    owner.addAndMakeVisible(owner.resetInButton);
    owner.resetInButton.setButtonText(ControlPanelCopy::clearButtonText());
    owner.resetInButton.setColour(juce::TextButton::buttonColourId, Config::Colors::Button::clear);
    owner.resetInButton.onClick = [this] {
        if (owner.cutResetPresenter != nullptr)
            owner.cutResetPresenter->resetIn();
    };

    owner.addAndMakeVisible(owner.resetOutButton);
    owner.resetOutButton.setButtonText(ControlPanelCopy::clearButtonText());
    owner.resetOutButton.setColour(juce::TextButton::buttonColourId, Config::Colors::Button::clear);
    owner.resetOutButton.onClick = [this] {
        if (owner.cutResetPresenter != nullptr)
            owner.cutResetPresenter->resetOut();
    };
}
