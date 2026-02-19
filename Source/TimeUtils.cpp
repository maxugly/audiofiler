/**
 * @file TimeUtils.cpp
 * @brief Defines the TimeUtils class.
 * @ingroup Engine
 */

#include "TimeUtils.h"

/**
 * @brief Undocumented method.
 * @param seconds [in] Description for seconds.
 * @return juce::String
 */
juce::String TimeUtils::formatTime(double seconds) {
  if (seconds < 0) seconds = 0;

  
  
  
  
  long long totalMilliseconds = (long long)(seconds * 1000.0 + 0.0001);

  int hours = (int)(totalMilliseconds / 3600000);
  totalMilliseconds %= 3600000;

  int minutes = (int)(totalMilliseconds / 60000);
  totalMilliseconds %= 60000;

  int secs = (int)(totalMilliseconds / 1000);
  int milliseconds = (int)(totalMilliseconds % 1000);

  /**
   * @brief Undocumented method.
   * @param "%02d:%02d:%02d:%03d" [in] Description for "%02d:%02d:%02d:%03d".
   * @param hours [in] Description for hours.
   * @param minutes [in] Description for minutes.
   * @param secs [in] Description for secs.
   * @param milliseconds [in] Description for milliseconds.
   * @return return
   */
  return juce::String::formatted("%02d:%02d:%02d:%03d", hours, minutes, secs, milliseconds);
}

double TimeUtils::parseTime(const juce::String& timeString)
{
    
    juce::String cleanTime = timeString.startsWithChar('-') ? timeString.substring(1) : timeString;

    auto parts = juce::StringArray::fromTokens(cleanTime, ":", "");
    if (parts.size() != 4)
        return -1.0;

    return parts[0].getIntValue() * 3600.0
         + parts[1].getIntValue() * 60.0
         + parts[2].getIntValue()
         + parts[3].getIntValue() / 1000.0;
}
