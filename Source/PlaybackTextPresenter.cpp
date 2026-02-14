#include "PlaybackTextPresenter.h"

#include "ControlPanel.h"
#include "Config.h"
#include "AudioPlayer.h"
#include "TimeUtils.h"
#include <cmath>

PlaybackTextPresenter::PlaybackTextPresenter(ControlPanel& ownerPanel)
    : owner(ownerPanel)
{
}

PlaybackTextPresenter::~PlaybackTextPresenter()
{
    owner.elapsedTimeEditor.removeListener(this);
    owner.remainingTimeEditor.removeListener(this);
    owner.loopLengthEditor.removeListener(this);
}

void PlaybackTextPresenter::initialiseEditors()
{
    auto configure = [this](juce::TextEditor& ed, juce::Justification justification) {
        ed.setJustification(justification);
        ed.setColour(juce::TextEditor::backgroundColourId, Config::Colors::textEditorBackground);
        ed.setColour(juce::TextEditor::textColourId, Config::Colors::playbackText);
        ed.setColour(juce::TextEditor::outlineColourId, Config::Colors::playbackText.withAlpha(0.5f));
        ed.setColour(juce::TextEditor::focusedOutlineColourId, Config::Colors::playbackText);
        ed.setFont(juce::Font(juce::FontOptions((float)Config::Layout::Text::playbackSize)));
        ed.applyFontToAllText(ed.getFont());
        ed.setMultiLine(false);
        ed.setReturnKeyStartsNewLine(false);
        ed.addListener(this);
        ed.addMouseListener(this, false);
    };

    configure(owner.elapsedTimeEditor, juce::Justification::left);
    configure(owner.remainingTimeEditor, juce::Justification::right);
    configure(owner.loopLengthEditor, juce::Justification::centred);
}

void PlaybackTextPresenter::updateEditors()
{
    if (!owner.elapsedTimeEditor.hasKeyboardFocus(false))
        syncEditorToPosition(owner.elapsedTimeEditor, owner.getAudioPlayer().getTransportSource().getCurrentPosition());

    if (!owner.remainingTimeEditor.hasKeyboardFocus(false))
    {
        const auto total = owner.getAudioPlayer().getThumbnail().getTotalLength();
        const auto remaining = juce::jmax(0.0, total - owner.getAudioPlayer().getTransportSource().getCurrentPosition());
        syncEditorToPosition(owner.remainingTimeEditor, remaining, true);
    }

    if (!owner.loopLengthEditor.hasKeyboardFocus(false))
    {
        double length = std::abs(owner.getLoopOutPosition() - owner.getLoopInPosition());
        owner.loopLengthEditor.setText(owner.formatTime(length), juce::dontSendNotification);
    }
}

void PlaybackTextPresenter::layoutEditors()
{
    const int textY = owner.getBottomRowTopY() - Config::Layout::Text::playbackOffsetY;
    auto [leftX, centreX, rightX] = owner.getPlaybackLabelXs();

    owner.elapsedTimeEditor.setBounds(leftX, textY, Config::Layout::Text::playbackWidth, Config::Layout::Text::playbackHeight);
    owner.remainingTimeEditor.setBounds(rightX, textY, Config::Layout::Text::playbackWidth, Config::Layout::Text::playbackHeight);
    
    // The center is special because it's shared with total time which is static
    owner.loopLengthEditor.setBounds(centreX, textY, Config::Layout::Text::playbackWidth / 2, Config::Layout::Text::playbackHeight);
}

void PlaybackTextPresenter::render(juce::Graphics& g) const
{
    if (owner.getAudioPlayer().getThumbnail().getTotalLength() <= 0.0)
        return;

    // Draw the static total time part behind or next to the loop length
    const int textY = owner.getBottomRowTopY() - Config::Layout::Text::playbackOffsetY;
    auto [leftX, centreX, rightX] = owner.getPlaybackLabelXs();

    g.setColour(Config::Colors::playbackText);
    g.setFont((float)Config::Layout::Text::playbackSize);
    
    juce::String totalTimeStr = " / " + getTotalTimeStaticString();
    
    // Position it relative to the loop length editor
    int loopLenX = owner.loopLengthEditor.getX();
    int loopLenW = owner.loopLengthEditor.getWidth();
    
    // We want to draw it centered-ish with the loop length
    // But since loopLength is its own editor, let's just draw totalTime to the right of centreX + playbackTextWidth/2
    g.drawText(totalTimeStr, centreX + (Config::Layout::Text::playbackWidth / 2), textY, Config::Layout::Text::playbackWidth / 2, Config::Layout::Text::playbackHeight, juce::Justification::left, false);
}

void PlaybackTextPresenter::textEditorTextChanged(juce::TextEditor& editor)
{
    const double totalLength = owner.getAudioPlayer().getThumbnail().getTotalLength();
    const double newPosition = TimeUtils::parseTime(editor.getText());

    if (newPosition >= 0.0 && newPosition <= totalLength)
    {
        editor.setColour(juce::TextEditor::textColourId, Config::Colors::playbackText);
    }
    else if (newPosition == -1.0)
    {
        editor.setColour(juce::TextEditor::textColourId, Config::Colors::textEditorError);
    }
    else
    {
        editor.setColour(juce::TextEditor::textColourId, Config::Colors::textEditorWarning);
    }
}

void PlaybackTextPresenter::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    applyTimeEdit(editor);
    editor.giveAwayKeyboardFocus();
}

void PlaybackTextPresenter::textEditorEscapeKeyPressed(juce::TextEditor& editor)
{
    updateEditors(); // Reset to current state
    editor.giveAwayKeyboardFocus();
}

