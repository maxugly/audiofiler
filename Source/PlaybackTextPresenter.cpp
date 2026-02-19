

#include "PlaybackTextPresenter.h"
#include "TimeUtils.h"

#include "AudioPlayer.h"
#include "Config.h"
#include "ControlPanel.h"
#include "TimeEntryHelpers.h"
#include "TimeUtils.h"
#include <cmath>

PlaybackTextPresenter::PlaybackTextPresenter(ControlPanel &ownerPanel)

    : owner(ownerPanel) {}

PlaybackTextPresenter::~PlaybackTextPresenter() {
  owner.elapsedTimeEditor.removeListener(this);
  owner.remainingTimeEditor.removeListener(this);
  owner.cutLengthEditor.removeListener(this);
}

void PlaybackTextPresenter::initialiseEditors() {
  auto configure = [&](juce::TextEditor &ed, juce::Justification just) {
    owner.addAndMakeVisible(ed);
    ed.setReadOnly(false);
    ed.setJustification(just);
    ed.setColour(juce::TextEditor::backgroundColourId,
                 juce::Colours::transparentBlack);
    ed.setColour(juce::TextEditor::outlineColourId,
                 juce::Colours::transparentBlack);
    ed.setColour(juce::TextEditor::textColourId, Config::Colors::playbackText);
    ed.setFont(juce::Font(
        juce::FontOptions((float)Config::Layout::Text::playbackSize)));
    ed.applyFontToAllText(ed.getFont());
    ed.setMultiLine(false);
    ed.setReturnKeyStartsNewLine(false);
    ed.setSelectAllWhenFocused(true);
    ed.addListener(this);
    ed.addMouseListener(this, false);
  };

  configure(owner.elapsedTimeEditor, juce::Justification::left);

  configure(owner.remainingTimeEditor, juce::Justification::right);

  configure(owner.cutLengthEditor, juce::Justification::centred);
}

void PlaybackTextPresenter::updateEditors() {
  if (!isEditingElapsed && !owner.elapsedTimeEditor.hasKeyboardFocus(true))
    syncEditorToPosition(
        owner.elapsedTimeEditor,
        owner.getAudioPlayer().getCurrentPosition());

  if (!isEditingRemaining &&
      !owner.remainingTimeEditor.hasKeyboardFocus(true)) {
    const auto& session = owner.getSessionState();
    const auto cutOut = owner.getAudioPlayer().getCutOut();
    const auto remaining = juce::jmax(
        0.0,
        (session.getCutPrefs().active ? cutOut : owner.getAudioPlayer().getThumbnail().getTotalLength()) -
            owner.getAudioPlayer().getCurrentPosition());

    syncEditorToPosition(owner.remainingTimeEditor, remaining, true);
  }

  if (!isEditingCutLength && !owner.cutLengthEditor.hasKeyboardFocus(true)) {
    double length =
        std::abs(owner.getCutOutPosition() - owner.getCutInPosition());
    juce::String newText = owner.formatTime(length);
    if (owner.cutLengthEditor.getText() != newText)
      owner.cutLengthEditor.setText(newText, juce::dontSendNotification);
  }
}

void PlaybackTextPresenter::layoutEditors() {
  const int textY =
      owner.getBottomRowTopY() - Config::Layout::Text::playbackOffsetY;
  auto [leftX, centreX, rightX] = owner.getPlaybackLabelXs();

  owner.elapsedTimeEditor.setBounds(leftX, textY,
                                    Config::Layout::Text::playbackWidth,
                                    Config::Layout::Text::playbackHeight);
  owner.remainingTimeEditor.setBounds(rightX, textY,
                                      Config::Layout::Text::playbackWidth,
                                      Config::Layout::Text::playbackHeight);

  owner.cutLengthEditor.setBounds(centreX, textY,
                                   Config::Layout::Text::playbackWidth / 2,
                                   Config::Layout::Text::playbackHeight);
}

