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
    loopInEditor.addMouseListener(this, false);
    loopOutEditor.addMouseListener(this, false);
}

LoopPresenter::~LoopPresenter()
{
    loopInEditor.removeListener(this);
    loopOutEditor.removeListener(this);
    loopInEditor.removeMouseListener(this);
    loopOutEditor.removeMouseListener(this);
}

void LoopPresenter::setLoopInPosition(double positionSeconds)
{
    const double totalLength = getAudioTotalLength();
    const double newPos = juce::jlimit(0.0, totalLength, positionSeconds);

    // Crossing Logic: If we manually move In past an Auto-Out, turn off Auto-Out
    if (!silenceDetector.getIsAutoCutInActive() && newPos >= loopOutPosition && silenceDetector.getIsAutoCutOutActive())
    {
        silenceDetector.setIsAutoCutOutActive(false);
        owner.updateComponentStates();
    }
    
    loopInPosition = newPos;
    
    // Push Logic: If In crosses Out and AC In is active
    if (silenceDetector.getIsAutoCutInActive() && loopInPosition >= loopOutPosition)
    {
        // Push Out to the end, then re-detect if AC Out is active
        setLoopOutPosition(totalLength);
        if (silenceDetector.getIsAutoCutOutActive())
            silenceDetector.detectOutSilence();
    }

    // Constrain playback head if it's outside new loopIn
    auto& audioPlayer = owner.getAudioPlayer();
    audioPlayer.setPositionConstrained(audioPlayer.getTransportSource().getCurrentPosition(),
                                       loopInPosition, loopOutPosition);
    ensureLoopOrder();
}

void LoopPresenter::setLoopOutPosition(double positionSeconds)
{
    const double totalLength = getAudioTotalLength();
    const double newPos = juce::jlimit(0.0, totalLength, positionSeconds);

    // Crossing Logic: If we manually move Out past an Auto-In, turn off Auto-In
    if (!silenceDetector.getIsAutoCutOutActive() && newPos <= loopInPosition && silenceDetector.getIsAutoCutInActive())
    {
        silenceDetector.setIsAutoCutInActive(false);
        owner.updateComponentStates();
    }

    loopOutPosition = newPos;

    // Pull Logic: If Out crosses In and AC Out is active
    if (silenceDetector.getIsAutoCutOutActive() && loopOutPosition <= loopInPosition)
    {
        // Pull In to the start, then re-detect if AC In is active
        setLoopInPosition(0.0);
        if (silenceDetector.getIsAutoCutInActive())
            silenceDetector.detectInSilence();
    }

    // Constrain playback head if it's outside new loopOut
    auto& audioPlayer = owner.getAudioPlayer();
    audioPlayer.setPositionConstrained(audioPlayer.getTransportSource().getCurrentPosition(),
                                       loopInPosition, loopOutPosition);
    ensureLoopOrder();
}

void LoopPresenter::ensureLoopOrder()
{
    if (loopInPosition > loopOutPosition)
    {
        std::swap(loopInPosition, loopOutPosition);
        
        // Swap Auto-Cut states as well so the "Auto" property follows the detected value
        bool acIn = silenceDetector.getIsAutoCutInActive();
        bool acOut = silenceDetector.getIsAutoCutOutActive();
        silenceDetector.setIsAutoCutInActive(acOut);
        silenceDetector.setIsAutoCutOutActive(acIn);
        
        // Ensure UI buttons reflect the swapped auto states
        owner.updateComponentStates();
    }
}

void LoopPresenter::updateLoopLabels()
{
    if (!loopInEditor.hasKeyboardFocus(false))
        syncEditorToPosition(loopInEditor, loopInPosition);
    if (!loopOutEditor.hasKeyboardFocus(false))
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
    editor.setColour(juce::TextEditor::textColourId, Config::Colors::playbackText);
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
    if (owner.isZKeyDown())
        return;

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

    // Determine character index under the mouse to set step size contextually
    // Format is HH:MM:SS:mmm (012345678901)
    int charIndex = editor->getTextIndexAt(event.getPosition());
    double step = Config::Audio::loopStepMilliseconds;
    
    if (charIndex >= 0 && charIndex <= 1)      // HH
        step = Config::Audio::loopStepHours;
    else if (charIndex >= 3 && charIndex <= 4) // MM
        step = Config::Audio::loopStepMinutes;
    else if (charIndex >= 6 && charIndex <= 7) // SS
        step = Config::Audio::loopStepSeconds;
    else if (charIndex >= 9)                   // mmm
    {
        if (event.mods.isCtrlDown() && event.mods.isShiftDown())
        {
            // Sample accurate (audio frame)
            auto& audioPlayer = owner.getAudioPlayer();
            if (auto* reader = audioPlayer.getAudioFormatReader())
                step = 1.0 / reader->sampleRate;
            else
                step = 0.0001; // Fallback
        }
        else if (event.mods.isShiftDown())
        {
            step = Config::Audio::loopStepMillisecondsFine; // 1ms
        }
        else
        {
            step = Config::Audio::loopStepMilliseconds; // 10ms
        }
    }

    // Alt is a x10 multiplier
    if (event.mods.isAltDown())
        step *= 10.0;

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
