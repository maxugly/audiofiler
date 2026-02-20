#include "UI/Components/MarkerStrip.h"
#include "ControlPanelCopy.h"
#include "Core/AppEnums.h"
#include "TimeUtils.h"

MarkerStrip::MarkerStrip(MarkerType type, AudioPlayer& player, SessionState& state, SilenceDetector& detector)
    : markerType(type), audioPlayer(player), sessionState(state), silenceDetector(detector)
{
    initialiseComponents();
}

void MarkerStrip::initialiseComponents()
{
    // Marker Button (In/Out)
    addAndMakeVisible(markerButton);
    markerButton.setButtonText(markerType == MarkerType::In ? ControlPanelCopy::cutInButtonText() : ControlPanelCopy::cutOutButtonText());
    markerButton.getProperties().set("GroupPosition", (int)(markerType == MarkerType::In ? AppEnums::GroupPosition::Left : AppEnums::GroupPosition::Right));
    markerButton.onLeftClick = [this] {
        if (markerType == MarkerType::In)
            sessionState.setCutIn(audioPlayer.getCurrentPosition());
        else
            sessionState.setCutOut(audioPlayer.getCurrentPosition());
        
        updateAutoCutState(false);
    };
    markerButton.onRightClick = [this] {
        if (onMarkerRightClick)
            onMarkerRightClick();
    };

    // Timer Editor
    addAndMakeVisible(timerEditor);
    // Style will be applied by RepeatPresenter::initialiseEditors()
    timerEditor.getProperties().set("GroupPosition", (int)AppEnums::GroupPosition::Middle);

    // Reset Button
    addAndMakeVisible(resetButton);
    resetButton.setButtonText(ControlPanelCopy::clearButtonText());
    resetButton.getProperties().set("GroupPosition", (int)AppEnums::GroupPosition::Middle);
    resetButton.setColour(juce::TextButton::buttonColourId, Config::Colors::Button::clear);
    resetButton.onClick = [this] {
        if (markerType == MarkerType::In) {
            sessionState.setCutIn(0.0);
            sessionState.setAutoCutInActive(false);
        } else {
            sessionState.setCutOut(audioPlayer.getThumbnail().getTotalLength());
            sessionState.setAutoCutOutActive(false);
        }
    };

    // Threshold Editor (percentage input)
    auto& detectorEditor = (markerType == MarkerType::In) ? silenceDetector.getInSilenceThresholdEditor() : silenceDetector.getOutSilenceThresholdEditor();
    addAndMakeVisible(detectorEditor);
    detectorEditor.getProperties().set("GroupPosition", (int)AppEnums::GroupPosition::Middle);

    // AutoCut Button
    addAndMakeVisible(autoCutButton);
    autoCutButton.setButtonText(markerType == MarkerType::In ? ControlPanelCopy::autoCutInButtonText() : ControlPanelCopy::autoCutOutButtonText());
    autoCutButton.getProperties().set("GroupPosition", (int)(markerType == MarkerType::In ? AppEnums::GroupPosition::Right : AppEnums::GroupPosition::Left));
    autoCutButton.setClickingTogglesState(true);
    autoCutButton.onClick = [this] {
        if (markerType == MarkerType::In)
            sessionState.setAutoCutInActive(autoCutButton.getToggleState());
        else
            sessionState.setAutoCutOutActive(autoCutButton.getToggleState());
    };
}

void MarkerStrip::resized()
{
    auto b = getLocalBounds();
    const float unit = Config::UI::WidgetUnit;
    const int spacing = (int)Config::UI::GroupSpacing;
    auto& detectorEditor = (markerType == MarkerType::In) ? silenceDetector.getInSilenceThresholdEditor() : silenceDetector.getOutSilenceThresholdEditor();

    const int markerWidth = (int)(Config::UI::CutButtonWidthUnits * unit);
    const int timerWidth = (int)(Config::UI::TimerWidthUnits * unit);
    const int resetWidth = (int)(Config::UI::ResetButtonWidthUnits * unit);
    const int thresholdWidth = (int)(Config::UI::ThresholdWidthUnits * unit);
    const int autoCutWidth = (int)(Config::UI::CutButtonWidthUnits * unit);

    if (markerType == MarkerType::In)
    {
        // [In(L), Timer, Reset, Threshold, AutoCut(R)]
        markerButton.setBounds(b.removeFromLeft(markerWidth));
        b.removeFromLeft(spacing);
        timerEditor.setBounds(b.removeFromLeft(timerWidth));
        b.removeFromLeft(spacing);
        resetButton.setBounds(b.removeFromLeft(resetWidth));
        b.removeFromLeft(spacing);
        detectorEditor.setBounds(b.removeFromLeft(thresholdWidth));
        b.removeFromLeft(spacing);
        autoCutButton.setBounds(b.removeFromLeft(autoCutWidth));
    }
    else
    {
        // [AutoCut(L), Threshold, Reset, Timer, Out(R)]
        markerButton.setBounds(b.removeFromRight(markerWidth));
        b.removeFromRight(spacing);
        timerEditor.setBounds(b.removeFromRight(timerWidth));
        b.removeFromRight(spacing);
        resetButton.setBounds(b.removeFromRight(resetWidth));
        b.removeFromRight(spacing);
        detectorEditor.setBounds(b.removeFromRight(thresholdWidth));
        b.removeFromRight(spacing);
        autoCutButton.setBounds(b.removeFromRight(autoCutWidth));
    }
}

void MarkerStrip::updateTimerText(const juce::String& text)
{
    timerEditor.setText(text, juce::dontSendNotification);
}

void MarkerStrip::updateAutoCutState(bool isActive)
{
    autoCutButton.setToggleState(isActive, juce::dontSendNotification);
}

void MarkerStrip::updateMarkerButtonColor(juce::Colour color)
{
    markerButton.setColour(juce::TextButton::buttonColourId, color);
}
