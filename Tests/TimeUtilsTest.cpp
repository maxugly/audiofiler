#include <juce_core/juce_core.h>
#include "../Source/TimeUtils.h"

class TimeUtilsTest : public juce::UnitTest
{
public:
    TimeUtilsTest() : juce::UnitTest("TimeUtils Testing") {}

    void runTest() override
    {
        beginTest("formatTime handles basic times");
        expectEquals(TimeUtils::formatTime(0.0), juce::String("00:00:00:000"));
        expectEquals(TimeUtils::formatTime(1.0), juce::String("00:00:01:000"));
        expectEquals(TimeUtils::formatTime(60.0), juce::String("00:01:00:000"));
        expectEquals(TimeUtils::formatTime(3600.0), juce::String("01:00:00:000"));

        beginTest("formatTime handles milliseconds");
        expectEquals(TimeUtils::formatTime(0.5), juce::String("00:00:00:500"));
        expectEquals(TimeUtils::formatTime(1.234), juce::String("00:00:01:234"));

        beginTest("formatTime handles negative input");
        expectEquals(TimeUtils::formatTime(-5.0), juce::String("00:00:00:000"));

        beginTest("formatTime handles complex times");
        // 1h 1m 1s 500ms = 3600 + 60 + 1 + 0.5 = 3661.5
        expectEquals(TimeUtils::formatTime(3661.5), juce::String("01:01:01:500"));
    }
};

static TimeUtilsTest timeUtilsTest;
