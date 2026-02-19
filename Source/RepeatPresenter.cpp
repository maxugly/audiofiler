/**
 * @file RepeatPresenter.cpp
 * @brief Defines the RepeatPresenter class.
 * @ingroup Presenters
 */

#include "RepeatPresenter.h"
#include "FocusManager.h"
#include "TimeUtils.h"

#include "AudioPlayer.h"
#include "Config.h"
#include "ControlPanel.h"
#include "SilenceDetector.h"
#include "TimeEntryHelpers.h"
#include "TimeUtils.h"
#include <utility>

RepeatPresenter::RepeatPresenter(ControlPanel &ownerPanel,
                             SilenceDetector &detector,
                             juce::TextEditor &cutIn,
                             juce::TextEditor &cutOut)
    : owner(ownerPanel), silenceDetector(detector), cutInEditor(cutIn),
      /**
       * @brief Undocumented method.
       * @param cutOut [in] Description for cutOut.
       */
      cutOutEditor(cutOut) {
  cutInEditor.addListener(this);
  cutOutEditor.addListener(this);
  cutInEditor.addMouseListener(this, false);
  cutOutEditor.addMouseListener(this, false);
}

/**
 * @brief Undocumented method.
 */
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

  owner.addAndMakeVisible(cutInEditor);
  /**
   * @brief Undocumented method.
   * @param cutInEditor [in] Description for cutInEditor.
   */
  configure(cutInEditor);

  owner.addAndMakeVisible(cutOutEditor);
  /**
   * @brief Undocumented method.
   * @param cutOutEditor [in] Description for cutOutEditor.
   */
  configure(cutOutEditor);
}

/**
 * @brief Undocumented method.
 */
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

/**
 * @brief Undocumented method.
 * @param positionSeconds [in] Description for positionSeconds.
 */
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
    /**
     * @brief Sets the CutOutPosition.
     * @param totalLength [in] Description for totalLength.
     */
    setCutOutPosition(totalLength);
    if (silenceDetector.getIsAutoCutOutActive())
      silenceDetector.detectOutSilence();
  }

  audioPlayer.setPlayheadPosition(
      audioPlayer.getCurrentPosition());
  /**
   * @brief Undocumented method.
   */
  ensureCutOrder();
}

/**
 * @brief Undocumented method.
 * @param positionSeconds [in] Description for positionSeconds.
 */
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
    /**
     * @brief Sets the CutInPosition.
     * @param 0.0 [in] Description for 0.0.
     */
    setCutInPosition(0.0);
    if (silenceDetector.getIsAutoCutInActive())
      silenceDetector.detectInSilence();
  }

  audioPlayer.setPlayheadPosition(
      audioPlayer.getCurrentPosition());
  /**
   * @brief Undocumented method.
   */
  ensureCutOrder();
}

/**
 * @brief Undocumented method.
 */
void RepeatPresenter::ensureCutOrder() {
  auto &audioPlayer = owner.getAudioPlayer();
  double currentIn = audioPlayer.getCutIn();
  double currentOut = audioPlayer.getCutOut();

  if (currentIn > currentOut) {
    /**
     * @brief Undocumented method.
     * @param currentIn [in] Description for currentIn.
     * @param currentOut [in] Description for currentOut.
     */
    std::swap(currentIn, currentOut);
    audioPlayer.setCutIn(currentIn);
    audioPlayer.setCutOut(currentOut);

    bool acIn = silenceDetector.getIsAutoCutInActive();
    bool acOut = silenceDetector.getIsAutoCutOutActive();
    owner.setAutoCutInActive(acOut);
    owner.setAutoCutOutActive(acIn);
  }
}

/**
 * @brief Undocumented method.
 */
void RepeatPresenter::updateCutLabels() {
  const double currentIn = owner.getAudioPlayer().getCutIn();
  const double currentOut = owner.getAudioPlayer().getCutOut();
  if (!isEditingIn && !cutInEditor.hasKeyboardFocus(true)) {
    /**
     * @brief Undocumented method.
     * @param cutInEditor [in] Description for cutInEditor.
     * @param currentIn [in] Description for currentIn.
     */
    syncEditorToPosition(cutInEditor, currentIn);
  }

  if (!isEditingOut && !cutOutEditor.hasKeyboardFocus(true)) {
    /**
     * @brief Undocumented method.
     * @param cutOutEditor [in] Description for cutOutEditor.
     * @param currentOut [in] Description for currentOut.
     */
    syncEditorToPosition(cutOutEditor, currentOut);
  }
}

