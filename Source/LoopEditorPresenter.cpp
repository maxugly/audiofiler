#include "LoopEditorPresenter.h"

#include "ControlPanel.h"
#include "Config.h"
#include "TimeUtils.h"

LoopEditorPresenter::LoopEditorPresenter(ControlPanel& ownerPanel)
    : owner(ownerPanel),
      loopInEditor(owner.loopInEditor),
      loopOutEditor(owner.loopOutEditor)
{
}

LoopEditorPresenter::~LoopEditorPresenter()
{
    loopInEditor.removeListener(this);
    loopOutEditor.removeListener(this);
}

void LoopEditorPresenter::initialiseEditors()
{
    auto configure = [](juce::TextEditor& editor)
    {
        editor.setReadOnly(false);
        editor.setJustification(juce::Justification::centred);
        editor.setColour(juce::TextEditor::backgroundColourId, Config::Colors::textEditorBackground);
        editor.setColour(juce::TextEditor::textColourId, Config::Colors::playbackText);
        editor.setFont(juce::Font(juce::FontOptions(Config::Layout::Text::playbackSize)));
        editor.setMultiLine(false);
        editor.setReturnKeyStartsNewLine(false);
        editor.setWantsKeyboardFocus(true);
    };

    owner.addAndMakeVisible(loopInEditor);
    configure(loopInEditor);
    loopInEditor.addListener(this);

    owner.addAndMakeVisible(loopOutEditor);
    configure(loopOutEditor);
    loopOutEditor.addListener(this);
}

void LoopEditorPresenter::textEditorTextChanged(juce::TextEditor& editor)
{
    const double totalLength = owner.getAudioPlayer().getThumbnail().getTotalLength();
    const double newPosition = TimeUtils::parseTime(editor.getText());

    if (newPosition >= 0.0 && newPosition <= totalLength)
        editor.setColour(juce::TextEditor::textColourId, Config::Colors::playbackText);
    else if (newPosition == -1.0)
        editor.setColour(juce::TextEditor::textColourId, Config::Colors::textEditorError);
    else
        editor.setColour(juce::TextEditor::textColourId, Config::Colors::textEditorWarning);
}

void LoopEditorPresenter::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    applyLoopEdit(editor, &editor == &loopInEditor);
    editor.giveAwayKeyboardFocus();
}

void LoopEditorPresenter::textEditorEscapeKeyPressed(juce::TextEditor& editor)
{
    restoreEditorValue(editor, &editor == &loopInEditor);
    editor.setColour(juce::TextEditor::textColourId, Config::Colors::playbackText);
    editor.giveAwayKeyboardFocus();
}

void LoopEditorPresenter::textEditorFocusLost(juce::TextEditor& editor)
{
    applyLoopEdit(editor, &editor == &loopInEditor);
}

void LoopEditorPresenter::applyLoopEdit(juce::TextEditor& editor, bool isLoopIn)
{
    const double totalLength = owner.getAudioPlayer().getThumbnail().getTotalLength();
    const double newPosition = TimeUtils::parseTime(editor.getText());

    if (newPosition >= 0.0 && newPosition <= totalLength)
    {
        if (isLoopIn)
        {
            if (owner.getLoopOutPosition() > -1.0 && newPosition > owner.getLoopOutPosition())
            {
                restoreEditorValue(editor, true);
                editor.setColour(juce::TextEditor::textColourId, Config::Colors::textEditorWarning);
                return;
            }
            owner.setLoopInPosition(newPosition);
            owner.updateLoopButtonColors();
            owner.silenceDetector->setIsAutoCutInActive(false);
        }
        else
        {
            if (owner.getShouldLoop() && owner.getAudioPlayer().getTransportSource().getCurrentPosition() >= owner.getLoopOutPosition())
                owner.getAudioPlayer().getTransportSource().setPosition(owner.getLoopInPosition());

            if (owner.getLoopInPosition() > -1.0 && newPosition < owner.getLoopInPosition())
            {
                restoreEditorValue(editor, false);
                editor.setColour(juce::TextEditor::textColourId, Config::Colors::textEditorWarning);
                return;
            }
            owner.setLoopOutPosition(newPosition);
        }

        editor.setColour(juce::TextEditor::textColourId, Config::Colors::playbackText);
        owner.ensureLoopOrder();
        owner.updateLoopLabels();
        owner.repaint();
    }
    else
    {
        restoreEditorValue(editor, isLoopIn);
        editor.setColour(juce::TextEditor::textColourId, Config::Colors::textEditorError);
        owner.repaint();
    }
}

void LoopEditorPresenter::restoreEditorValue(juce::TextEditor& editor, bool isLoopIn)
{
    const double value = isLoopIn ? owner.getLoopInPosition() : owner.getLoopOutPosition();
    editor.setText(owner.formatTime(value), juce::dontSendNotification);
}


