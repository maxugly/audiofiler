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
                             juce::TextEditor &loopIn,
                             juce::TextEditor &loopOut)
    : owner(ownerPanel), silenceDetector(detector), cutInEditor(loopIn),
      cutOutEditor(loopOut) {
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

double LoopPresenter::getCutInPosition() const noexcept {
  return owner.getAudioPlayer().getCutIn();
}

double LoopPresenter::getCutOutPosition() const noexcept {
  return owner.getAudioPlayer().getCutOut();
}

void LoopPresenter::setCutInPosition(double positionSeconds) {
  const double totalLength = getAudioTotalLength();
  const double newPos = juce::jlimit(0.0, totalLength, positionSeconds);
  auto &audioPlayer = owner.getAudioPlayer();
  const double currentOut = audioPlayer.getCutOut();

  // Crossing Logic: If we manually move In past an Auto-Out, turn off Auto-Out
  if (!silenceDetector.getIsAutoCutInActive() && newPos >= currentOut &&
      silenceDetector.getIsAutoCutOutActive()) {
    owner.setAutoCutOutActive(false);
  }

  audioPlayer.setCutIn(newPos);

  // Push Logic: If In crosses Out and AC In is active
  if (silenceDetector.getIsAutoCutInActive() &&
      newPos >= currentOut) {
    // Push Out to the end, then re-detect if AC Out is active
    setCutOutPosition(totalLength);
    if (silenceDetector.getIsAutoCutOutActive())
      silenceDetector.detectOutSilence();
  }

  // Constrain playback head if it's outside new loopIn
  audioPlayer.setPlayheadPosition(
      audioPlayer.getTransportSource().getCurrentPosition());
  ensureLoopOrder();
}

void LoopPresenter::setCutOutPosition(double positionSeconds) {
  const double totalLength = getAudioTotalLength();
  const double newPos = juce::jlimit(0.0, totalLength, positionSeconds);
  auto &audioPlayer = owner.getAudioPlayer();
  const double currentIn = audioPlayer.getCutIn();

  // Crossing Logic: If we manually move Out past an Auto-In, turn off Auto-In
  if (!silenceDetector.getIsAutoCutOutActive() && newPos <= currentIn &&
      silenceDetector.getIsAutoCutInActive()) {
    owner.setAutoCutInActive(false);
  }

  audioPlayer.setCutOut(newPos);

  // Pull Logic: If Out crosses In and AC Out is active
  if (silenceDetector.getIsAutoCutOutActive() &&
      newPos <= currentIn) {
    // Pull In to the start, then re-detect if AC In is active
    setCutInPosition(0.0);
    if (silenceDetector.getIsAutoCutInActive())
      silenceDetector.detectInSilence();
  }

  // Constrain playback head if it's outside new loopOut
  audioPlayer.setPlayheadPosition(
      audioPlayer.getTransportSource().getCurrentPosition());
  ensureLoopOrder();
}

void LoopPresenter::ensureLoopOrder() {
  auto &audioPlayer = owner.getAudioPlayer();
  double currentIn = audioPlayer.getCutIn();
  double currentOut = audioPlayer.getCutOut();

  if (currentIn > currentOut) {
    std::swap(currentIn, currentOut);
    audioPlayer.setCutIn(currentIn);
    audioPlayer.setCutOut(currentOut);

    // Swap Auto-Cut states as well so the "Auto" property follows the detected
    // value
    bool acIn = silenceDetector.getIsAutoCutInActive();
    bool acOut = silenceDetector.getIsAutoCutOutActive();
    owner.setAutoCutInActive(acOut);
    owner.setAutoCutOutActive(acIn);
  }
}

void LoopPresenter::updateLoopLabels() {
  const double currentIn = owner.getAudioPlayer().getCutIn();
  const double currentOut = owner.getAudioPlayer().getCutOut();
  // Guard against timer overwriting while editing OR focused
  if (!isEditingIn && !cutInEditor.hasKeyboardFocus(true)) {
    syncEditorToPosition(cutInEditor, currentIn);
  }

  if (!isEditingOut && !cutOutEditor.hasKeyboardFocus(true)) {
    syncEditorToPosition(cutOutEditor, currentOut);
  }
}

void LoopPresenter::setLoopStartFromSample(int sampleIndex) {
  AudioPlayer &audioPlayer = owner.getAudioPlayer();
  double sampleRate = 0.0;
  juce::int64 length = 0;
  if (!audioPlayer.getReaderInfo(sampleRate, length) || sampleRate <= 0.0)
    return;

  setCutInPosition((double)sampleIndex / sampleRate);
  ensureLoopOrder();
  updateLoopLabels();
  owner.repaint();
}

void LoopPresenter::setLoopEndFromSample(int sampleIndex) {
  AudioPlayer &audioPlayer = owner.getAudioPlayer();
  double sampleRate = 0.0;
  juce::int64 length = 0;
  if (!audioPlayer.getReaderInfo(sampleRate, length) || sampleRate <= 0.0)
    return;

  setCutOutPosition((double)sampleIndex / sampleRate);
  ensureLoopOrder();
  updateLoopLabels();
  owner.repaint();
}

void LoopPresenter::textEditorTextChanged(juce::TextEditor &editor) {
  if (&editor == &cutInEditor)
    isEditingIn = true;
  else if (&editor == &cutOutEditor)
    isEditingOut = true;

  const double totalLength = getAudioTotalLength();
  TimeEntryHelpers::validateTimeEntry(editor, totalLength);
}

void LoopPresenter::textEditorReturnKeyPressed(juce::TextEditor &editor) {
  if (&editor == &cutInEditor)
    isEditingIn = false;
  if (&editor == &cutOutEditor)
    isEditingOut = false;

  const double newPosition = TimeUtils::parseTime(editor.getText());
  if (&editor == &cutInEditor) {
    applyLoopInFromEditor(newPosition, editor);
  } else if (&editor == &cutOutEditor) {
    applyLoopOutFromEditor(newPosition, editor);
  }
  editor.giveAwayKeyboardFocus();
}

void LoopPresenter::textEditorEscapeKeyPressed(juce::TextEditor &editor) {
  if (&editor == &cutInEditor)
    isEditingIn = false;
  if (&editor == &cutOutEditor)
    isEditingOut = false;

  if (&editor == &cutInEditor) {
    syncEditorToPosition(editor, owner.getAudioPlayer().getCutIn());
  } else if (&editor == &cutOutEditor) {
    syncEditorToPosition(editor, owner.getAudioPlayer().getCutOut());
  }
  editor.setColour(juce::TextEditor::textColourId,
                   Config::Colors::playbackText);
  editor.giveAwayKeyboardFocus();
}

void LoopPresenter::textEditorFocusLost(juce::TextEditor &editor) {
  if (&editor == &cutInEditor)
    isEditingIn = false;
  if (&editor == &cutOutEditor)
    isEditingOut = false;

  const double newPosition = TimeUtils::parseTime(editor.getText());
  if (&editor == &cutInEditor) {
    applyLoopInFromEditor(newPosition, editor);
  } else if (&editor == &cutOutEditor) {
    applyLoopOutFromEditor(newPosition, editor);
  }

  // Clear zoom on focus lost
  owner.setActiveZoomPoint(ControlPanel::ActiveZoomPoint::None);
  owner.performDelayedJumpIfNeeded();
}

void LoopPresenter::mouseDown(const juce::MouseEvent &event) {
  if (event.eventComponent == &cutInEditor)
    isEditingIn = true;
  else if (event.eventComponent == &cutOutEditor)
    isEditingOut = true;
}


double LoopPresenter::getAudioTotalLength() const {
  return owner.getAudioPlayer().getThumbnail().getTotalLength();
}

bool LoopPresenter::applyLoopInFromEditor(double newPosition,
                                          juce::TextEditor &editor) {
  const double totalLength = getAudioTotalLength();
  if (newPosition >= 0.0 && newPosition <= totalLength) {

    setCutInPosition(newPosition);
    owner.updateLoopButtonColors();
    owner.setAutoCutInActive(false);

    if (owner.getActiveZoomPoint() != ControlPanel::ActiveZoomPoint::None)
      owner.setNeedsJumpToLoopIn(true);

    editor.setColour(juce::TextEditor::textColourId,
                     Config::Colors::playbackText);
    owner.repaint();
    updateLoopLabels();
    return true;
  }

  syncEditorToPosition(editor, owner.getAudioPlayer().getCutIn());
  editor.setColour(juce::TextEditor::textColourId,
                   Config::Colors::textEditorError);
  owner.repaint();
  return false;
}

bool LoopPresenter::applyLoopOutFromEditor(double newPosition,
                                           juce::TextEditor &editor) {
  const double totalLength = getAudioTotalLength();
  if (newPosition >= 0.0 && newPosition <= totalLength) {
    AudioPlayer &audioPlayer = owner.getAudioPlayer();
    auto &transport = audioPlayer.getTransportSource();
    if (owner.getShouldLoop() &&
        transport.getCurrentPosition() >= owner.getAudioPlayer().getCutOut())
      owner.getAudioPlayer().setPlayheadPosition(owner.getAudioPlayer().getCutIn());

    setCutOutPosition(newPosition);
    owner.updateLoopButtonColors();
    owner.setAutoCutOutActive(false);

    if (owner.getActiveZoomPoint() != ControlPanel::ActiveZoomPoint::None)
      owner.setNeedsJumpToLoopIn(true);

    editor.setColour(juce::TextEditor::textColourId,
                     Config::Colors::playbackText);
    owner.repaint();
    updateLoopLabels();
    return true;
  }

  syncEditorToPosition(editor, owner.getAudioPlayer().getCutOut());
  editor.setColour(juce::TextEditor::textColourId,
                   Config::Colors::textEditorError);
  owner.repaint();
  return false;
}

void LoopPresenter::syncEditorToPosition(juce::TextEditor &editor,
                                         double positionSeconds) {
  // Multi-layered guard: Check flags AND OS-level focus
  if (editor.hasKeyboardFocus(true) ||
      (&editor == &cutInEditor && isEditingIn) ||
      (&editor == &cutOutEditor && isEditingOut)) {
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
    isEditingIn = true;
  else if (editor == &cutOutEditor)
    isEditingOut = true;

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
        (editor == &cutInEditor && isEditingIn) ||
        (editor == &cutOutEditor && isEditingOut))
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
  juce::int64 length = 0;
  owner.getAudioPlayer().getReaderInfo(sampleRate, length);

  double step =
      TimeEntryHelpers::calculateStepSize(charIndex, event.mods, sampleRate);

  const double direction = (wheel.deltaY > 0) ? 1.0 : -1.0;
  const double delta = direction * step;

  if (editor == &cutInEditor) {
    const double currentIn = owner.getAudioPlayer().getCutIn();
    double newPos = juce::jlimit(0.0, totalLength, currentIn + delta);
    if (newPos != currentIn) {
      setCutInPosition(newPos);
      owner.setAutoCutInActive(false);
      owner.setNeedsJumpToLoopIn(true);
      ensureLoopOrder();
      updateLoopLabels();
      owner.repaint();
    }
  } else if (editor == &cutOutEditor) {
    const double currentOut = owner.getAudioPlayer().getCutOut();
    double newPos = juce::jlimit(0.0, totalLength, currentOut + delta);
    if (newPos != currentOut) {
      setCutOutPosition(newPos);
      owner.setAutoCutOutActive(false);
      owner.setNeedsJumpToLoopIn(true);
      ensureLoopOrder();
      updateLoopLabels();
      owner.repaint();
    }
  }
}
