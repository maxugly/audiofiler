#include "LoopPresenter.h"
#include "FocusManager.h"
#include "TimeUtils.h"

#include "AudioPlayer.h"
#include "Config.h"
#include "ControlPanel.h"
#include "SilenceDetector.h"
#include "TimeEntryHelpers.h"
#include "TimeUtils.h"
#include <utility>

LoopPresenter::LoopPresenter(ControlPanel &ownerPanel,
                             SilenceDetector &detector,
                             juce::TextEditor &cutIn,
                             juce::TextEditor &cutOut)
    : owner(ownerPanel), silenceDetector(detector), cutInEditor(cutIn),
      cutOutEditor(cutOut) {
  cutInEditor.addListener(this);
  cutOutEditor.addListener(this);
  cutInEditor.addMouseListener(this, false);
  cutOutEditor.addMouseListener(this, false);
}

void LoopPresenter::initialiseEditors() {
  auto configure = [](juce::TextEditor &editor) {
    editor.setReadOnly(false);
    editor.setJustification(juce::Justification::centred);
    editor.setColour(juce::TextEditor::backgroundColourId,
                     Config::Colors::textEditorBackground);
    editor.setColour(juce::TextEditor::textColourId,
                     Config::Colors::playbackText);
    editor.setFont(
        juce::Font(juce::FontOptions(Config::Layout::Text::playbackSize)));
    editor.setMultiLine(false);
    editor.setReturnKeyStartsNewLine(false);
    editor.setWantsKeyboardFocus(true);
    editor.setSelectAllWhenFocused(true);
  };

  owner.addAndMakeVisible(cutInEditor);
  configure(cutInEditor);

  owner.addAndMakeVisible(cutOutEditor);
  configure(cutOutEditor);
}

LoopPresenter::~LoopPresenter() {
  cutInEditor.removeListener(this);
  cutOutEditor.removeListener(this);
  cutInEditor.removeMouseListener(this);
  cutOutEditor.removeMouseListener(this);
}

void LoopPresenter::setCutInPosition(double positionSeconds) {
  const double totalLength = getAudioTotalLength();
  const double newPos = juce::jlimit(0.0, totalLength, positionSeconds);

  // Crossing Logic: If we manually move In past an Auto-Out, turn off Auto-Out
  if (!silenceDetector.getIsAutoCutInActive() && newPos >= cutOutPosition &&
      silenceDetector.getIsAutoCutOutActive()) {
    silenceDetector.setIsAutoCutOutActive(false);
    owner.updateComponentStates();
  }

  cutInPosition = newPos;

  // Push Logic: If In crosses Out and AC In is active
  if (silenceDetector.getIsAutoCutInActive() &&
      cutInPosition >= cutOutPosition) {
    // Push Out to the end, then re-detect if AC Out is active
    setCutOutPosition(totalLength);
    if (silenceDetector.getIsAutoCutOutActive())
      silenceDetector.detectOutSilence();
  }

  // Constrain playback head if it's outside new cutIn
  auto &audioPlayer = owner.getAudioPlayer();
  audioPlayer.setPositionConstrained(
      audioPlayer.getTransportSource().getCurrentPosition(), cutInPosition,
      cutOutPosition);
  ensureCutOrder();
}

void LoopPresenter::setCutOutPosition(double positionSeconds) {
  const double totalLength = getAudioTotalLength();
  const double newPos = juce::jlimit(0.0, totalLength, positionSeconds);

  // Crossing Logic: If we manually move Out past an Auto-In, turn off Auto-In
  if (!silenceDetector.getIsAutoCutOutActive() && newPos <= cutInPosition &&
      silenceDetector.getIsAutoCutInActive()) {
    silenceDetector.setIsAutoCutInActive(false);
    owner.updateComponentStates();
  }

  cutOutPosition = newPos;

  // Pull Logic: If Out crosses In and AC Out is active
  if (silenceDetector.getIsAutoCutOutActive() &&
      cutOutPosition <= cutInPosition) {
    // Pull In to the start, then re-detect if AC In is active
    setCutInPosition(0.0);
    if (silenceDetector.getIsAutoCutInActive())
      silenceDetector.detectInSilence();
  }

  // Constrain playback head if it's outside new cutOut
  auto &audioPlayer = owner.getAudioPlayer();
  audioPlayer.setPositionConstrained(
      audioPlayer.getTransportSource().getCurrentPosition(), cutInPosition,
      cutOutPosition);
  ensureCutOrder();
}

