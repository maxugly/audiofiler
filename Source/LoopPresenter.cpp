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
    loopInPosition = positionSeconds;
    
    // Constrain playback head if it's outside new loopIn
    auto& audioPlayer = owner.getAudioPlayer();
    audioPlayer.setPositionConstrained(audioPlayer.getTransportSource().getCurrentPosition(),
                                       loopInPosition, loopOutPosition);
}

void LoopPresenter::setLoopOutPosition(double positionSeconds)
{
    loopOutPosition = positionSeconds;

    // Constrain playback head if it's outside new loopOut
    auto& audioPlayer = owner.getAudioPlayer();
    audioPlayer.setPositionConstrained(audioPlayer.getTransportSource().getCurrentPosition(),
                                       loopInPosition, loopOutPosition);
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
    
    // Clear zoom on focus lost
    owner.setActiveZoomPoint(ControlPanel::ActiveZoomPoint::None);
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
        if (loopOutPosition > -1.0 && newPosition > loopOutPosition)
        {
            syncEditorToPosition(editor, loopInPosition);
            editor.setColour(juce::TextEditor::textColourId, Config::textEditorWarningColor);
            return false;
        }

        setLoopInPosition(newPosition);
        owner.updateLoopButtonColors();
        silenceDetector.setIsAutoCutInActive(false);
        owner.updateComponentStates();
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
        silenceDetector.setIsAutoCutOutActive(false);
        owner.updateComponentStates();
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
    }
}

void LoopPresenter::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (wheel.deltaY == 0.0f)
        return;

    // If zoomed in, mouse wheel controls zoom level
    if (owner.getActiveZoomPoint() != ControlPanel::ActiveZoomPoint::None)
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
    double step = Config::loopStepMilliseconds;
    
    if (charIndex >= 0 && charIndex <= 1)      // HH
        step = Config::loopStepHours;
    else if (charIndex >= 3 && charIndex <= 4) // MM
        step = Config::loopStepMinutes;
    else if (charIndex >= 6 && charIndex <= 7) // SS
        step = Config::loopStepSeconds;
    else if (charIndex >= 9)                   // mmm
    {
        step = event.mods.isShiftDown() ? Config::loopStepMillisecondsFine : Config::loopStepMilliseconds;
    }

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
            ensureLoopOrder();
            updateLoopLabels();
            owner.repaint();
        }
    }
}
