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
    const int margin = Config::Layout::windowBorderMargins;
    const int rowHeight = Config::Layout::buttonHeight + margin * 2;

    layoutTopRowButtons(bounds, rowHeight);
    layoutLoopAndCutControls(bounds, rowHeight);
    layoutBottomRowAndTextDisplay(bounds, rowHeight);
    layoutWaveformAndStats(bounds);
}

void LayoutManager::layoutTopRowButtons(juce::Rectangle<int>& bounds, int rowHeight)
{
    const int margin = Config::Layout::windowBorderMargins;
    const int buttonWidth = Config::Layout::buttonWidth;

    auto topRow = bounds.removeFromTop(rowHeight).reduced(margin);
    controlPanel.openButton.setBounds(topRow.removeFromLeft(buttonWidth)); topRow.removeFromLeft(margin);
    controlPanel.playStopButton.setBounds(topRow.removeFromLeft(buttonWidth)); topRow.removeFromLeft(margin);
    controlPanel.autoplayButton.setBounds(topRow.removeFromLeft(buttonWidth)); topRow.removeFromLeft(margin);
    controlPanel.loopButton.setBounds(topRow.removeFromLeft(buttonWidth)); topRow.removeFromLeft(margin);
    controlPanel.cutButton.setBounds(topRow.removeFromLeft(buttonWidth)); topRow.removeFromLeft(margin);
    controlPanel.exitButton.setBounds(topRow.removeFromRight(buttonWidth)); topRow.removeFromRight(margin);
}

void LayoutManager::layoutLoopAndCutControls(juce::Rectangle<int>& bounds, int rowHeight)
{
    const int margin = Config::Layout::windowBorderMargins;
    const int buttonWidth = Config::Layout::buttonWidth;
    const int loopTextWidth = Config::Layout::loopTextWidth;
    const int clearButtonWidth = Config::Layout::clearButtonWidth;
    const int thresholdEditorWidth = Config::Layout::thresholdEditorWidth;

    auto loopRow = bounds.removeFromTop(rowHeight).reduced(margin);
    
    // Group "In" controls
    controlPanel.loopInButton.setBounds(loopRow.removeFromLeft(buttonWidth));
    loopRow.removeFromLeft(margin);
    controlPanel.loopInEditor.setBounds(loopRow.removeFromLeft(loopTextWidth));
    loopRow.removeFromLeft(margin / 2);
    controlPanel.clearLoopInButton.setBounds(loopRow.removeFromLeft(clearButtonWidth));
    loopRow.removeFromLeft(margin);
    controlPanel.silenceDetector->getInSilenceThresholdEditor().setBounds(loopRow.removeFromLeft(thresholdEditorWidth));
    loopRow.removeFromLeft(margin / 2);
    controlPanel.autoCutInButton.setBounds(loopRow.removeFromLeft(buttonWidth));
    
    // Space between groups
    loopRow.removeFromLeft(margin * 3);
    
    // Group "Out" controls
    controlPanel.loopOutButton.setBounds(loopRow.removeFromLeft(buttonWidth));
    loopRow.removeFromLeft(margin);
    controlPanel.loopOutEditor.setBounds(loopRow.removeFromLeft(loopTextWidth));
    loopRow.removeFromLeft(margin / 2);
    controlPanel.clearLoopOutButton.setBounds(loopRow.removeFromLeft(clearButtonWidth));
    loopRow.removeFromLeft(margin);
    controlPanel.silenceDetector->getOutSilenceThresholdEditor().setBounds(loopRow.removeFromLeft(thresholdEditorWidth));
    loopRow.removeFromLeft(margin / 2);
    controlPanel.autoCutOutButton.setBounds(loopRow.removeFromLeft(buttonWidth));
}

void LayoutManager::layoutBottomRowAndTextDisplay(juce::Rectangle<int>& bounds, int rowHeight)
{
    const int margin = Config::Layout::windowBorderMargins;
    const int buttonWidth = Config::Layout::buttonWidth;
    const int playbackWidth = Config::Layout::Text::playbackWidth;

    auto bottomRow = bounds.removeFromBottom(rowHeight).reduced(margin);
    controlPanel.layoutCache.bottomRowTopY = bottomRow.getY();
    controlPanel.layoutCache.contentAreaBounds = bounds.reduced(margin);
    controlPanel.qualityButton.setBounds(bottomRow.removeFromRight(buttonWidth)); bottomRow.removeFromRight(margin);
    controlPanel.channelViewButton.setBounds(bottomRow.removeFromRight(buttonWidth)); bottomRow.removeFromRight(margin);
    controlPanel.statsButton.setBounds(bottomRow.removeFromRight(buttonWidth)); bottomRow.removeFromRight(margin);
    controlPanel.modeButton.setBounds(bottomRow.removeFromRight(buttonWidth));

    controlPanel.layoutCache.playbackLeftTextX = controlPanel.getLocalBounds().getX() + margin;
    controlPanel.layoutCache.playbackCenterTextX = (controlPanel.getLocalBounds().getWidth() / 2) - (playbackWidth / 2);
    controlPanel.layoutCache.playbackRightTextX = controlPanel.getLocalBounds().getRight() - margin - playbackWidth;

    if (controlPanel.playbackTextPresenter != nullptr)
        controlPanel.playbackTextPresenter->layoutEditors();
}

void LayoutManager::layoutWaveformAndStats(juce::Rectangle<int>& bounds)
{
    const int margin = Config::Layout::windowBorderMargins;
    if (controlPanel.currentMode == AppEnums::ViewMode::Overlay) {
        controlPanel.layoutCache.waveformBounds = controlPanel.getLocalBounds();
    } else {
        controlPanel.layoutCache.waveformBounds = bounds.reduced(margin);
    }

    if (controlPanel.statsPresenter != nullptr)
        controlPanel.statsPresenter->layoutWithin(controlPanel.layoutCache.contentAreaBounds);
}