void LoopPresenter::ensureCutOrder() {
  if (cutInPosition > cutOutPosition) {
    std::swap(cutInPosition, cutOutPosition);

    // Swap Auto-Cut states as well so the "Auto" property follows the detected
    // value
    bool acIn = silenceDetector.getIsAutoCutInActive();
    bool acOut = silenceDetector.getIsAutoCutOutActive();
    silenceDetector.setIsAutoCutInActive(acOut);
    silenceDetector.setIsAutoCutOutActive(acIn);

    // Ensure UI buttons reflect the swapped auto states
    owner.updateComponentStates();
  }
}

void LoopPresenter::updateCutLabels() {
  // Guard against timer overwriting while editing OR focused
  if (!isEditingCutIn && !cutInEditor.hasKeyboardFocus(true)) {
    syncEditorToPosition(cutInEditor, cutInPosition);
  }

  if (!isEditingCutOut && !cutOutEditor.hasKeyboardFocus(true)) {
    syncEditorToPosition(cutOutEditor, cutOutPosition);
  }
}

void LoopPresenter::setCutStartFromSample(int sampleIndex) {
  AudioPlayer &audioPlayer = owner.getAudioPlayer();
  if (audioPlayer.getAudioFormatReader() != nullptr) {
    const double sampleRate = audioPlayer.getAudioFormatReader()->sampleRate;
    setCutInPosition((double)sampleIndex / sampleRate);
    ensureCutOrder();
    updateCutLabels();
    owner.repaint();
  }
}

void LoopPresenter::setCutEndFromSample(int sampleIndex) {
  AudioPlayer &audioPlayer = owner.getAudioPlayer();
  if (audioPlayer.getAudioFormatReader() != nullptr) {
    const double sampleRate = audioPlayer.getAudioFormatReader()->sampleRate;
    setCutOutPosition((double)sampleIndex / sampleRate);
    ensureCutOrder();
    updateCutLabels();
    owner.repaint();
  }
}

void LoopPresenter::textEditorTextChanged(juce::TextEditor &editor) {
  if (&editor == &cutInEditor)
    isEditingCutIn = true;
  else if (&editor == &cutOutEditor)
    isEditingCutOut = true;

  const double totalLength = getAudioTotalLength();
  TimeEntryHelpers::validateTimeEntry(editor, totalLength);
}

void LoopPresenter::textEditorReturnKeyPressed(juce::TextEditor &editor) {
  if (&editor == &cutInEditor)
    isEditingCutIn = false;
  if (&editor == &cutOutEditor)
    isEditingCutOut = false;

  const double newPosition = TimeUtils::parseTime(editor.getText());
  if (&editor == &cutInEditor) {
    applyCutInFromEditor(newPosition, editor);
  } else if (&editor == &cutOutEditor) {
    applyCutOutFromEditor(newPosition, editor);
  }
  editor.giveAwayKeyboardFocus();
}

void LoopPresenter::textEditorEscapeKeyPressed(juce::TextEditor &editor) {
  if (&editor == &cutInEditor)
    isEditingCutIn = false;
  if (&editor == &cutOutEditor)
    isEditingCutOut = false;

  if (&editor == &cutInEditor) {
    syncEditorToPosition(editor, cutInPosition);
  } else if (&editor == &cutOutEditor) {
    syncEditorToPosition(editor, cutOutPosition);
  }
  editor.setColour(juce::TextEditor::textColourId,
                   Config::Colors::playbackText);
  editor.giveAwayKeyboardFocus();
}

