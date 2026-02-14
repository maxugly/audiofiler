#include "LayoutManager.h"

#include "ControlPanel.h"
#include "Config.h"
#include "SilenceDetector.h"
#include "StatsPresenter.h"
#include "PlaybackTextPresenter.h"

LayoutManager::LayoutManager(ControlPanel& controlPanelIn)
    : controlPanel(controlPanelIn)
{
}

void LayoutManager::performLayout()
{
    auto bounds = controlPanel.getLocalBounds();
    const int rowHeight = Config::Layout::buttonHeight + Config::Layout::windowBorderMargins * 2;

    layoutTopRowButtons(bounds, rowHeight);
    layoutLoopAndCutControls(bounds, rowHeight);
    layoutBottomRowAndTextDisplay(bounds, rowHeight);
    layoutWaveformAndStats(bounds);
}

void LayoutManager::layoutTopRowButtons(juce::Rectangle<int>& bounds, int rowHeight)
{
    auto topRow = bounds.removeFromTop(rowHeight).reduced(Config::Layout::windowBorderMargins);
    controlPanel.openButton.setBounds(topRow.removeFromLeft(Config::Layout::buttonWidth)); topRow.removeFromLeft(Config::Layout::windowBorderMargins);
    controlPanel.playStopButton.setBounds(topRow.removeFromLeft(Config::Layout::buttonWidth)); topRow.removeFromLeft(Config::Layout::windowBorderMargins);
    controlPanel.autoplayButton.setBounds(topRow.removeFromLeft(Config::Layout::buttonWidth)); topRow.removeFromLeft(Config::Layout::windowBorderMargins);
    controlPanel.loopButton.setBounds(topRow.removeFromLeft(Config::Layout::buttonWidth)); topRow.removeFromLeft(Config::Layout::windowBorderMargins);
    controlPanel.cutButton.setBounds(topRow.removeFromLeft(Config::Layout::buttonWidth)); topRow.removeFromLeft(Config::Layout::windowBorderMargins);
    controlPanel.exitButton.setBounds(topRow.removeFromRight(Config::Layout::buttonWidth)); topRow.removeFromRight(Config::Layout::windowBorderMargins);
}

void LayoutManager::layoutLoopAndCutControls(juce::Rectangle<int>& bounds, int rowHeight)
{
    auto loopRow = bounds.removeFromTop(rowHeight).reduced(Config::Layout::windowBorderMargins);
    
    // Group "In" controls
    controlPanel.loopInButton.setBounds(loopRow.removeFromLeft(Config::Layout::buttonWidth));
    loopRow.removeFromLeft(Config::Layout::windowBorderMargins);
    controlPanel.loopInEditor.setBounds(loopRow.removeFromLeft(Config::Layout::loopTextWidth));
    loopRow.removeFromLeft(Config::Layout::windowBorderMargins / 2);
    controlPanel.clearLoopInButton.setBounds(loopRow.removeFromLeft(Config::Layout::clearButtonWidth));
    loopRow.removeFromLeft(Config::Layout::windowBorderMargins);
    controlPanel.silenceDetector->getInSilenceThresholdEditor().setBounds(loopRow.removeFromLeft(Config::Layout::thresholdEditorWidth));
    loopRow.removeFromLeft(Config::Layout::windowBorderMargins / 2);
    controlPanel.autoCutInButton.setBounds(loopRow.removeFromLeft(Config::Layout::buttonWidth));
    
    // Space between groups
    loopRow.removeFromLeft(Config::Layout::windowBorderMargins * 3);
    
    // Group "Out" controls
    controlPanel.loopOutButton.setBounds(loopRow.removeFromLeft(Config::Layout::buttonWidth));
    loopRow.removeFromLeft(Config::Layout::windowBorderMargins);
    controlPanel.loopOutEditor.setBounds(loopRow.removeFromLeft(Config::Layout::loopTextWidth));
    loopRow.removeFromLeft(Config::Layout::windowBorderMargins / 2);
    controlPanel.clearLoopOutButton.setBounds(loopRow.removeFromLeft(Config::Layout::clearButtonWidth));
    loopRow.removeFromLeft(Config::Layout::windowBorderMargins);
    controlPanel.silenceDetector->getOutSilenceThresholdEditor().setBounds(loopRow.removeFromLeft(Config::Layout::thresholdEditorWidth));
    loopRow.removeFromLeft(Config::Layout::windowBorderMargins / 2);
    controlPanel.autoCutOutButton.setBounds(loopRow.removeFromLeft(Config::Layout::buttonWidth));
}

void LayoutManager::layoutBottomRowAndTextDisplay(juce::Rectangle<int>& bounds, int rowHeight)
{
    auto bottomRow = bounds.removeFromBottom(rowHeight).reduced(Config::Layout::windowBorderMargins);
    controlPanel.layoutCache.bottomRowTopY = bottomRow.getY();
    controlPanel.layoutCache.contentAreaBounds = bounds.reduced(Config::Layout::windowBorderMargins);
    controlPanel.qualityButton.setBounds(bottomRow.removeFromRight(Config::Layout::buttonWidth)); bottomRow.removeFromRight(Config::Layout::windowBorderMargins);
    controlPanel.channelViewButton.setBounds(bottomRow.removeFromRight(Config::Layout::buttonWidth)); bottomRow.removeFromRight(Config::Layout::windowBorderMargins);
    controlPanel.statsButton.setBounds(bottomRow.removeFromRight(Config::Layout::buttonWidth)); bottomRow.removeFromRight(Config::Layout::windowBorderMargins);
    controlPanel.modeButton.setBounds(bottomRow.removeFromRight(Config::Layout::buttonWidth));

    controlPanel.layoutCache.playbackLeftTextX = controlPanel.getLocalBounds().getX() + Config::Layout::windowBorderMargins;
    controlPanel.layoutCache.playbackCenterTextX = (controlPanel.getLocalBounds().getWidth() / 2) - (Config::Layout::Text::playbackWidth / 2);
    controlPanel.layoutCache.playbackRightTextX = controlPanel.getLocalBounds().getRight() - Config::Layout::windowBorderMargins - Config::Layout::Text::playbackWidth;

    if (controlPanel.playbackTextPresenter != nullptr)
        controlPanel.playbackTextPresenter->layoutEditors();
}

void LayoutManager::layoutWaveformAndStats(juce::Rectangle<int>& bounds)
{
    if (controlPanel.currentMode == AppEnums::ViewMode::Overlay) {
        controlPanel.layoutCache.waveformBounds = controlPanel.getLocalBounds();
    } else {
        controlPanel.layoutCache.waveformBounds = bounds.reduced(Config::Layout::windowBorderMargins);
    }

    if (controlPanel.statsPresenter != nullptr)
        controlPanel.statsPresenter->layoutWithin(controlPanel.layoutCache.contentAreaBounds);
}
