#include "TimeUtils.h"

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
