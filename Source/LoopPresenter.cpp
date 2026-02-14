#include "LoopPresenter.h"
#include "ControlPanel.h"
#include "SilenceDetector.h"
#include "FocusManager.h"
#include "Config.h"
#include "AudioPlayer.h"
#include "TimeUtils.h"

LoopPresenter::LoopPresenter(ControlPanel& ownerPanel,
                             SilenceDetector& detector,
                             juce::TextEditor& loopIn,
                             juce::TextEditor& loopOut)
    : owner(ownerPanel),
      silenceDetector(detector),
      loopInEditor(loopIn),
      loopOutEditor(loopOut)
{
    auto configure = [this](juce::TextEditor& ed) {
        ed.setJustification(juce::Justification::centred);
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

    configure(loopInEditor);
    configure(loopOutEditor);
}

LoopPresenter::~LoopPresenter()
{
    loopInEditor.removeListener(this);
    loopOutEditor.removeListener(this);
}

void LoopPresenter::setLoopInPosition(double positionSeconds)
{
    loopInPosition = positionSeconds;
    // Don't update labels immediately to allow smooth typing/updates
    // Labels are updated via updateLoopLabels() called explicitly or by repaint
}

void LoopPresenter::setLoopOutPosition(double positionSeconds)
{
    loopOutPosition = positionSeconds;
}

void LoopPresenter::ensureLoopOrder()
{
    if (loopInPosition > loopOutPosition)
    {
        std::swap(loopInPosition, loopOutPosition);
    }
}

void LoopPresenter::updateLoopLabels()
{
    if (!loopInEditor.hasKeyboardFocus(false))
    {
        syncEditorToPosition(loopInEditor, loopInPosition);
        loopInEditor.setColour(juce::TextEditor::textColourId, Config::Colors::playbackText);
    }

    if (!loopOutEditor.hasKeyboardFocus(false))
    {
        syncEditorToPosition(loopOutEditor, loopOutPosition);
        loopOutEditor.setColour(juce::TextEditor::textColourId, Config::Colors::playbackText);
    }
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

void LoopPresenter::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    const double newPosition = TimeUtils::parseTime(editor.getText());
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
    editor.setColour(juce::TextEditor::textColourId, Config::Colors::playbackText);
    editor.giveAwayKeyboardFocus();
}

void LoopPresenter::textEditorFocusLost(juce::TextEditor& editor)
{
    const double newPosition = TimeUtils::parseTime(editor.getText());
    if (&editor == &loopInEditor)
    {
        applyLoopInFromEditor(newPosition, editor);
    }
    else if (&editor == &loopOutEditor)
    {
        applyLoopOutFromEditor(newPosition, editor);
    }
    
    // Clear zoom on focus lost
    owner.setActiveZoomPoint(ControlPanel::ActiveZoomPoint::None);
    owner.performDelayedJumpIfNeeded();
}

void LoopPresenter::textEditorFocusGained(juce::TextEditor& editor)
{
    if (&editor == &loopInEditor)
        owner.setActiveZoomPoint(ControlPanel::ActiveZoomPoint::In);
    else if (&editor == &loopOutEditor)
        owner.setActiveZoomPoint(ControlPanel::ActiveZoomPoint::Out);
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


        setLoopInPosition(newPosition);
        owner.updateLoopButtonColors();
        silenceDetector.setIsAutoCutInActive(false);
        owner.updateComponentStates();
        
        if (owner.getActiveZoomPoint() != ControlPanel::ActiveZoomPoint::None)
            owner.setNeedsJumpToLoopIn(true);

        editor.setColour(juce::TextEditor::textColourId, Config::Colors::playbackText);
        owner.repaint();
        updateLoopLabels();
        return true;
    }

    syncEditorToPosition(editor, loopInPosition);
    editor.setColour(juce::TextEditor::textColourId, Config::Colors::textEditorError);
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



        setLoopOutPosition(newPosition);
        owner.updateLoopButtonColors();
        silenceDetector.setIsAutoCutOutActive(false);
        owner.updateComponentStates();

        if (owner.getActiveZoomPoint() != ControlPanel::ActiveZoomPoint::None)
            owner.setNeedsJumpToLoopIn(true);

        editor.setColour(juce::TextEditor::textColourId, Config::Colors::playbackText);
        owner.repaint();
        updateLoopLabels();
        return true;
    }

    syncEditorToPosition(editor, loopOutPosition);
    editor.setColour(juce::TextEditor::textColourId, Config::Colors::textEditorError);
    owner.repaint();
    return false;
}

