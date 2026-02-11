#include "PlaybackTextPresenter.h"

#include "ControlPanel.h"
#include "Config.h"
#include "AudioPlayer.h"

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
    auto configure = [&](juce::TextEditor& ed, juce::Justification just) {
        owner.addAndMakeVisible(ed);
        ed.setReadOnly(false);
        ed.setJustification(just);
        ed.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
        ed.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
        ed.setColour(juce::TextEditor::textColourId, Config::playbackTextColor);
        ed.setFont(juce::Font(juce::FontOptions((float)Config::playbackTextSize)));
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
    const int textY = owner.getBottomRowTopY() - Config::playbackTimeTextOffsetY;
    auto [leftX, centreX, rightX] = owner.getPlaybackLabelXs();

    owner.elapsedTimeEditor.setBounds(leftX, textY, Config::playbackTextWidth, Config::playbackTextHeight);
    owner.remainingTimeEditor.setBounds(rightX, textY, Config::playbackTextWidth, Config::playbackTextHeight);
    
    // The center is special because it's shared with total time which is static
    owner.loopLengthEditor.setBounds(centreX, textY, Config::playbackTextWidth / 2, Config::playbackTextHeight);
}

void PlaybackTextPresenter::render(juce::Graphics& g) const
{
    if (owner.getAudioPlayer().getThumbnail().getTotalLength() <= 0.0)
        return;

    // Draw the static total time part behind or next to the loop length
    const int textY = owner.getBottomRowTopY() - Config::playbackTimeTextOffsetY;
    auto [leftX, centreX, rightX] = owner.getPlaybackLabelXs();

    g.setColour(Config::playbackTextColor);
    g.setFont((float)Config::playbackTextSize);
    
    juce::String totalTimeStr = " / " + getTotalTimeStaticString();
    
    // Position it relative to the loop length editor
    int loopLenX = owner.loopLengthEditor.getX();
    int loopLenW = owner.loopLengthEditor.getWidth();
    
    // We want to draw it centered-ish with the loop length
    // But since loopLength is its own editor, let's just draw totalTime to the right of centreX + playbackTextWidth/2
    g.drawText(totalTimeStr, centreX + (Config::playbackTextWidth / 2), textY, Config::playbackTextWidth / 2, Config::playbackTextHeight, juce::Justification::left, false);
}

void PlaybackTextPresenter::textEditorTextChanged(juce::TextEditor& editor)
{
    // Validation logic similar to LoopPresenter
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
    double newTime = parseTime(editor.getText());
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
        owner.setLoopOutPosition(owner.getLoopInPosition() + newTime);
        owner.ensureLoopOrder();
        owner.updateLoopLabels();
    }
    
    updateEditors();
}

double PlaybackTextPresenter::parseTime(const juce::String& timeString) const
{
    // Remove leading '-' if present for remaining time
    juce::String cleanTime = timeString.startsWithChar('-') ? timeString.substring(1) : timeString;
    
    auto parts = juce::StringArray::fromTokens(cleanTime, ":", "");
    if (parts.size() != 4)
        return -1.0;

    return parts[0].getIntValue() * 3600.0
         + parts[1].getIntValue() * 60.0
         + parts[2].getIntValue()
         + parts[3].getIntValue() / 1000.0;
}

void PlaybackTextPresenter::syncEditorToPosition(juce::TextEditor& editor, double positionSeconds, bool isRemaining)
{
    juce::String text = owner.formatTime(positionSeconds);
    if (isRemaining) text = "-" + text;
    editor.setText(text, juce::dontSendNotification);
}

void PlaybackTextPresenter::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (wheel.deltaY == 0.0f) return;

    auto* editor = dynamic_cast<juce::TextEditor*>(event.eventComponent);
    if (editor == nullptr) return;

    double currentVal = parseTime(editor->getText());
    if (currentVal < 0.0) currentVal = 0.0;

    // Determine step size (matching LoopPresenter logic)
    int charIndex = editor->getTextIndexAt(event.getPosition());
    double step = Config::loopStepMilliseconds;
    
    if (charIndex >= 0 && charIndex <= 1)      step = Config::loopStepHours;
    else if (charIndex >= 3 && charIndex <= 4) step = Config::loopStepMinutes;
    else if (charIndex >= 6 && charIndex <= 7) step = Config::loopStepSeconds;
    else if (charIndex >= 9) 
    {
        if (event.mods.isCtrlDown() && event.mods.isShiftDown())
        {
            auto& audioPlayer = owner.getAudioPlayer();
            if (auto* reader = audioPlayer.getAudioFormatReader())
                step = 1.0 / reader->sampleRate;
            else
                step = 0.0001;
        }
        else if (event.mods.isShiftDown()) step = Config::loopStepMillisecondsFine;
        else step = Config::loopStepMilliseconds;
    }

    if (event.mods.isAltDown()) step *= 10.0;

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
