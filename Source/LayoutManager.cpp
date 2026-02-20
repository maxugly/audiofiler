

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
    const int rowHeight = (int)Config::UI::WidgetHeight + margin * 2;

    layoutTopRowButtons(bounds, rowHeight);

    layoutCutControls(bounds, rowHeight);

    layoutBottomRowAndTextDisplay(bounds, rowHeight);

    layoutWaveformAndStats(bounds);
}

void LayoutManager::layoutTopRowButtons(juce::Rectangle<int>& bounds, int rowHeight)
{
    juce::ignoreUnused(rowHeight);
    const int margin = Config::Layout::windowBorderMargins;
    const int buttonWidth = Config::Layout::buttonWidth;
    const int height = (int)Config::UI::WidgetHeight;
    const int spacing = (int)Config::UI::GroupSpacing;

    auto topRow = bounds.removeFromTop(height + margin * 2).reduced(margin);
    topRow.setHeight(height);

    // Open button (Alone)
    controlPanel.openButton.setBounds(topRow.removeFromLeft(buttonWidth));
    topRow.removeFromLeft(margin);

    // Transport Group (Left, Middle, Middle, Middle, Right)
    controlPanel.playStopButton.setBounds(topRow.removeFromLeft(buttonWidth));
    topRow.removeFromLeft(spacing);

    controlPanel.stopButton.setBounds(topRow.removeFromLeft(buttonWidth));
    topRow.removeFromLeft(spacing);

    controlPanel.autoplayButton.setBounds(topRow.removeFromLeft(buttonWidth));
    topRow.removeFromLeft(spacing);

    controlPanel.repeatButton.setBounds(topRow.removeFromLeft(buttonWidth));
    topRow.removeFromLeft(spacing);

    controlPanel.cutButton.setBounds(topRow.removeFromLeft(buttonWidth));
    topRow.removeFromLeft(margin);

    controlPanel.exitButton.setBounds(topRow.removeFromRight(buttonWidth));
    topRow.removeFromRight(margin);

    // View Group (Left, Middle, Right) - Grouped and right-aligned
    controlPanel.channelViewButton.setBounds(topRow.removeFromRight(buttonWidth));
    topRow.removeFromRight(spacing);
    controlPanel.statsButton.setBounds(topRow.removeFromRight(buttonWidth));
    topRow.removeFromRight(spacing);
    controlPanel.modeButton.setBounds(topRow.removeFromRight(buttonWidth));
}

void LayoutManager::layoutCutControls(juce::Rectangle<int>& bounds, int rowHeight)
{
    juce::ignoreUnused(rowHeight);
    const int margin = Config::Layout::windowBorderMargins;
    const float unit = Config::UI::WidgetUnit;
    const int spacing = (int)Config::UI::GroupSpacing;
    const int height = (int)Config::UI::WidgetHeight;

    auto cutRow = bounds.removeFromTop(height + margin * 2).reduced(margin);
    cutRow.setHeight(height);

    // In Strip: [In(L)] [Timer] [Reset] [Threshold] [AutoCut(R)]
    controlPanel.cutInButton.setBounds(cutRow.removeFromLeft((int)(2.0f * unit)));
    cutRow.removeFromLeft(spacing);
    controlPanel.cutInEditor.setBounds(cutRow.removeFromLeft((int)(5.5f * unit)));
    cutRow.removeFromLeft(spacing);
    controlPanel.resetInButton.setBounds(cutRow.removeFromLeft((int)(1.0f * unit)));
    cutRow.removeFromLeft(spacing);
    controlPanel.getSilenceDetector().getInSilenceThresholdEditor().setBounds(cutRow.removeFromLeft((int)(1.5f * unit)));
    cutRow.removeFromLeft(spacing);
    controlPanel.autoCutInButton.setBounds(cutRow.removeFromLeft((int)(2.0f * unit)));

    // Out Strip (Right-Aligned & Mirrored): [AutoCut(L)] [Threshold] [Reset] [Timer] [Out(R)]
    controlPanel.cutOutButton.setBounds(cutRow.removeFromRight((int)(2.0f * unit)));
    cutRow.removeFromRight(spacing);
    controlPanel.cutOutEditor.setBounds(cutRow.removeFromRight((int)(5.5f * unit)));
    cutRow.removeFromRight(spacing);
    controlPanel.resetOutButton.setBounds(cutRow.removeFromRight((int)(1.0f * unit)));
    cutRow.removeFromRight(spacing);
    controlPanel.getSilenceDetector().getOutSilenceThresholdEditor().setBounds(cutRow.removeFromRight((int)(1.5f * unit)));
    cutRow.removeFromRight(spacing);
    controlPanel.autoCutOutButton.setBounds(cutRow.removeFromRight((int)(2.0f * unit)));
}

void LayoutManager::layoutBottomRowAndTextDisplay(juce::Rectangle<int>& bounds, int rowHeight)
{
    const int margin = Config::Layout::windowBorderMargins;
    const int buttonWidth = Config::Layout::buttonWidth;
    const int playbackWidth = Config::Layout::Text::playbackWidth;

    auto bottomRow = bounds.removeFromBottom(rowHeight).reduced(margin);
    controlPanel.layoutCache.bottomRowTopY = bottomRow.getY();
    controlPanel.layoutCache.contentAreaBounds = bounds.reduced(margin);

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