void PlaybackTextPresenter::render(juce::Graphics &g) const {
  if (owner.getAudioPlayer().getThumbnail().getTotalLength() <= 0.0)
    return;

  const int textY =
      owner.getBottomRowTopY() - Config::Layout::Text::playbackOffsetY;
  auto [leftX, centreX, rightX] = owner.getPlaybackLabelXs();

  g.setColour(Config::Colors::playbackText);
  g.setFont((float)Config::Layout::Text::playbackSize);

  juce::String totalTimeStr = " / " + getTotalTimeStaticString();

  g.drawText(totalTimeStr, centreX + (Config::Layout::Text::playbackWidth / 2),
             textY, Config::Layout::Text::playbackWidth / 2,
             Config::Layout::Text::playbackHeight, juce::Justification::left,
             false);
}

void PlaybackTextPresenter::textEditorTextChanged(juce::TextEditor &editor) {
  if (&editor == &owner.elapsedTimeEditor)
    isEditingElapsed = true;
  else if (&editor == &owner.remainingTimeEditor)
    isEditingRemaining = true;
  else if (&editor == &owner.cutLengthEditor)
    isEditingCutLength = true;

  const double totalLength =
      owner.getAudioPlayer().getThumbnail().getTotalLength();

  TimeEntryHelpers::validateTimeEntry(editor, totalLength);
}

void PlaybackTextPresenter::textEditorReturnKeyPressed(
    juce::TextEditor &editor) {
  if (&editor == &owner.elapsedTimeEditor)
    isEditingElapsed = false;
  else if (&editor == &owner.remainingTimeEditor)
    isEditingRemaining = false;
  else if (&editor == &owner.cutLengthEditor)
    isEditingCutLength = false;

  applyTimeEdit(editor);
  editor.giveAwayKeyboardFocus();
}

void PlaybackTextPresenter::textEditorEscapeKeyPressed(
    juce::TextEditor &editor) {
  if (&editor == &owner.elapsedTimeEditor)
    isEditingElapsed = false;
  else if (&editor == &owner.remainingTimeEditor)
    isEditingRemaining = false;
  else if (&editor == &owner.cutLengthEditor)
    isEditingCutLength = false;

  updateEditors();
  editor.giveAwayKeyboardFocus();
}

void PlaybackTextPresenter::textEditorFocusLost(juce::TextEditor &editor) {
  if (&editor == &owner.elapsedTimeEditor)
    isEditingElapsed = false;
  else if (&editor == &owner.remainingTimeEditor)
    isEditingRemaining = false;
  else if (&editor == &owner.cutLengthEditor)
    isEditingCutLength = false;

  applyTimeEdit(editor);
}

void PlaybackTextPresenter::applyTimeEdit(juce::TextEditor &editor) {
  double newTime = TimeUtils::parseTime(editor.getText());
  if (newTime < 0.0)
    return;

  double totalLength = owner.getAudioPlayer().getThumbnail().getTotalLength();
  double cutOut = owner.getAudioPlayer().getCutOut();

  if (&editor == &owner.elapsedTimeEditor) {
    owner.getAudioPlayer().setPlayheadPosition(newTime);
  } else if (&editor == &owner.remainingTimeEditor) {
    const auto& session = owner.getSessionState();
    const double base = session.getCutPrefs().active ? cutOut : totalLength;
    owner.getAudioPlayer().setPlayheadPosition(base - newTime);
  } else if (&editor == &owner.cutLengthEditor) {
    double currentIn = owner.getCutInPosition();
    newTime = juce::jlimit(0.0, totalLength, newTime);

    double proposedOut = currentIn + newTime;

    if (proposedOut > totalLength) {
      double newIn = totalLength - newTime;
      owner.setCutInPosition(newIn);
      owner.setCutOutPosition(totalLength);
    } else {
      owner.setCutOutPosition(proposedOut);
    }

    owner.ensureCutOrder();
    owner.updateCutLabels();
  }

  updateEditors();
}

void PlaybackTextPresenter::syncEditorToPosition(juce::TextEditor &editor,
                                                 double positionSeconds,
                                                 bool isRemaining) {
  if (editor.hasKeyboardFocus(true) ||
      (&editor == &owner.elapsedTimeEditor && isEditingElapsed) ||
      (&editor == &owner.remainingTimeEditor && isEditingRemaining) ||
      (&editor == &owner.cutLengthEditor && isEditingCutLength)) {
    return;
  }

  juce::String text = owner.formatTime(positionSeconds);
  if (isRemaining)
    text = "-" + text;

  if (editor.getText() != text)
    editor.setText(text, juce::dontSendNotification);
}

