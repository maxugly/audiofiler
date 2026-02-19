

#ifndef AUDIOFILER_TIMEENTRYHELPERS_H
#define AUDIOFILER_TIMEENTRYHELPERS_H

#include <JuceHeader.h>

namespace TimeEntryHelpers
{

    void validateTimeEntry(juce::TextEditor& editor, double totalLength);

    double calculateStepSize(int charIndex, const juce::ModifierKeys& mods, double sampleRate = 0.0);
}

#endif 