/**
 * @brief Undocumented method.
 * @param sampleIndex [in] Description for sampleIndex.
 */
void RepeatPresenter::setCutStartFromSample(int sampleIndex) {
  AudioPlayer &audioPlayer = owner.getAudioPlayer();
  double sampleRate = 0.0;
  juce::int64 length = 0;
  if (!audioPlayer.getReaderInfo(sampleRate, length) || sampleRate <= 0.0)
    return;

  setCutInPosition((double)sampleIndex / sampleRate);
  /**
   * @brief Undocumented method.
   */
  ensureCutOrder();
  /**
   * @brief Undocumented method.
   */
  updateCutLabels();
  owner.repaint();
}

/**
 * @brief Undocumented method.
 * @param sampleIndex [in] Description for sampleIndex.
 */
void RepeatPresenter::setCutEndFromSample(int sampleIndex) {
  AudioPlayer &audioPlayer = owner.getAudioPlayer();
  double sampleRate = 0.0;
  juce::int64 length = 0;
  if (!audioPlayer.getReaderInfo(sampleRate, length) || sampleRate <= 0.0)
    return;

  setCutOutPosition((double)sampleIndex / sampleRate);
  /**
   * @brief Undocumented method.
   */
  ensureCutOrder();
  /**
   * @brief Undocumented method.
   */
  updateCutLabels();
  owner.repaint();
}

/**
 * @brief Undocumented method.
 * @param editor [in] Description for editor.
 */
void RepeatPresenter::textEditorTextChanged(juce::TextEditor &editor) {
  if (&editor == &cutInEditor)
    isEditingIn = true;
  else if (&editor == &cutOutEditor)
    isEditingOut = true;

  const double totalLength = getAudioTotalLength();
  /**
   * @brief Undocumented method.
   * @param editor [in] Description for editor.
   * @param totalLength [in] Description for totalLength.
   */
  TimeEntryHelpers::validateTimeEntry(editor, totalLength);
}

/**
 * @brief Undocumented method.
 * @param editor [in] Description for editor.
 */
void RepeatPresenter::textEditorReturnKeyPressed(juce::TextEditor &editor) {
  if (&editor == &cutInEditor)
    isEditingIn = false;
  if (&editor == &cutOutEditor)
    isEditingOut = false;

  const double newPosition = TimeUtils::parseTime(editor.getText());
  if (&editor == &cutInEditor) {
    /**
     * @brief Undocumented method.
     * @param newPosition [in] Description for newPosition.
     * @param editor [in] Description for editor.
     */
    applyCutInFromEditor(newPosition, editor);
  } else if (&editor == &cutOutEditor) {
    /**
     * @brief Undocumented method.
     * @param newPosition [in] Description for newPosition.
     * @param editor [in] Description for editor.
     */
    applyCutOutFromEditor(newPosition, editor);
  }
  editor.giveAwayKeyboardFocus();
}

/**
 * @brief Undocumented method.
 * @param editor [in] Description for editor.
 */
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

/**
 * @brief Undocumented method.
 * @param editor [in] Description for editor.
 */
void RepeatPresenter::textEditorFocusLost(juce::TextEditor &editor) {
  if (&editor == &cutInEditor)
    isEditingIn = false;
  if (&editor == &cutOutEditor)
    isEditingOut = false;

  const double newPosition = TimeUtils::parseTime(editor.getText());
  if (&editor == &cutInEditor) {
    /**
     * @brief Undocumented method.
     * @param newPosition [in] Description for newPosition.
     * @param editor [in] Description for editor.
     */
    applyCutInFromEditor(newPosition, editor);
  } else if (&editor == &cutOutEditor) {
    /**
     * @brief Undocumented method.
     * @param newPosition [in] Description for newPosition.
     * @param editor [in] Description for editor.
     */
    applyCutOutFromEditor(newPosition, editor);
  }

  owner.setActiveZoomPoint(ControlPanel::ActiveZoomPoint::None);
  owner.performDelayedJumpIfNeeded();
}

/**
 * @brief Undocumented method.
 * @param event [in] Description for event.
 */