void LoopPresenter::syncEditorToPosition(juce::TextEditor& editor, double positionSeconds)
{
    editor.setText(owner.formatTime(positionSeconds), juce::dontSendNotification);
}

void LoopPresenter::mouseEnter(const juce::MouseEvent& event)
{
    // Don't switch if 'z' key is already holding a zoom point

    if (event.eventComponent == &loopInEditor)
        owner.setActiveZoomPoint(ControlPanel::ActiveZoomPoint::In);
    else if (event.eventComponent == &loopOutEditor)
        owner.setActiveZoomPoint(ControlPanel::ActiveZoomPoint::Out);
}

void LoopPresenter::mouseExit(const juce::MouseEvent& event)
{
    // Only clear if the editor doesn't have focus
    auto* editor = dynamic_cast<juce::TextEditor*>(event.eventComponent);
    if (editor != nullptr && !editor->hasKeyboardFocus(false))
    {
        owner.setActiveZoomPoint(ControlPanel::ActiveZoomPoint::None);
        owner.performDelayedJumpIfNeeded();
    }
}

void LoopPresenter::mouseUp(const juce::MouseEvent& event)
{
    auto* editor = dynamic_cast<juce::TextEditor*>(event.eventComponent);
    if (editor == nullptr)
        return;

    // Only apply smart highlight if the user hasn't made a manual selection
    if (editor->getHighlightedRegion().getLength() > 0)
        return;

    int charIndex = editor->getTextIndexAt(event.getPosition());
    if (charIndex < 0) return;

    // Time format: HH:MM:SS:mmm
    // Indices:
    // HH: 0-1 (2 chars)
    // :   2
    // MM: 3-4 (2 chars)
    // :   5
    // SS: 6-7 (2 chars)
    // :   8
    // mmm: 9-11 (3 chars)

    if (charIndex <= 1)
        editor->setHighlightedRegion(juce::Range<int>(0, 2)); // HH
    else if (charIndex >= 3 && charIndex <= 4)
        editor->setHighlightedRegion(juce::Range<int>(3, 5)); // MM
    else if (charIndex >= 6 && charIndex <= 7)
        editor->setHighlightedRegion(juce::Range<int>(6, 8)); // SS
    else if (charIndex >= 9 && charIndex <= 11)
        editor->setHighlightedRegion(juce::Range<int>(9, 12)); // mmm
}

void LoopPresenter::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (wheel.deltaY == 0.0f)
        return;

    // CTRL + Mouse Wheel (without Shift) controls zoom
    if (event.mods.isCtrlDown() && !event.mods.isShiftDown())
    {
        float currentZoom = owner.getZoomFactor();
        float zoomDelta = wheel.deltaY > 0 ? 1.1f : 0.9f;
        owner.setZoomFactor(currentZoom * zoomDelta);
        return;
    }

    auto* editor = dynamic_cast<juce::TextEditor*>(event.eventComponent);
    if (editor == nullptr)
        return;

    const double totalLength = getAudioTotalLength();
    if (totalLength <= 0.0)
        return;

    int charIndex = editor->getTextIndexAt(event.getPosition());
    
    // Get sample rate
    double sampleRate = 0.0;
    if (auto* reader = owner.getAudioPlayer().getAudioFormatReader())
    {
        sampleRate = reader->sampleRate;
    }

    double step = TimeUtils::getStepSize(charIndex,
                                         event.mods.isShiftDown(),
                                         event.mods.isCtrlDown(),
                                         event.mods.isAltDown(),
                                         sampleRate);

    const double direction = (wheel.deltaY > 0) ? 1.0 : -1.0;
    const double delta = direction * step;

    if (editor == &loopInEditor)
    {
        double newPos = juce::jlimit(0.0, totalLength, loopInPosition + delta);
        if (newPos != loopInPosition)
        {
            setLoopInPosition(newPos);
            silenceDetector.setIsAutoCutInActive(false);
            owner.updateComponentStates();
            owner.setNeedsJumpToLoopIn(true);
            ensureLoopOrder();
            updateLoopLabels();
            owner.repaint();
        }
    }
    else if (editor == &loopOutEditor)
    {
        double newPos = juce::jlimit(0.0, totalLength, loopOutPosition + delta);
        if (newPos != loopOutPosition)
        {
            setLoopOutPosition(newPos);
            silenceDetector.setIsAutoCutOutActive(false);
            owner.updateComponentStates();
            owner.setNeedsJumpToLoopIn(true);
            ensureLoopOrder();
            updateLoopLabels();
            owner.repaint();
        }
    }
}
