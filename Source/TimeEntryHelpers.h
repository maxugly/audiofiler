#ifndef AUDIOFILER_TIMEENTRYHELPERS_H
#define AUDIOFILER_TIMEENTRYHELPERS_H

#include <JuceHeader.h>

/**
 * @ingroup Helpers
 * @namespace TimeEntryHelpers
 * @brief Helper functions for validating and manipulating time strings in text editors.
 */
namespace TimeEntryHelpers
{

    /**
     * @brief Validates the time entered in a text editor and updates its color.
     * @details Checks if the text is a valid timestamp (MM:SS.ms) and if it falls
     *          within the valid range (0 to totalLength). Updates the text color
     *          using `Config::Colors` to indicate errors or warnings.
     *
     * @param editor The TextEditor to validate.
     * @param totalLength The maximum allowed time in seconds.
     */
    void validateTimeEntry(juce::TextEditor& editor, double totalLength);

    /**
     * @brief Calculates the time step size based on cursor position and modifier keys.
     * @details Used for "scrubbing" time values with the mouse wheel or arrow keys.
     *          The step size changes depending on which part of the time string (minutes,
     *          seconds, milliseconds) the cursor is hovering over.
     *
     * @param charIndex The character index of the cursor or mouse hover.
     * @param mods The modifier keys currently pressed.
     * @param sampleRate The audio sample rate (used for fine adjustments).
     * @return The step size in seconds.
     */
    double calculateStepSize(int charIndex, const juce::ModifierKeys& mods, double sampleRate = 0.0);
}

#endif 