void PlaybackTextPresenter::textEditorFocusLost(juce::TextEditor& editor)
{
    applyTimeEdit(editor);
}

void PlaybackTextPresenter::textEditorFocusGained(juce::TextEditor&)
{
}

void PlaybackTextPresenter::applyTimeEdit(juce::TextEditor& editor)
{
    double newTime = TimeUtils::parseTime(editor.getText());
    if (newTime < 0.0) return;

    auto& transport = owner.getAudioPlayer().getTransportSource();
    double totalLength = owner.getAudioPlayer().getThumbnail().getTotalLength();

    if (&editor == &owner.elapsedTimeEditor)
    {
        transport.setPosition(juce::jlimit(0.0, totalLength, newTime));
    }
    else if (&editor == &owner.remainingTimeEditor)
    {
        transport.setPosition(juce::jlimit(0.0, totalLength, totalLength - newTime));
    }
    else if (&editor == &owner.loopLengthEditor)
    {
        // Adjust loop out based on loop in
        double currentIn = owner.getLoopInPosition();

        // Clamp length to total length to prevent issues if loopIn is 0
        newTime = juce::jlimit(0.0, totalLength, newTime);

        double proposedOut = currentIn + newTime;

        if (proposedOut > totalLength)
        {
             // Shift In backwards so that [In, Out] has length newTime and Out <= totalLength
             double newIn = totalLength - newTime;
             owner.setLoopInPosition(newIn);
             owner.setLoopOutPosition(totalLength);
        }
        else
        {
             owner.setLoopOutPosition(proposedOut);
        }

        owner.ensureLoopOrder();
        owner.updateLoopLabels();
    }
    
    updateEditors();
}

void PlaybackTextPresenter::syncEditorToPosition(juce::TextEditor& editor, double positionSeconds, bool isRemaining)
{
    juce::String text = owner.formatTime(positionSeconds);
    if (isRemaining) text = "-" + text;
    editor.setText(text, juce::dontSendNotification);
}

void PlaybackTextPresenter::mouseUp(const juce::MouseEvent& event)
{
    auto* editor = dynamic_cast<juce::TextEditor*>(event.eventComponent);
    if (editor == nullptr) return;

    // Only apply smart highlight if the user hasn't made a manual selection
    if (editor->getHighlightedRegion().getLength() > 0)
        return;

    // Check if it's remaining time (starts with '-')
    bool isNegative = (editor == &owner.remainingTimeEditor) || editor->getText().startsWith("-");
    int offset = isNegative ? 1 : 0;

    int charIndex = editor->getTextIndexAt(event.getPosition());
    if (charIndex < 0) return;

    int effectiveIndex = charIndex - offset;

    // Time format: [opt -]HH:MM:SS:mmm
    // Indices (with offset):
    // HH: 0-1 (2 chars)
    // :   2
    // MM: 3-4 (2 chars)
    // :   5
    // SS: 6-7 (2 chars)
    // :   8
    // mmm: 9-11 (3 chars)

    if (effectiveIndex <= 1)
        editor->setHighlightedRegion(juce::Range<int>(0 + offset, 2 + offset)); // HH
    else if (effectiveIndex >= 3 && effectiveIndex <= 4)
        editor->setHighlightedRegion(juce::Range<int>(3 + offset, 5 + offset)); // MM
    else if (effectiveIndex >= 6 && effectiveIndex <= 7)
        editor->setHighlightedRegion(juce::Range<int>(6 + offset, 8 + offset)); // SS
    else if (effectiveIndex >= 9 && effectiveIndex <= 11)
        editor->setHighlightedRegion(juce::Range<int>(9 + offset, 12 + offset)); // mmm
}

void PlaybackTextPresenter::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (wheel.deltaY == 0.0f) return;

    auto* editor = dynamic_cast<juce::TextEditor*>(event.eventComponent);
    if (editor == nullptr) return;

    double currentVal = TimeUtils::parseTime(editor->getText());
    if (currentVal < 0.0) currentVal = 0.0;

    // Determine character index, accounting for possible negative sign offset
    bool isNegative = (editor == &owner.remainingTimeEditor) || editor->getText().startsWith("-");
    int offset = isNegative ? 1 : 0;
    int charIndex = editor->getTextIndexAt(event.getPosition());
    int effectiveIndex = charIndex - offset;

    double sampleRate = 0.0;
    if (auto* reader = owner.getAudioPlayer().getAudioFormatReader())
    {
        sampleRate = reader->sampleRate;
    }

    double step = TimeUtils::getStepSize(effectiveIndex,
                                         event.mods.isShiftDown(),
                                         event.mods.isCtrlDown(),
                                         event.mods.isAltDown(),
                                         sampleRate);

    double direction = (wheel.deltaY > 0) ? 1.0 : -1.0;
    double newVal = juce::jmax(0.0, currentVal + (direction * step));

    if (editor == &owner.elapsedTimeEditor)
    {
        owner.getAudioPlayer().getTransportSource().setPosition(newVal);
    }
    else if (editor == &owner.remainingTimeEditor)
    {
        double total = owner.getAudioPlayer().getThumbnail().getTotalLength();
        owner.getAudioPlayer().getTransportSource().setPosition(total - newVal);
    }
    else if (editor == &owner.loopLengthEditor)
    {
        owner.setLoopOutPosition(owner.getLoopInPosition() + newVal);
        owner.ensureLoopOrder();
        owner.updateLoopLabels();
    }

    updateEditors();
}
