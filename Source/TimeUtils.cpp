#include "TimeUtils.h"

juce::String TimeUtils::formatTime(double seconds) {
  if (seconds < 0) seconds = 0;
  int hours = (int)(seconds / 3600.0);
  int minutes = ((int)(seconds / 60.0)) % 60;
  int secs = ((int)seconds) % 60;
  int milliseconds = (int)((seconds - (int)seconds) * 1000.0);
  return juce::String::formatted("%02d:%02d:%02d:%03d", hours, minutes, secs, milliseconds);
}

double TimeUtils::parseTime(const juce::String& timeString)
{
    // Remove leading '-' if present for remaining time
    juce::String cleanTime = timeString.startsWithChar('-') ? timeString.substring(1) : timeString;

    auto parts = juce::StringArray::fromTokens(cleanTime, ":", "");
    if (parts.size() != 4)
        return -1.0;

    return parts[0].getIntValue() * 3600.0
         + parts[1].getIntValue() * 60.0
         + parts[2].getIntValue()
         + parts[3].getIntValue() / 1000.0;
}
