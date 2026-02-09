#include "LayoutManager.h"

#include "ControlPanel.h"
#include "Config.h"
#include "SilenceDetector.h"
#include "StatsPresenter.h"

LayoutManager::LayoutManager(ControlPanel& controlPanelIn)
    : controlPanel(controlPanelIn)
{
}

void LayoutManager::performLayout()
{
    DBG("ControlPanel::resized() - START");
    auto bounds = controlPanel.getLocalBounds();
    const int rowHeight = Config::buttonHeight + Config::windowBorderMargins * 2;

    layoutTopRowButtons(bounds, rowHeight);
    layoutLoopAndCutControls(bounds, rowHeight);
    layoutBottomRowAndTextDisplay(bounds, rowHeight);
    layoutWaveformAndStats(bounds);
    DBG("ControlPanel::resized() - END");
}

void LayoutManager::layoutTopRowButtons(juce::Rectangle<int>& bounds, int rowHeight)
{
    auto topRow = bounds.removeFromTop(rowHeight).reduced(Config::windowBorderMargins);
    controlPanel.openButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
    controlPanel.playStopButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
    controlPanel.autoplayButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
    controlPanel.loopButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
    controlPanel.cutButton.setBounds(topRow.removeFromLeft(Config::buttonWidth)); topRow.removeFromLeft(Config::windowBorderMargins);
    controlPanel.exitButton.setBounds(topRow.removeFromRight(Config::buttonWidth)); topRow.removeFromRight(Config::windowBorderMargins);
}

void LayoutManager::layoutLoopAndCutControls(juce::Rectangle<int>& bounds, int rowHeight)
{
    auto loopRow = bounds.removeFromTop(rowHeight).reduced(Config::windowBorderMargins);
    controlPanel.loopInButton.setBounds(loopRow.removeFromLeft(Config::buttonWidth)); loopRow.removeFromLeft(Config::windowBorderMargins);
    controlPanel.loopInEditor.setBounds(loopRow.removeFromLeft(Config::loopTextWidth)); loopRow.removeFromLeft(Config::windowBorderMargins / 2);
    controlPanel.clearLoopInButton.setBounds(loopRow.removeFromLeft(Config::clearButtonWidth)); loopRow.removeFromLeft(Config::clearButtonMargin);
    loopRow.removeFromLeft(Config::windowBorderMargins * 2);
    controlPanel.loopOutButton.setBounds(loopRow.removeFromLeft(Config::buttonWidth)); loopRow.removeFromLeft(Config::windowBorderMargins);
    controlPanel.loopOutEditor.setBounds(loopRow.removeFromLeft(Config::loopTextWidth)); loopRow.removeFromLeft(Config::windowBorderMargins / 2);
    controlPanel.clearLoopOutButton.setBounds(loopRow.removeFromLeft(Config::clearButtonWidth)); loopRow.removeFromLeft(Config::clearButtonMargin);
    loopRow.removeFromLeft(Config::windowBorderMargins * 2);
    controlPanel.silenceDetector->getInSilenceThresholdEditor().setBounds(loopRow.removeFromLeft(Config::thresholdEditorWidth)); loopRow.removeFromLeft(Config::windowBorderMargins);
    controlPanel.autoCutInButton.setBounds(loopRow.removeFromLeft(Config::buttonWidth)); loopRow.removeFromLeft(Config::windowBorderMargins * 2);
    controlPanel.silenceDetector->getOutSilenceThresholdEditor().setBounds(loopRow.removeFromLeft(Config::thresholdEditorWidth)); loopRow.removeFromLeft(Config::windowBorderMargins);
    controlPanel.autoCutOutButton.setBounds(loopRow.removeFromLeft(Config::buttonWidth));
}

void LayoutManager::layoutBottomRowAndTextDisplay(juce::Rectangle<int>& bounds, int rowHeight)
{
    auto bottomRow = bounds.removeFromBottom(rowHeight).reduced(Config::windowBorderMargins);
    controlPanel.bottomRowTopY = bottomRow.getY();
    controlPanel.contentAreaBounds = bounds.reduced(Config::windowBorderMargins);
    controlPanel.qualityButton.setBounds(bottomRow.removeFromRight(Config::buttonWidth)); bottomRow.removeFromRight(Config::windowBorderMargins);
    controlPanel.channelViewButton.setBounds(bottomRow.removeFromRight(Config::buttonWidth)); bottomRow.removeFromRight(Config::windowBorderMargins);
    controlPanel.statsButton.setBounds(bottomRow.removeFromRight(Config::buttonWidth)); bottomRow.removeFromRight(Config::windowBorderMargins);
    controlPanel.modeButton.setBounds(bottomRow.removeFromRight(Config::buttonWidth));

    controlPanel.playbackLeftTextX = controlPanel.getLocalBounds().getX() + Config::windowBorderMargins;
    controlPanel.playbackCenterTextX = (controlPanel.getLocalBounds().getWidth() / 2) - (Config::playbackTextWidth / 2);
    controlPanel.playbackRightTextX = controlPanel.getLocalBounds().getRight() - Config::windowBorderMargins - Config::playbackTextWidth;
}

void LayoutManager::layoutWaveformAndStats(juce::Rectangle<int>& bounds)
{
    if (controlPanel.currentMode == AppEnums::ViewMode::Overlay) {
        controlPanel.waveformBounds = controlPanel.getLocalBounds();
    } else {
        controlPanel.waveformBounds = bounds.reduced(Config::windowBorderMargins);
    }

    if (controlPanel.statsPresenter != nullptr)
        controlPanel.statsPresenter->layoutWithin(controlPanel.contentAreaBounds);
}