void RepeatPresenter::mouseDown(const juce::MouseEvent &event) {
  if (event.eventComponent == &cutInEditor)
    isEditingIn = true;
  else if (event.eventComponent == &cutOutEditor)
    isEditingOut = true;
}


/**
 * @brief Undocumented method.
 * @return double
 */
double RepeatPresenter::getAudioTotalLength() const {
  return owner.getAudioPlayer().getThumbnail().getTotalLength();
}

bool RepeatPresenter::applyCutInFromEditor(double newPosition,
                                          juce::TextEditor &editor) {
  const double totalLength = getAudioTotalLength();
  if (newPosition >= 0.0 && newPosition <= totalLength) {

    /**
     * @brief Sets the CutInPosition.
     * @param newPosition [in] Description for newPosition.
     */
    setCutInPosition(newPosition);
    owner.updateCutButtonColors();
    owner.setAutoCutInActive(false);

    if (owner.getActiveZoomPoint() != ControlPanel::ActiveZoomPoint::None)
      owner.setNeedsJumpToCutIn(true);

    editor.setColour(juce::TextEditor::textColourId,
                     Config::Colors::playbackText);
    owner.repaint();
    /**
     * @brief Undocumented method.
     */
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

    /**
     * @brief Sets the CutOutPosition.
     * @param newPosition [in] Description for newPosition.
     */
    setCutOutPosition(newPosition);
    owner.updateCutButtonColors();
    owner.setAutoCutOutActive(false);

    if (owner.getActiveZoomPoint() != ControlPanel::ActiveZoomPoint::None)
      owner.setNeedsJumpToCutIn(true);

    editor.setColour(juce::TextEditor::textColourId,
                     Config::Colors::playbackText);
    owner.repaint();
    /**
     * @brief Undocumented method.
     */
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

/**
 * @brief Undocumented method.
 * @param event [in] Description for event.
 */
void RepeatPresenter::mouseEnter(const juce::MouseEvent &event) {
  if (event.eventComponent == &cutInEditor)
    owner.setActiveZoomPoint(ControlPanel::ActiveZoomPoint::In);
  else if (event.eventComponent == &cutOutEditor)
    owner.setActiveZoomPoint(ControlPanel::ActiveZoomPoint::Out);
}

/**
 * @brief Undocumented method.
 * @param event [in] Description for event.
 */
void RepeatPresenter::mouseExit(const juce::MouseEvent &event) {
  auto *editor = dynamic_cast<juce::TextEditor *>(event.eventComponent);
  if (editor != nullptr && !editor->hasKeyboardFocus(false)) {
    owner.setActiveZoomPoint(ControlPanel::ActiveZoomPoint::None);
    owner.performDelayedJumpIfNeeded();
  }
}

/**
 * @brief Undocumented method.
 * @param event [in] Description for event.
 */
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
      /**
       * @brief Undocumented method.
       * @param charIndex [in] Description for charIndex.
       * @param event.mods [in] Description for event.mods.
       * @param sampleRate [in] Description for sampleRate.
       */
      TimeEntryHelpers::calculateStepSize(charIndex, event.mods, sampleRate);

  const double direction = (wheel.deltaY > 0) ? 1.0 : -1.0;
  const double delta = direction * step;

  if (editor == &cutInEditor) {
    const double currentIn = owner.getAudioPlayer().getCutIn();
    double newPos = currentIn + delta;
    if (newPos != currentIn) {
      /**
       * @brief Sets the CutInPosition.
       * @param newPos [in] Description for newPos.
       */
      setCutInPosition(newPos);
      owner.setAutoCutInActive(false);
      owner.setNeedsJumpToCutIn(true);
      /**
       * @brief Undocumented method.
       */
      ensureCutOrder();
      /**
       * @brief Undocumented method.
       */
      updateCutLabels();
      owner.repaint();
    }
  } else if (editor == &cutOutEditor) {
    const double currentOut = owner.getAudioPlayer().getCutOut();
    double newPos = currentOut + delta;
    if (newPos != currentOut) {
      /**
       * @brief Sets the CutOutPosition.
       * @param newPos [in] Description for newPos.
       */
      setCutOutPosition(newPos);
      owner.setAutoCutOutActive(false);
      owner.setNeedsJumpToCutIn(true);
      /**
       * @brief Undocumented method.
       */
      ensureCutOrder();
      /**
       * @brief Undocumented method.
       */
      updateCutLabels();
      owner.repaint();
    }
  }
}
