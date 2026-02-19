/**
 * @file LayoutManager.cpp
 * @brief Defines the LayoutManager class.
 * @ingroup Engine
 */

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

    /**
     * @brief Undocumented method.
     * @param bounds [in] Description for bounds.
     * @param rowHeight [in] Description for rowHeight.
     */
    layoutTopRowButtons(bounds, rowHeight);
    /**
     * @brief Undocumented method.
     * @param bounds [in] Description for bounds.
     * @param rowHeight [in] Description for rowHeight.
     */
    layoutCutControls(bounds, rowHeight);
    /**
     * @brief Undocumented method.
     * @param bounds [in] Description for bounds.
     * @param rowHeight [in] Description for rowHeight.
     */
    layoutBottomRowAndTextDisplay(bounds, rowHeight);
    /**
     * @brief Undocumented method.
     * @param bounds [in] Description for bounds.
     */
    layoutWaveformAndStats(bounds);
}

void LayoutManager::layoutTopRowButtons(juce::Rectangle<int>& bounds, int rowHeight)
{
    const int margin = Config::Layout::windowBorderMargins;
    const int buttonWidth = Config::Layout::buttonWidth;

    auto topRow = bounds.removeFromTop(rowHeight).reduced(margin);

    controlPanel.openButton.setBounds(topRow.removeFromLeft(buttonWidth));
    topRow.removeFromLeft(margin);

    controlPanel.playStopButton.setBounds(topRow.removeFromLeft(buttonWidth));
    topRow.removeFromLeft(margin);

    controlPanel.autoplayButton.setBounds(topRow.removeFromLeft(buttonWidth));
    topRow.removeFromLeft(margin);

    controlPanel.repeatButton.setBounds(topRow.removeFromLeft(buttonWidth));
    topRow.removeFromLeft(margin);

    controlPanel.cutButton.setBounds(topRow.removeFromLeft(buttonWidth));
    topRow.removeFromLeft(margin);

    controlPanel.exitButton.setBounds(topRow.removeFromRight(buttonWidth));
    topRow.removeFromRight(margin);
}

void LayoutManager::layoutCutControls(juce::Rectangle<int>& bounds, int rowHeight)
{
    const int margin = Config::Layout::windowBorderMargins;
    const int buttonWidth = Config::Layout::buttonWidth;
    const int cutTextWidth = Config::Layout::cutTextWidth;
    const int clearButtonWidth = Config::Layout::clearButtonWidth;
    const int thresholdEditorWidth = Config::Layout::thresholdEditorWidth;

    auto cutRow = bounds.removeFromTop(rowHeight).reduced(margin);
    
    
    controlPanel.cutInButton.setBounds(cutRow.removeFromLeft(buttonWidth));
    cutRow.removeFromLeft(margin);
    controlPanel.cutInEditor.setBounds(cutRow.removeFromLeft(cutTextWidth));
    cutRow.removeFromLeft(margin / 2);
    controlPanel.resetInButton.setBounds(cutRow.removeFromLeft(clearButtonWidth));
    cutRow.removeFromLeft(margin);
    controlPanel.getSilenceDetector().getInSilenceThresholdEditor().setBounds(cutRow.removeFromLeft(thresholdEditorWidth));
    cutRow.removeFromLeft(margin / 2);
    controlPanel.autoCutInButton.setBounds(cutRow.removeFromLeft(buttonWidth));
    
    
    cutRow.removeFromLeft(margin * 3);
    
    
    controlPanel.cutOutButton.setBounds(cutRow.removeFromLeft(buttonWidth));
    cutRow.removeFromLeft(margin);
    controlPanel.cutOutEditor.setBounds(cutRow.removeFromLeft(cutTextWidth));
    cutRow.removeFromLeft(margin / 2);
    controlPanel.resetOutButton.setBounds(cutRow.removeFromLeft(clearButtonWidth));
    cutRow.removeFromLeft(margin);
    controlPanel.getSilenceDetector().getOutSilenceThresholdEditor().setBounds(cutRow.removeFromLeft(thresholdEditorWidth));
    cutRow.removeFromLeft(margin / 2);
    controlPanel.autoCutOutButton.setBounds(cutRow.removeFromLeft(buttonWidth));
}

void LayoutManager::layoutBottomRowAndTextDisplay(juce::Rectangle<int>& bounds, int rowHeight)
{
    const int margin = Config::Layout::windowBorderMargins;
    const int buttonWidth = Config::Layout::buttonWidth;
    const int playbackWidth = Config::Layout::Text::playbackWidth;

    auto bottomRow = bounds.removeFromBottom(rowHeight).reduced(margin);
    controlPanel.layoutCache.bottomRowTopY = bottomRow.getY();
    controlPanel.layoutCache.contentAreaBounds = bounds.reduced(margin);

    controlPanel.qualityButton.setBounds(bottomRow.removeFromRight(buttonWidth));
    bottomRow.removeFromRight(margin);

    controlPanel.channelViewButton.setBounds(bottomRow.removeFromRight(buttonWidth));
    bottomRow.removeFromRight(margin);

    controlPanel.statsButton.setBounds(bottomRow.removeFromRight(buttonWidth));
    bottomRow.removeFromRight(margin);

    controlPanel.modeButton.setBounds(bottomRow.removeFromRight(buttonWidth));

    const auto fullBounds = controlPanel.getLocalBounds();
    controlPanel.layoutCache.playbackLeftTextX = fullBounds.getX() + margin;
    controlPanel.layoutCache.playbackCenterTextX = (fullBounds.getWidth() / 2) - (playbackWidth / 2);
    controlPanel.layoutCache.playbackRightTextX = fullBounds.getRight() - margin - playbackWidth;

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
