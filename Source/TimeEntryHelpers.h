#ifndef AUDIOFILER_TIMEENTRYHELPERS_H
#define AUDIOFILER_TIMEENTRYHELPERS_H

#ifdef JUCE_HEADLESS
 #include <juce_core/juce_core.h>
#else
 #include <JuceHeader.h>
#endif

#include "TimeUtils.h"

namespace TimeEntryHelpers
{
#ifndef JUCE_HEADLESS
    /**
     * @brief Validates the time entered in a TextEditor and updates its text colour.
     *
     * Parses the text in the editor using TimeUtils::parseTime. If the time is valid
     * (non-negative and within [0, totalLength]), the text colour is set to Config::Colors::playbackText.
     * If invalid (-1.0), it is set to Config::Colors::textEditorError.
     * Otherwise (out of range), it is set to Config::Colors::textEditorWarning.
     *
     * @param editor The TextEditor to validate.
     * @param totalLength The total length of the audio file in seconds.
     */
    void validateTimeEntry(juce::TextEditor& editor, double totalLength);

    /**
     * @brief Calculates the step size for mouse wheel adjustments based on cursor position and modifiers.
     *
     * @param charIndex The character index of the cursor in the text editor (normalized to HH:MM:SS:mmm).
     * @param mods The modifier keys pressed.
     * @param sampleRate The sample rate of the audio (if available) for sample-accurate stepping.
     * @return The calculated step size in seconds.
     */
    double calculateStepSize(int charIndex, const juce::ModifierKeys& mods, double sampleRate = 0.0);
#endif
}

#endif // AUDIOFILER_TIMEENTRYHELPERS_H
