#include <juce_core/juce_core.h>
#include "Utils/TimeUtils.h"

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

        beginTest("formatTime handles precision edge cases");
        // 0.99999 -> 00:00:00:999 (should not round up to 1s with current epsilon)
        expectEquals(TimeUtils::formatTime(0.99999), juce::String("00:00:00:999"));

        beginTest("formatTime handles negative zero");
        expectEquals(TimeUtils::formatTime(-0.0), juce::String("00:00:00:000"));

        beginTest("formatTime handles extremely large values");
        // 2500 hours = 9,000,000 seconds -> 9,000,000,000 ms (fits in long long, exceeds int)
        // 9000000 / 3600 = 2500
        expectEquals(TimeUtils::formatTime(9000000.0), juce::String("2500:00:00:000"));

        // ==========================================
        // parseTime tests
        // ==========================================
        beginTest("parseTime handles valid inputs");
        expectEquals(TimeUtils::parseTime("00:00:00:000"), 0.0);
        expectEquals(TimeUtils::parseTime("01:00:00:000"), 3600.0);
        expectEquals(TimeUtils::parseTime("00:01:00:000"), 60.0);
        expectEquals(TimeUtils::parseTime("00:00:01:000"), 1.0);
        expectEquals(TimeUtils::parseTime("00:00:00:500"), 0.5);

        beginTest("parseTime handles complex inputs");
        expectEquals(TimeUtils::parseTime("01:01:01:500"), 3661.5);

        beginTest("parseTime handles negative string inputs (strips minus)");
        expectEquals(TimeUtils::parseTime("-00:00:01:000"), 1.0);
        expectEquals(TimeUtils::parseTime("-01:00:00:000"), 3600.0);

        beginTest("parseTime handles invalid inputs");
        expectEquals(TimeUtils::parseTime("invalid"), -1.0);
        expectEquals(TimeUtils::parseTime("00:00"), -1.0); // Too few parts
        expectEquals(TimeUtils::parseTime(""), -1.0);
        expectEquals(TimeUtils::parseTime("00:00:00:000:000"), -1.0); // Too many parts

        beginTest("Round trip consistency");
        // Check a range of times
        double testTimes[] = { 0.0, 0.5, 1.0, 60.0, 3600.0, 3661.5, 9999.999 };
        for (double t : testTimes)
        {
            expectWithinAbsoluteError(TimeUtils::parseTime(TimeUtils::formatTime(t)), t, 0.001);
        }
    }
};

static TimeUtilsTest timeUtilsTest;
