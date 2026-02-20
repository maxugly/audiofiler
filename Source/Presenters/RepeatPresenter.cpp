

#include "Presenters/RepeatPresenter.h"
#include "UI/FocusManager.h"
#include "Utils/TimeUtils.h"

#include "Core/AudioPlayer.h"
#include "Utils/Config.h"
#include "UI/ControlPanel.h"
#include "Workers/SilenceDetector.h"
#include "Utils/TimeEntryHelpers.h"
#include "Utils/TimeUtils.h"
#include <utility>

RepeatPresenter::RepeatPresenter(ControlPanel &ownerPanel,
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

void RepeatPresenter::initialiseEditors() {
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

  configure(cutInEditor);
  configure(cutOutEditor);
}

RepeatPresenter::~RepeatPresenter() {
  cutInEditor.removeListener(this);
  cutOutEditor.removeListener(this);
  cutInEditor.removeMouseListener(this);
  cutOutEditor.removeMouseListener(this);
}

double RepeatPresenter::getCutInPosition() const noexcept {
  return owner.getAudioPlayer().getCutIn();
}

double RepeatPresenter::getCutOutPosition() const noexcept {
  return owner.getAudioPlayer().getCutOut();
}

void RepeatPresenter::setCutInPosition(double positionSeconds) {
  const double totalLength = getAudioTotalLength();
  const double newPos = positionSeconds;
  auto &audioPlayer = owner.getAudioPlayer();
  const double currentOut = audioPlayer.getCutOut();

  if (!silenceDetector.getIsAutoCutInActive() && newPos >= currentOut &&
      silenceDetector.getIsAutoCutOutActive()) {
    owner.setAutoCutOutActive(false);
  }

  audioPlayer.setCutIn(newPos);

  if (silenceDetector.getIsAutoCutInActive() &&
      newPos >= currentOut) {

    setCutOutPosition(totalLength);
    if (silenceDetector.getIsAutoCutOutActive())
      silenceDetector.detectOutSilence();
  }

  audioPlayer.setPlayheadPosition(
      audioPlayer.getCurrentPosition());

  ensureCutOrder();
}

void RepeatPresenter::setCutOutPosition(double positionSeconds) {
  const double newPos = positionSeconds;
  auto &audioPlayer = owner.getAudioPlayer();
  const double currentIn = audioPlayer.getCutIn();

  if (!silenceDetector.getIsAutoCutOutActive() && newPos <= currentIn &&
      silenceDetector.getIsAutoCutInActive()) {
    owner.setAutoCutInActive(false);
  }

  audioPlayer.setCutOut(newPos);

  if (silenceDetector.getIsAutoCutOutActive() &&
      newPos <= currentIn) {

    setCutInPosition(0.0);
    if (silenceDetector.getIsAutoCutInActive())
      silenceDetector.detectInSilence();
  }

  audioPlayer.setPlayheadPosition(
      audioPlayer.getCurrentPosition());

  ensureCutOrder();
}

void RepeatPresenter::ensureCutOrder() {
  auto &audioPlayer = owner.getAudioPlayer();
  double currentIn = audioPlayer.getCutIn();
  double currentOut = audioPlayer.getCutOut();

  if (currentIn > currentOut) {

    std::swap(currentIn, currentOut);
    audioPlayer.setCutIn(currentIn);
    audioPlayer.setCutOut(currentOut);

    bool acIn = silenceDetector.getIsAutoCutInActive();
    bool acOut = silenceDetector.getIsAutoCutOutActive();
    owner.setAutoCutInActive(acOut);
    owner.setAutoCutOutActive(acIn);
  }
}

void RepeatPresenter::updateCutLabels() {
  const double currentIn = owner.getAudioPlayer().getCutIn();
  const double currentOut = owner.getAudioPlayer().getCutOut();
  if (!isEditingIn && !cutInEditor.hasKeyboardFocus(true)) {

    syncEditorToPosition(cutInEditor, currentIn);
  }

  if (!isEditingOut && !cutOutEditor.hasKeyboardFocus(true)) {

    syncEditorToPosition(cutOutEditor, currentOut);
  }
}

void RepeatPresenter::setCutStartFromSample(int sampleIndex) {
  AudioPlayer &audioPlayer = owner.getAudioPlayer();
  double sampleRate = 0.0;
  juce::int64 length = 0;
  if (!audioPlayer.getReaderInfo(sampleRate, length) || sampleRate <= 0.0)
    return;

  setCutInPosition((double)sampleIndex / sampleRate);

  ensureCutOrder();

  updateCutLabels();
  owner.repaint();
}

void RepeatPresenter::setCutEndFromSample(int sampleIndex) {
  AudioPlayer &audioPlayer = owner.getAudioPlayer();
  double sampleRate = 0.0;
  juce::int64 length = 0;
  if (!audioPlayer.getReaderInfo(sampleRate, length) || sampleRate <= 0.0)
    return;

  setCutOutPosition((double)sampleIndex / sampleRate);

  ensureCutOrder();

  updateCutLabels();
  owner.repaint();
}

void RepeatPresenter::textEditorTextChanged(juce::TextEditor &editor) {
  if (&editor == &cutInEditor)
    isEditingIn = true;
  else if (&editor == &cutOutEditor)
    isEditingOut = true;

  const double totalLength = getAudioTotalLength();

  TimeEntryHelpers::validateTimeEntry(editor, totalLength);
}

void RepeatPresenter::textEditorReturnKeyPressed(juce::TextEditor &editor) {
  if (&editor == &cutInEditor)
    isEditingIn = false;
  if (&editor == &cutOutEditor)
    isEditingOut = false;

  const double newPosition = TimeUtils::parseTime(editor.getText());
  if (&editor == &cutInEditor) {

    applyCutInFromEditor(newPosition, editor);
  } else if (&editor == &cutOutEditor) {

    applyCutOutFromEditor(newPosition, editor);
  }
  editor.giveAwayKeyboardFocus();
}

void RepeatPresenter::textEditorEscapeKeyPressed(juce::TextEditor &editor) {
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

void RepeatPresenter::textEditorFocusLost(juce::TextEditor &editor) {
  if (&editor == &cutInEditor)
    isEditingIn = false;
  if (&editor == &cutOutEditor)
    isEditingOut = false;

  const double newPosition = TimeUtils::parseTime(editor.getText());
  if (&editor == &cutInEditor) {

    applyCutInFromEditor(newPosition, editor);
  } else if (&editor == &cutOutEditor) {

    applyCutOutFromEditor(newPosition, editor);
  }

  owner.getPlaybackTimerManager().setManualZoomPoint(AppEnums::ActiveZoomPoint::None);
}

void RepeatPresenter::mouseDown(const juce::MouseEvent &event) {
  if (event.eventComponent == &cutInEditor)
    isEditingIn = true;
  else if (event.eventComponent == &cutOutEditor)
    isEditingOut = true;
}

double RepeatPresenter::getAudioTotalLength() const {
  return owner.getAudioPlayer().getThumbnail().getTotalLength();
}

bool RepeatPresenter::applyCutInFromEditor(double newPosition,
                                          juce::TextEditor &editor) {
  const double totalLength = getAudioTotalLength();
  if (newPosition >= 0.0 && newPosition <= totalLength) {

    setCutInPosition(newPosition);
    owner.updateCutButtonColors();
    owner.setAutoCutInActive(false);

    if (owner.getActiveZoomPoint() != AppEnums::ActiveZoomPoint::None)
      owner.setNeedsJumpToCutIn(true);

    editor.setColour(juce::TextEditor::textColourId,
                     Config::Colors::playbackText);
    owner.repaint();

    updateCutLabels();
    return true;
  }

  syncEditorToPosition(editor, owner.getAudioPlayer().getCutIn());
  editor.setColour(juce::TextEditor::textColourId,
                   Config::Colors::textEditorError);
  owner.repaint();
  return false;
}

bool RepeatPresenter::applyCutOutFromEditor(double newPosition,
                                           juce::TextEditor &editor) {
  const double totalLength = getAudioTotalLength();
  if (newPosition >= 0.0 && newPosition <= totalLength) {
    AudioPlayer &audioPlayer = owner.getAudioPlayer();
    if (owner.getShouldRepeat() &&
        audioPlayer.getCurrentPosition() >= owner.getAudioPlayer().getCutOut())
      owner.getAudioPlayer().setPlayheadPosition(owner.getAudioPlayer().getCutIn());

    setCutOutPosition(newPosition);
    owner.updateCutButtonColors();
    owner.setAutoCutOutActive(false);

    if (owner.getActiveZoomPoint() != AppEnums::ActiveZoomPoint::None)
      owner.setNeedsJumpToCutIn(true);

    editor.setColour(juce::TextEditor::textColourId,
                     Config::Colors::playbackText);
    owner.repaint();

    updateCutLabels();
    return true;
  }

  syncEditorToPosition(editor, owner.getAudioPlayer().getCutOut());
  editor.setColour(juce::TextEditor::textColourId,
                   Config::Colors::textEditorError);
  owner.repaint();
  return false;
}

void RepeatPresenter::syncEditorToPosition(juce::TextEditor &editor,
                                         double positionSeconds) {
  if (editor.hasKeyboardFocus(true) ||
      (&editor == &cutInEditor && isEditingIn) ||
      (&editor == &cutOutEditor && isEditingOut)) {
    return;
  }

  juce::String newText = owner.formatTime(positionSeconds);
  if (editor.getText() != newText)
    editor.setText(newText, juce::dontSendNotification);
}

void RepeatPresenter::mouseEnter(const juce::MouseEvent &event) {
  if (event.eventComponent == &cutInEditor)
    owner.getPlaybackTimerManager().setManualZoomPoint(AppEnums::ActiveZoomPoint::In);
  else if (event.eventComponent == &cutOutEditor)
    owner.getPlaybackTimerManager().setManualZoomPoint(AppEnums::ActiveZoomPoint::Out);
}

void RepeatPresenter::mouseExit(const juce::MouseEvent &event) {
  auto *editor = dynamic_cast<juce::TextEditor *>(event.eventComponent);
  if (editor != nullptr && !editor->hasKeyboardFocus(false)) {
    owner.getPlaybackTimerManager().setManualZoomPoint(AppEnums::ActiveZoomPoint::None);
  }
}

void RepeatPresenter::mouseUp(const juce::MouseEvent &event) {
  auto *editor = dynamic_cast<juce::TextEditor *>(event.eventComponent);
  if (editor == nullptr)
    return;

  if (editor == &cutInEditor)
    isEditingIn = true;
  else if (editor == &cutOutEditor)
    isEditingOut = true;

  int charIndex = editor->getTextIndexAt(event.getPosition());
  if (charIndex < 0)
    return;

  juce::Range<int> newRange;

  if (charIndex <= 1)
    newRange = juce::Range<int>(0, 2);
  else if (charIndex >= 3 && charIndex <= 4)
    newRange = juce::Range<int>(3, 5);
  else if (charIndex >= 6 && charIndex <= 7)
    newRange = juce::Range<int>(6, 8);
  else if (charIndex >= 9 && charIndex <= 11)
    newRange = juce::Range<int>(9, 12);
  else
    return;

  juce::MessageManager::callAsync([editor, newRange] {
    if (editor != nullptr)
      editor->setHighlightedRegion(newRange);
  });
}

void RepeatPresenter::mouseWheelMove(const juce::MouseEvent &event,
                                   const juce::MouseWheelDetails &wheel) {
  if (wheel.deltaY == 0.0f)
    return;

  auto *editor = dynamic_cast<juce::TextEditor *>(event.eventComponent);
  if (editor != nullptr) {
    if (editor->hasKeyboardFocus(true) ||
        (editor == &cutInEditor && isEditingIn) ||
        (editor == &cutOutEditor && isEditingOut))
      return;
  }

  if (event.mods.isCtrlDown() && !event.mods.isShiftDown()) {
    float currentZoom = owner.getZoomFactor();
    float zoomDelta = wheel.deltaY > 0 ? 1.1f : 0.9f;
    owner.setZoomFactor(currentZoom * zoomDelta);
    return;
  }

  if (editor == nullptr)
    return;

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
    double newPos = currentIn + delta;
    if (newPos != currentIn) {

      setCutInPosition(newPos);
      owner.setAutoCutInActive(false);
      owner.setNeedsJumpToCutIn(true);

      ensureCutOrder();

      updateCutLabels();
      owner.repaint();
    }
  } else if (editor == &cutOutEditor) {
    const double currentOut = owner.getAudioPlayer().getCutOut();
    double newPos = currentOut + delta;
    if (newPos != currentOut) {

      setCutOutPosition(newPos);
      owner.setAutoCutOutActive(false);
      owner.setNeedsJumpToCutIn(true);

      ensureCutOrder();

      updateCutLabels();
      owner.repaint();
    }
  }
}
