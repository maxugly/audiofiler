/**
 * @file TimeEntryHelpers.cpp
 * @brief Defines the TimeEntryHelpers class.
 * @ingroup Engine
 */

#include "TimeEntryHelpers.h"
#include "TimeUtils.h"
#include "Config.h"

namespace TimeEntryHelpers
{
    void validateTimeEntry(juce::TextEditor& editor, double totalLength)
    {
        const double newPosition = TimeUtils::parseTime(editor.getText());

        if (newPosition >= 0.0 && newPosition <= totalLength)
        {
            editor.setColour(juce::TextEditor::textColourId, Config::Colors::playbackText);
        }
        else if (newPosition == -1.0)
        {
            editor.setColour(juce::TextEditor::textColourId, Config::Colors::textEditorError);
        }
        else
        {
            editor.setColour(juce::TextEditor::textColourId, Config::Colors::textEditorWarning);
        }
    }

    double calculateStepSize(int charIndex, const juce::ModifierKeys& mods, double sampleRate)
    {
        double step = Config::Audio::cutStepMilliseconds;
        bool isMillis = false;

        // Determine base step from cursor position
        if (charIndex >= 0 && charIndex <= 1)      // HH
            step = Config::Audio::cutStepHours;
        else if (charIndex >= 3 && charIndex <= 4) // MM
            step = Config::Audio::cutStepMinutes;
        else if (charIndex >= 6 && charIndex <= 7) // SS
            step = Config::Audio::cutStepSeconds;
        else if (charIndex >= 9)                   // mmm
            isMillis = true;

        if (isMillis)
        {
            if (mods.isCtrlDown() && mods.isShiftDown())
            {
                 step = (sampleRate > 0.0) ? (1.0 / sampleRate) : 0.0001;
            }
            else if (mods.isShiftDown())
            {
                step = Config::Audio::cutStepMillisecondsFine;
            }
        }
        else
        {
            double multiplier = 1.0;
            if (mods.isShiftDown() && mods.isCtrlDown())
                multiplier = 0.01;
            else if (mods.isShiftDown())
                multiplier = 0.1;

            step *= multiplier;
        }

        if (mods.isAltDown())
            step *= 10.0;

        return step;
    }
}
