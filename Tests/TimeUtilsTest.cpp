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

        beginTest("formatTime handles milliseconds and truncation");
        expectEquals(TimeUtils::formatTime(0.5), juce::String("00:00:00:500"));
        expectEquals(TimeUtils::formatTime(1.234), juce::String("00:00:01:234"));
        // Truncation check: 0.9999 should become 999ms, not 1s
        expectEquals(TimeUtils::formatTime(0.9999), juce::String("00:00:00:999"));
        // 0.0001 should become 0ms
        expectEquals(TimeUtils::formatTime(0.0001), juce::String("00:00:00:000"));
        // 0.001 should become 1ms
        expectEquals(TimeUtils::formatTime(0.001), juce::String("00:00:00:001"));

        beginTest("formatTime handles negative input");
        expectEquals(TimeUtils::formatTime(-5.0), juce::String("00:00:00:000"));
        expectEquals(TimeUtils::formatTime(-0.001), juce::String("00:00:00:000"));

        beginTest("formatTime handles complex times");
        // 1h 1m 1s 500ms = 3600 + 60 + 1 + 0.5 = 3661.5
        expectEquals(TimeUtils::formatTime(3661.5), juce::String("01:01:01:500"));

        beginTest("formatTime handles boundaries");
        // 59 seconds 999 ms -> 00:00:59:999
        expectEquals(TimeUtils::formatTime(59.999), juce::String("00:00:59:999"));
        // 59 minutes 59 seconds 999 ms -> 00:59:59:999
        // 59 * 60 + 59 + 0.999 = 3540 + 59 + 0.999 = 3599.999
        expectEquals(TimeUtils::formatTime(3599.999), juce::String("00:59:59:999"));

        beginTest("formatTime handles large values");
        // 100 hours -> 100 * 3600 = 360000
        expectEquals(TimeUtils::formatTime(360000.0), juce::String("100:00:00:000"));
        // 25 hours -> 25 * 3600 = 90000
        expectEquals(TimeUtils::formatTime(90000.0), juce::String("25:00:00:000"));
    }
};

static TimeUtilsTest timeUtilsTest;
