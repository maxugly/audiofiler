

#include "UI/LayoutManager.h"

#include "UI/ControlPanel.h"
#include "Utils/Config.h"
#include "Workers/SilenceDetector.h"
#include "Presenters/StatsPresenter.h"
#include "Presenters/PlaybackTextPresenter.h"

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

    // Transport Group (Modular TransportStrip)
    if (auto* ts = controlPanel.getTransportStrip()) {
        const int stripWidth = (buttonWidth * 5) + (spacing * 4);
        ts->setBounds(topRow.removeFromLeft(stripWidth));
    }
    topRow.removeFromLeft(margin);

    controlPanel.exitButton.setBounds(topRow.removeFromRight(buttonWidth));
    topRow.removeFromRight(margin);

    // View Group (Left, Middle, Right) - Grouped and right-aligned
    controlPanel.channelViewButton.setBounds(topRow.removeFromRight(buttonWidth));
    topRow.removeFromRight(spacing);
    controlPanel.statsButton.setBounds(topRow.removeFromRight(buttonWidth));
    topRow.removeFromRight(spacing);
    controlPanel.modeButton.setBounds(topRow.removeFromRight(buttonWidth));

    // Eye Candy Toggle (Alone) - Left of View Group
    topRow.removeFromRight(margin);
    controlPanel.eyeCandyButton.setBounds(topRow.removeFromRight((int)Config::UI::WidgetUnit));
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

    const int stripWidth = (int)((Config::UI::CutButtonWidthUnits * 2 + 
                                  Config::UI::TimerWidthUnits + 
                                  Config::UI::ResetButtonWidthUnits + 
                                  Config::UI::ThresholdWidthUnits) * unit) + (spacing * 4);

    if (controlPanel.inStrip != nullptr)
        controlPanel.inStrip->setBounds(cutRow.removeFromLeft(stripWidth));

    if (controlPanel.outStrip != nullptr)
        controlPanel.outStrip->setBounds(cutRow.removeFromRight(stripWidth));
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
