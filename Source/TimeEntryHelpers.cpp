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
}