void PlaybackTextPresenter::mouseDown(const juce::MouseEvent &event) {
  if (auto *editor = dynamic_cast<juce::TextEditor *>(event.eventComponent)) {
    if (editor == &owner.elapsedTimeEditor)
      isEditingElapsed = true;
    else if (editor == &owner.remainingTimeEditor)
      isEditingRemaining = true;
    else if (editor == &owner.cutLengthEditor)
      isEditingCutLength = true;
  }
}

void PlaybackTextPresenter::mouseUp(const juce::MouseEvent &event) {
  auto *editor = dynamic_cast<juce::TextEditor *>(event.eventComponent);
  if (editor == nullptr)
    return;

  if (editor == &owner.elapsedTimeEditor)
    isEditingElapsed = true;
  else if (editor == &owner.remainingTimeEditor)
    isEditingRemaining = true;
  else if (editor == &owner.cutLengthEditor)
    isEditingCutLength = true;

  bool isNegative = (editor == &owner.remainingTimeEditor) ||
                    editor->getText().startsWith("-");
  int offset = isNegative ? 1 : 0;

  int charIndex = editor->getTextIndexAt(event.getPosition());
  if (charIndex < 0)
    return;

  int effectiveIndex = charIndex - offset;

  juce::Range<int> newRange;

  if (effectiveIndex <= 1)
    newRange = juce::Range<int>(0 + offset, 2 + offset);
  else if (effectiveIndex >= 3 && effectiveIndex <= 4)
    newRange = juce::Range<int>(3 + offset, 5 + offset);
  else if (effectiveIndex >= 6 && effectiveIndex <= 7)
    newRange = juce::Range<int>(6 + offset, 8 + offset);
  else if (effectiveIndex >= 9 && effectiveIndex <= 11)
    newRange = juce::Range<int>(9 + offset, 12 + offset);
  else
    return;

  juce::MessageManager::callAsync([editor, newRange] {
    if (editor != nullptr)
      editor->setHighlightedRegion(newRange);
  });
}

void PlaybackTextPresenter::mouseWheelMove(
    const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) {
  if (wheel.deltaY == 0.0f)
    return;

  auto *editor = dynamic_cast<juce::TextEditor *>(event.eventComponent);
  if (editor == nullptr)
    return;

  if (editor->hasKeyboardFocus(true) ||
      (editor == &owner.elapsedTimeEditor && isEditingElapsed) ||
      (editor == &owner.remainingTimeEditor && isEditingRemaining) ||
      (editor == &owner.cutLengthEditor && isEditingCutLength))
    return;

  double currentVal = TimeUtils::parseTime(editor->getText());
  if (currentVal < 0.0)
    currentVal = 0.0;

  int charIndex = editor->getTextIndexAt(event.getPosition());

  bool isNegative = (editor == &owner.remainingTimeEditor) ||
                    editor->getText().startsWith("-");
  int offset = isNegative ? 1 : 0;
  int effectiveIndex = charIndex - offset;

  double sampleRate = 0.0;
  juce::int64 length = 0;
  owner.getAudioPlayer().getReaderInfo(sampleRate, length);

  double step = TimeEntryHelpers::calculateStepSize(effectiveIndex, event.mods,
                                                    sampleRate);

  double direction = (wheel.deltaY > 0) ? 1.0 : -1.0;
  double newVal = juce::jmax(0.0, currentVal + (direction * step));

  if (editor == &owner.elapsedTimeEditor) {
    owner.getAudioPlayer().setPlayheadPosition(newVal);
  } else if (editor == &owner.remainingTimeEditor) {
    double total = owner.getAudioPlayer().getThumbnail().getTotalLength();
    owner.getAudioPlayer().setPlayheadPosition(total - newVal);
  } else if (editor == &owner.cutLengthEditor) {
    const double totalLength = owner.getAudioPlayer().getThumbnail().getTotalLength();
    newVal = juce::jlimit(0.0, totalLength, newVal);
    double proposedOut = owner.getCutInPosition() + newVal;

    if (proposedOut > totalLength) {
        owner.setCutInPosition(totalLength - newVal);
        owner.setCutOutPosition(totalLength);
    } else {
        owner.setCutOutPosition(proposedOut);
    }

    owner.ensureCutOrder();
    owner.updateCutLabels();
  }

  updateEditors();
}
