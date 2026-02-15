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
  owner.loopLengthEditor.removeListener(this);
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
  configure(owner.loopLengthEditor, juce::Justification::centred);
}

void PlaybackTextPresenter::updateEditors() {
  if (!isEditingElapsed && !owner.elapsedTimeEditor.hasKeyboardFocus(true))
    syncEditorToPosition(
        owner.elapsedTimeEditor,
        owner.getAudioPlayer().getTransportSource().getCurrentPosition());

  if (!isEditingRemaining &&
      !owner.remainingTimeEditor.hasKeyboardFocus(true)) {
    const auto total = owner.getAudioPlayer().getThumbnail().getTotalLength();
    const auto remaining = juce::jmax(
        0.0,
        total -
            owner.getAudioPlayer().getTransportSource().getCurrentPosition());
    syncEditorToPosition(owner.remainingTimeEditor, remaining, true);
  }

  if (!isEditingLoopLength && !owner.loopLengthEditor.hasKeyboardFocus(true)) {
    double length =
        std::abs(owner.getLoopOutPosition() - owner.getLoopInPosition());
    juce::String newText = owner.formatTime(length);
    if (owner.loopLengthEditor.getText() != newText)
      owner.loopLengthEditor.setText(newText, juce::dontSendNotification);
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

  // The center is special because it's shared with total time which is static
  owner.loopLengthEditor.setBounds(centreX, textY,
                                   Config::Layout::Text::playbackWidth / 2,
                                   Config::Layout::Text::playbackHeight);
}

void PlaybackTextPresenter::render(juce::Graphics &g) const {
  if (owner.getAudioPlayer().getThumbnail().getTotalLength() <= 0.0)
    return;

  // Draw the static total time part behind or next to the loop length
  const int textY =
      owner.getBottomRowTopY() - Config::Layout::Text::playbackOffsetY;
  auto [leftX, centreX, rightX] = owner.getPlaybackLabelXs();

  g.setColour(Config::Colors::playbackText);
  g.setFont((float)Config::Layout::Text::playbackSize);

  juce::String totalTimeStr = " / " + getTotalTimeStaticString();

  // Position it relative to the loop length editor
  int loopLenX = owner.loopLengthEditor.getX();
  int loopLenW = owner.loopLengthEditor.getWidth();

  // We want to draw it centered-ish with the loop length
  // But since loopLength is its own editor, let's just draw totalTime to the
  // right of centreX + playbackTextWidth/2
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
  else if (&editor == &owner.loopLengthEditor)
    isEditingLoopLength = true;

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
  else if (&editor == &owner.loopLengthEditor)
    isEditingLoopLength = false;

  applyTimeEdit(editor);
  editor.giveAwayKeyboardFocus();
}

void PlaybackTextPresenter::textEditorEscapeKeyPressed(
    juce::TextEditor &editor) {
  if (&editor == &owner.elapsedTimeEditor)
    isEditingElapsed = false;
  else if (&editor == &owner.remainingTimeEditor)
    isEditingRemaining = false;
  else if (&editor == &owner.loopLengthEditor)
    isEditingLoopLength = false;

  updateEditors(); // Reset to current state
  editor.giveAwayKeyboardFocus();
}

void PlaybackTextPresenter::textEditorFocusLost(juce::TextEditor &editor) {
  if (&editor == &owner.elapsedTimeEditor)
    isEditingElapsed = false;
  else if (&editor == &owner.remainingTimeEditor)
    isEditingRemaining = false;
  else if (&editor == &owner.loopLengthEditor)
    isEditingLoopLength = false;

  applyTimeEdit(editor);
}

void PlaybackTextPresenter::applyTimeEdit(juce::TextEditor &editor) {
  double newTime = TimeUtils::parseTime(editor.getText());
  if (newTime < 0.0)
    return;

  auto &transport = owner.getAudioPlayer().getTransportSource();
  double totalLength = owner.getAudioPlayer().getThumbnail().getTotalLength();

  if (&editor == &owner.elapsedTimeEditor) {
    transport.setPosition(juce::jlimit(0.0, totalLength, newTime));
  } else if (&editor == &owner.remainingTimeEditor) {
    transport.setPosition(
        juce::jlimit(0.0, totalLength, totalLength - newTime));
  } else if (&editor == &owner.loopLengthEditor) {
    // Adjust loop out based on loop in
    double currentIn = owner.getLoopInPosition();

    // Clamp length to total length to prevent issues if loopIn is 0
    newTime = juce::jlimit(0.0, totalLength, newTime);

    double proposedOut = currentIn + newTime;

    if (proposedOut > totalLength) {
      // Shift In backwards so that [In, Out] has length newTime and Out <=
      // totalLength
      double newIn = totalLength - newTime;
      owner.setLoopInPosition(newIn);
      owner.setLoopOutPosition(totalLength);
    } else {
      owner.setLoopOutPosition(proposedOut);
    }

    owner.ensureLoopOrder();
    owner.updateLoopLabels();
  }

  updateEditors();
}

void PlaybackTextPresenter::syncEditorToPosition(juce::TextEditor &editor,
                                                 double positionSeconds,
                                                 bool isRemaining) {
  juce::String text = owner.formatTime(positionSeconds);
  if (isRemaining)
    text = "-" + text;

  if (editor.getText() != text)
    editor.setText(text, juce::dontSendNotification);
}

void PlaybackTextPresenter::mouseDown(const juce::MouseEvent &event) {
  if (event.eventComponent == &owner.elapsedTimeEditor)
    isEditingElapsed = true;
  else if (event.eventComponent == &owner.remainingTimeEditor)
    isEditingRemaining = true;
  else if (event.eventComponent == &owner.loopLengthEditor)
    isEditingLoopLength = true;
}

void PlaybackTextPresenter::mouseUp(const juce::MouseEvent &event) {
  auto *editor = dynamic_cast<juce::TextEditor *>(event.eventComponent);
  if (editor == nullptr)
    return;

  if (editor == &owner.elapsedTimeEditor)
    isEditingElapsed = true;
  else if (editor == &owner.remainingTimeEditor)
    isEditingRemaining = true;
  else if (editor == &owner.loopLengthEditor)
    isEditingLoopLength = true;

  editor->grabKeyboardFocus();

  // Check if it's remaining time (starts with '-')
  bool isNegative = (editor == &owner.remainingTimeEditor) ||
                    editor->getText().startsWith("-");
  int offset = isNegative ? 1 : 0;

  int charIndex = editor->getTextIndexAt(event.getPosition());
  if (charIndex < 0)
    return;

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

  juce::Range<int> newRange;

  if (effectiveIndex <= 1)
    newRange = juce::Range<int>(0 + offset, 2 + offset); // HH
  else if (effectiveIndex >= 3 && effectiveIndex <= 4)
    newRange = juce::Range<int>(3 + offset, 5 + offset); // MM
  else if (effectiveIndex >= 6 && effectiveIndex <= 7)
    newRange = juce::Range<int>(6 + offset, 8 + offset); // SS
  else if (effectiveIndex >= 9 && effectiveIndex <= 11)
    newRange = juce::Range<int>(9 + offset, 12 + offset); // mmm
  else
    return;

  // Use callAsync to set the selection AFTER the TextEditor's internal mouseUp
  // has finished. This prevents the TextEditor from resetting the caret
  // position immediately.
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

  // Don't allow wheel adjustments if we are actively typing
  if (editor->hasKeyboardFocus(true))
    return;

  double currentVal = TimeUtils::parseTime(editor->getText());
  if (currentVal < 0.0)
    currentVal = 0.0;

  // Determine step size
  int charIndex = editor->getTextIndexAt(event.getPosition());

  // Check if it's remaining time (starts with '-')
  bool isNegative = (editor == &owner.remainingTimeEditor) ||
                    editor->getText().startsWith("-");
  int offset = isNegative ? 1 : 0;
  int effectiveIndex = charIndex - offset;

  double sampleRate = 0.0;
  if (auto *reader = owner.getAudioPlayer().getAudioFormatReader())
    sampleRate = reader->sampleRate;

  double step = TimeEntryHelpers::calculateStepSize(effectiveIndex, event.mods,
                                                    sampleRate);

  double direction = (wheel.deltaY > 0) ? 1.0 : -1.0;
  double newVal = juce::jmax(0.0, currentVal + (direction * step));

  if (editor == &owner.elapsedTimeEditor) {
    owner.getAudioPlayer().getTransportSource().setPosition(newVal);
  } else if (editor == &owner.remainingTimeEditor) {
    double total = owner.getAudioPlayer().getThumbnail().getTotalLength();
    owner.getAudioPlayer().getTransportSource().setPosition(total - newVal);
  } else if (editor == &owner.loopLengthEditor) {
    owner.setLoopOutPosition(owner.getLoopInPosition() + newVal);
    owner.ensureLoopOrder();
    owner.updateLoopLabels();
  }

  updateEditors();
}
