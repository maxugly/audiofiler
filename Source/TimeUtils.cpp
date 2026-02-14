#include "TimeUtils.h"
#include "ConfigAudio.h"

juce::String TimeUtils::formatTime(double seconds) {
  if (seconds < 0) seconds = 0;

  // Use long long to prevent overflow for large hours and handle precision better
  // Add small epsilon to correct floating point drift (e.g. 3599.999 -> 3599.99899...)
  // We use 0.0001 (0.1ms) which is small enough not to affect valid milliseconds
  // but large enough to fix precision errors.
  long long totalMilliseconds = (long long)(seconds * 1000.0 + 0.0001);

  int hours = (int)(totalMilliseconds / 3600000);
  totalMilliseconds %= 3600000;

  int minutes = (int)(totalMilliseconds / 60000);
  totalMilliseconds %= 60000;

  int secs = (int)(totalMilliseconds / 1000);
  int milliseconds = (int)(totalMilliseconds % 1000);

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

double TimeUtils::getStepSize(int charIndex, bool shift, bool ctrl, bool alt, double sampleRate)
{
    double step = Config::Audio::loopStepMilliseconds;

    if (charIndex >= 0 && charIndex <= 1)      // HH
    {
        step = Config::Audio::loopStepHours;
    }
    else if (charIndex >= 3 && charIndex <= 4) // MM
    {
        step = Config::Audio::loopStepMinutes;
    }
    else if (charIndex >= 6 && charIndex <= 7) // SS
    {
        step = Config::Audio::loopStepSeconds;
    }
    else if (charIndex >= 9) // mmm
    {
        if (ctrl && shift)
        {
            if (sampleRate > 0.0)
                step = 1.0 / sampleRate;
            else
                step = 0.0001;
        }
        else if (shift)
        {
            step = Config::Audio::loopStepMillisecondsFine;
        }
        else
        {
            step = Config::Audio::loopStepMilliseconds;
        }
    }

    if (alt)
        step *= 10.0;

    return step;
}