void LoopPresenter::textEditorFocusLost(juce::TextEditor &editor) {
  if (&editor == &cutInEditor)
    isEditingCutIn = false;
  if (&editor == &cutOutEditor)
    isEditingCutOut = false;

  const double newPosition = TimeUtils::parseTime(editor.getText());
  if (&editor == &cutInEditor) {
    applyCutInFromEditor(newPosition, editor);
  } else if (&editor == &cutOutEditor) {
    applyCutOutFromEditor(newPosition, editor);
  }

  // Clear zoom on focus lost
  owner.setActiveZoomPoint(ControlPanel::ActiveZoomPoint::None);
  owner.performDelayedJumpIfNeeded();
}

void LoopPresenter::mouseDown(const juce::MouseEvent &event) {
  if (event.eventComponent == &cutInEditor)
    isEditingCutIn = true;
  else if (event.eventComponent == &cutOutEditor)
    isEditingCutOut = true;
}


double LoopPresenter::getAudioTotalLength() const {
  return owner.getAudioPlayer().getThumbnail().getTotalLength();
}

bool LoopPresenter::applyCutInFromEditor(double newPosition,
                                          juce::TextEditor &editor) {
  const double totalLength = getAudioTotalLength();
  if (newPosition >= 0.0 && newPosition <= totalLength) {

    setCutInPosition(newPosition);
    owner.updateCutButtonColors();
    silenceDetector.setIsAutoCutInActive(false);
    owner.updateComponentStates();

    if (owner.getActiveZoomPoint() != ControlPanel::ActiveZoomPoint::None)
      owner.setNeedsJumpToLoopIn(true);

    editor.setColour(juce::TextEditor::textColourId,
                     Config::Colors::playbackText);
    owner.repaint();
    updateCutLabels();
    return true;
  }

  syncEditorToPosition(editor, cutInPosition);
  editor.setColour(juce::TextEditor::textColourId,
                   Config::Colors::textEditorError);
  owner.repaint();
  return false;
}

bool LoopPresenter::applyCutOutFromEditor(double newPosition,
                                           juce::TextEditor &editor) {
  const double totalLength = getAudioTotalLength();
  if (newPosition >= 0.0 && newPosition <= totalLength) {
    AudioPlayer &audioPlayer = owner.getAudioPlayer();
    auto &transport = audioPlayer.getTransportSource();
    if (owner.getShouldLoop() &&
        transport.getCurrentPosition() >= cutOutPosition)
      transport.setPosition(cutInPosition);

    setCutOutPosition(newPosition);
    owner.updateCutButtonColors();
    silenceDetector.setIsAutoCutOutActive(false);
    owner.updateComponentStates();

    if (owner.getActiveZoomPoint() != ControlPanel::ActiveZoomPoint::None)
      owner.setNeedsJumpToLoopIn(true);

    editor.setColour(juce::TextEditor::textColourId,
                     Config::Colors::playbackText);
    owner.repaint();
    updateCutLabels();
    return true;
  }

  syncEditorToPosition(editor, cutOutPosition);
  editor.setColour(juce::TextEditor::textColourId,
                   Config::Colors::textEditorError);
  owner.repaint();
  return false;
}

void LoopPresenter::syncEditorToPosition(juce::TextEditor &editor,
                                         double positionSeconds) {
  // Multi-layered guard: Check flags AND OS-level focus
  if (editor.hasKeyboardFocus(true) ||
      (&editor == &cutInEditor && isEditingCutIn) ||
      (&editor == &cutOutEditor && isEditingCutOut)) {
    return;
  }

  juce::String newText = owner.formatTime(positionSeconds);
  if (editor.getText() != newText)
    editor.setText(newText, juce::dontSendNotification);
}

void LoopPresenter::mouseEnter(const juce::MouseEvent &event) {
  // Don't switch if 'z' key is already holding a zoom point

  if (event.eventComponent == &cutInEditor)
    owner.setActiveZoomPoint(ControlPanel::ActiveZoomPoint::In);
  else if (event.eventComponent == &cutOutEditor)
    owner.setActiveZoomPoint(ControlPanel::ActiveZoomPoint::Out);
}

void LoopPresenter::mouseExit(const juce::MouseEvent &event) {
  // Only clear if the editor doesn't have focus
  auto *editor = dynamic_cast<juce::TextEditor *>(event.eventComponent);
  if (editor != nullptr && !editor->hasKeyboardFocus(false)) {
    owner.setActiveZoomPoint(ControlPanel::ActiveZoomPoint::None);
    owner.performDelayedJumpIfNeeded();
  }
}

