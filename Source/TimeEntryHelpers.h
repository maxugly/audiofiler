#ifndef AUDIOFILER_TIMEENTRYHELPERS_H
#define AUDIOFILER_TIMEENTRYHELPERS_H

#include <JuceHeader.h>

namespace TimeEntryHelpers
{
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
}

#endif // AUDIOFILER_TIMEENTRYHELPERS_H
