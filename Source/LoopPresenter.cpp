#include "LoopPresenter.h"

#include "ControlPanel.h"
#include "SilenceDetector.h"
#include "AudioPlayer.h"
#include "Config.h"
#include <utility>

LoopPresenter::LoopPresenter(ControlPanel& ownerPanel,
                             SilenceDetector& detector,
                             juce::TextEditor& loopIn,
                             juce::TextEditor& loopOut)
    : owner(ownerPanel),
      silenceDetector(detector),
      loopInEditor(loopIn),
      loopOutEditor(loopOut)
{
    loopInEditor.addListener(this);
    loopOutEditor.addListener(this);
}

LoopPresenter::~LoopPresenter()
{
    loopInEditor.removeListener(this);
    loopOutEditor.removeListener(this);
}

void LoopPresenter::setLoopInPosition(double positionSeconds)
{
    loopInPosition = positionSeconds;
}

void LoopPresenter::setLoopOutPosition(double positionSeconds)
{
    loopOutPosition = positionSeconds;
}

void LoopPresenter::ensureLoopOrder()
{
    if (loopInPosition > loopOutPosition)
        std::swap(loopInPosition, loopOutPosition);
}

void LoopPresenter::updateLoopLabels()
{
    syncEditorToPosition(loopInEditor, loopInPosition);
    syncEditorToPosition(loopOutEditor, loopOutPosition);
}

void LoopPresenter::setLoopStartFromSample(int sampleIndex)
{
    AudioPlayer& audioPlayer = owner.getAudioPlayer();
    if (audioPlayer.getAudioFormatReader() != nullptr)
    {
        const double sampleRate = audioPlayer.getAudioFormatReader()->sampleRate;
        setLoopInPosition((double) sampleIndex / sampleRate);
        ensureLoopOrder();
        updateLoopLabels();
        owner.repaint();
    }
}

void LoopPresenter::setLoopEndFromSample(int sampleIndex)
{
    AudioPlayer& audioPlayer = owner.getAudioPlayer();
    if (audioPlayer.getAudioFormatReader() != nullptr)
    {
        const double sampleRate = audioPlayer.getAudioFormatReader()->sampleRate;
        setLoopOutPosition((double) sampleIndex / sampleRate);
        ensureLoopOrder();
        updateLoopLabels();
        owner.repaint();
    }
}

void LoopPresenter::textEditorTextChanged(juce::TextEditor& editor)
{
    const double totalLength = getAudioTotalLength();
    const double newPosition = parseTime(editor.getText());

    if (newPosition >= 0.0 && newPosition <= totalLength)
    {
        editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
    }
    else if (newPosition == -1.0)
    {
        editor.setColour(juce::TextEditor::textColourId, Config::textEditorErrorColor);
    }
    else
    {
        editor.setColour(juce::TextEditor::textColourId, Config::textEditorWarningColor);
    }
}

void LoopPresenter::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    const double newPosition = parseTime(editor.getText());
    if (&editor == &loopInEditor)
    {
        applyLoopInFromEditor(newPosition, editor);
    }
    else if (&editor == &loopOutEditor)
    {
        applyLoopOutFromEditor(newPosition, editor);
    }
    editor.giveAwayKeyboardFocus();
}

void LoopPresenter::textEditorEscapeKeyPressed(juce::TextEditor& editor)
{
    if (&editor == &loopInEditor)
    {
        syncEditorToPosition(editor, loopInPosition);
    }
    else if (&editor == &loopOutEditor)
    {
        syncEditorToPosition(editor, loopOutPosition);
    }
    editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
    editor.giveAwayKeyboardFocus();
}

void LoopPresenter::textEditorFocusLost(juce::TextEditor& editor)
{
    const double newPosition = parseTime(editor.getText());
    if (&editor == &loopInEditor)
    {
        applyLoopInFromEditor(newPosition, editor);
    }
    else if (&editor == &loopOutEditor)
    {
        applyLoopOutFromEditor(newPosition, editor);
    }
}

double LoopPresenter::parseTime(const juce::String& timeString) const
{
    auto parts = juce::StringArray::fromTokens(timeString, ":", "");
    if (parts.size() != 4)
        return -1.0;

    return parts[0].getIntValue() * 3600.0
         + parts[1].getIntValue() * 60.0
         + parts[2].getIntValue()
         + parts[3].getIntValue() / 1000.0;
}

double LoopPresenter::getAudioTotalLength() const
{
    return owner.getAudioPlayer().getThumbnail().getTotalLength();
}

bool LoopPresenter::applyLoopInFromEditor(double newPosition, juce::TextEditor& editor)
{
    const double totalLength = getAudioTotalLength();
    if (newPosition >= 0.0 && newPosition <= totalLength)
    {
        if (loopOutPosition > -1.0 && newPosition > loopOutPosition)
        {
            syncEditorToPosition(editor, loopInPosition);
            editor.setColour(juce::TextEditor::textColourId, Config::textEditorWarningColor);
            return false;
        }

        setLoopInPosition(newPosition);
        owner.updateLoopButtonColors();
        silenceDetector.setIsAutoCutInActive(false);
        editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
        owner.repaint();
        return true;
    }

    syncEditorToPosition(editor, loopInPosition);
    editor.setColour(juce::TextEditor::textColourId, Config::textEditorErrorColor);
    owner.repaint();
    return false;
}

bool LoopPresenter::applyLoopOutFromEditor(double newPosition, juce::TextEditor& editor)
{
    const double totalLength = getAudioTotalLength();
    if (newPosition >= 0.0 && newPosition <= totalLength)
    {
        AudioPlayer& audioPlayer = owner.getAudioPlayer();
        auto& transport = audioPlayer.getTransportSource();
        if (owner.getShouldLoop() && transport.getCurrentPosition() >= loopOutPosition)
            transport.setPosition(loopInPosition);

        if (loopInPosition > -1.0 && newPosition < loopInPosition)
        {
            syncEditorToPosition(editor, loopOutPosition);
            editor.setColour(juce::TextEditor::textColourId, Config::textEditorWarningColor);
            return false;
        }

        setLoopOutPosition(newPosition);
        owner.updateLoopButtonColors();
        editor.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
        owner.repaint();
        return true;
    }

    syncEditorToPosition(editor, loopOutPosition);
    editor.setColour(juce::TextEditor::textColourId, Config::textEditorErrorColor);
    owner.repaint();
    return false;
}

void LoopPresenter::syncEditorToPosition(juce::TextEditor& editor, double positionSeconds)
{
    editor.setText(owner.formatTime(positionSeconds), juce::dontSendNotification);
}
