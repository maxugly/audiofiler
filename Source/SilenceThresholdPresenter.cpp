#include "SilenceThresholdPresenter.h"

#include "SilenceDetector.h"
#include "ControlPanel.h"
#include "Config.h"
#include "ControlPanelCopy.h"

SilenceThresholdPresenter::SilenceThresholdPresenter(SilenceDetector& detectorIn,
                                                     ControlPanel& ownerPanel)
    : detector(detectorIn),
      owner(ownerPanel)
{
    const auto& autoCut = owner.getSessionState().getCutPrefs().autoCut;
    detector.currentInSilenceThreshold = autoCut.thresholdIn;
    detector.currentOutSilenceThreshold = autoCut.thresholdOut;

    configureEditor(detector.inSilenceThresholdEditor,
                    detector.currentInSilenceThreshold,
                    ControlPanelCopy::silenceThresholdInTooltip());
    configureEditor(detector.outSilenceThresholdEditor,
                    detector.currentOutSilenceThreshold,
                    ControlPanelCopy::silenceThresholdOutTooltip());
}

SilenceThresholdPresenter::~SilenceThresholdPresenter()
{
    detector.inSilenceThresholdEditor.removeListener(this);
    detector.outSilenceThresholdEditor.removeListener(this);
}

void SilenceThresholdPresenter::configureEditor(juce::TextEditor& editor,
                                                float initialValue,
                                                const juce::String& tooltip)
{
    editor.setText(juce::String(static_cast<int>(initialValue * 100.0f)));
    editor.setReadOnly(false);
    editor.setJustification(juce::Justification::centred);
    editor.setColour(juce::TextEditor::backgroundColourId, Config::Colors::textEditorBackground);
    editor.setColour(juce::TextEditor::textColourId, Config::Colors::playbackText);
    editor.setFont(juce::Font(juce::FontOptions(Config::Layout::Text::playbackSize)));
    editor.applyFontToAllText(editor.getFont());
    editor.setMultiLine(false);
    editor.setReturnKeyStartsNewLine(false);
    editor.addListener(this);
    editor.setWantsKeyboardFocus(true);
    editor.setTooltip(tooltip);
    editor.setSelectAllWhenFocused(true);
}

void SilenceThresholdPresenter::textEditorTextChanged(juce::TextEditor& editor)
{
    const int newPercentage = editor.getText().getIntValue();
    if (isValidPercentage(newPercentage))
        editor.setColour(juce::TextEditor::textColourId, Config::Colors::playbackText);
    else
        editor.setColour(juce::TextEditor::textColourId, Config::Colors::textEditorOutOfRange);

    editor.setColour(juce::TextEditor::backgroundColourId,
                     owner.getLookAndFeel().findColour(juce::TextEditor::backgroundColourId));

    updateThresholdFromEditorIfValid(editor);
}

void SilenceThresholdPresenter::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    applyThresholdFromEditor(editor);
}

void SilenceThresholdPresenter::textEditorFocusLost(juce::TextEditor& editor)
{
    applyThresholdFromEditor(editor);
}

void SilenceThresholdPresenter::applyThresholdFromEditor(juce::TextEditor& editor)
{
    const int intValue = editor.getText().getIntValue();

    if (isValidPercentage(intValue))
    {
        updateThresholdFromEditorIfValid(editor);

        editor.setColour(juce::TextEditor::textColourId, Config::Colors::playbackText);
        editor.setColour(juce::TextEditor::backgroundColourId,
                         owner.getLookAndFeel().findColour(juce::TextEditor::backgroundColourId));
        editor.setText(juce::String(intValue), juce::dontSendNotification);
    }
    else
    {
        restoreEditorToCurrentValue(editor);
        editor.setColour(juce::TextEditor::textColourId, Config::Colors::textEditorWarning);
        owner.getStatsDisplay().insertTextAtCaret("Warning: Threshold value must be between 1 and 99. Restored to last valid value.\n");
    }
}

void SilenceThresholdPresenter::updateThresholdFromEditorIfValid(juce::TextEditor& editor)
{
    const int intValue = editor.getText().getIntValue();
    if (!isValidPercentage(intValue))
        return;

    const float normalized = static_cast<float>(intValue) / 100.0f;
    if (isInEditor(editor))
    {
        if (detector.currentInSilenceThreshold == normalized)
            return;
        detector.currentInSilenceThreshold = normalized;
        owner.getSessionState().setThresholdIn(normalized);
        if (detector.getIsAutoCutInActive())
            detector.detectInSilence();
    }
    else
    {
        if (detector.currentOutSilenceThreshold == normalized)
            return;
        detector.currentOutSilenceThreshold = normalized;
        owner.getSessionState().setThresholdOut(normalized);
        if (detector.getIsAutoCutOutActive())
            detector.detectOutSilence();
    }
}

void SilenceThresholdPresenter::restoreEditorToCurrentValue(juce::TextEditor& editor)
{
    const float currentValue = isInEditor(editor)
        ? detector.currentInSilenceThreshold
        : detector.currentOutSilenceThreshold;
    editor.setText(juce::String(static_cast<int>(currentValue * 100.0f)), juce::dontSendNotification);
}

bool SilenceThresholdPresenter::isInEditor(const juce::TextEditor& editor) const noexcept
{
    return &editor == &detector.inSilenceThresholdEditor;
}