void LoopPresenter::mouseUp(const juce::MouseEvent &event) {
  auto *editor = dynamic_cast<juce::TextEditor *>(event.eventComponent);
  if (editor == nullptr)
    return;

  // Set flags to block timer. 
  // CRITICAL: Do NOT call grabKeyboardFocus here; let juce::TextEditor's internal logic 
  // handle the click/focus sequence to avoid selection reset bugs.
  if (editor == &cutInEditor)
    isEditingCutIn = true;
  else if (editor == &cutOutEditor)
    isEditingCutOut = true;

  int charIndex = editor->getTextIndexAt(event.getPosition());
  if (charIndex < 0)
    return;

  juce::Range<int> newRange;

  if (charIndex <= 1)
    newRange = juce::Range<int>(0, 2); // HH
  else if (charIndex >= 3 && charIndex <= 4)
    newRange = juce::Range<int>(3, 5); // MM
  else if (charIndex >= 6 && charIndex <= 7)
    newRange = juce::Range<int>(6, 8); // SS
  else if (charIndex >= 9 && charIndex <= 11)
    newRange = juce::Range<int>(9, 12); // mmm
  else
    return;

  // Selection AFTER the internal mouseUp sequence
  juce::MessageManager::callAsync([editor, newRange] {
    if (editor != nullptr)
      editor->setHighlightedRegion(newRange);
  });
}

void LoopPresenter::mouseWheelMove(const juce::MouseEvent &event,
                                   const juce::MouseWheelDetails &wheel) {
  if (wheel.deltaY == 0.0f)
    return;

  auto *editor = dynamic_cast<juce::TextEditor *>(event.eventComponent);
  if (editor != nullptr) {
    // NEW GUARD: If typing or focused, ignore the wheel
    if (editor->hasKeyboardFocus(true) ||
        (editor == &cutInEditor && isEditingCutIn) ||
        (editor == &cutOutEditor && isEditingCutOut))
      return;
  }

  // CTRL + Mouse Wheel (without Shift) controls zoom
  if (event.mods.isCtrlDown() && !event.mods.isShiftDown()) {
    float currentZoom = owner.getZoomFactor();
    float zoomDelta = wheel.deltaY > 0 ? 1.1f : 0.9f;
    owner.setZoomFactor(currentZoom * zoomDelta);
    return;
  }

  if (editor == nullptr)
    return;

  const double totalLength = getAudioTotalLength();
  if (totalLength <= 0.0)
    return;
  // Determine character index under the mouse to set step size contextually
  // Format is HH:MM:SS:mmm (012345678901)
  int charIndex = editor->getTextIndexAt(event.getPosition());

  double sampleRate = 0.0;
  if (auto *reader = owner.getAudioPlayer().getAudioFormatReader())
    sampleRate = reader->sampleRate;

  double step =
      TimeEntryHelpers::calculateStepSize(charIndex, event.mods, sampleRate);

  const double direction = (wheel.deltaY > 0) ? 1.0 : -1.0;
  const double delta = direction * step;

  if (editor == &cutInEditor) {
    double newPos = juce::jlimit(0.0, totalLength, cutInPosition + delta);
    if (newPos != cutInPosition) {
      setCutInPosition(newPos);
      silenceDetector.setIsAutoCutInActive(false);
      owner.updateComponentStates();
      owner.setNeedsJumpToLoopIn(true);
      ensureCutOrder();
      updateCutLabels();
      owner.repaint();
    }
  } else if (editor == &cutOutEditor) {
    double newPos = juce::jlimit(0.0, totalLength, cutOutPosition + delta);
    if (newPos != cutOutPosition) {
      setCutOutPosition(newPos);
      silenceDetector.setIsAutoCutOutActive(false);
      owner.updateComponentStates();
      owner.setNeedsJumpToLoopIn(true);
      ensureCutOrder();
      updateCutLabels();
      owner.repaint();
    }
  }
}
