#include <juce_core/juce_core.h>
#include "../Source/TimeUtils.h"
#include "../Source/ConfigAudio.h"

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
        expectEquals(TimeUtils::formatTime(0.9999), juce::String("00:00:00:999"));
        expectEquals(TimeUtils::formatTime(0.0001), juce::String("00:00:00:000"));
        expectEquals(TimeUtils::formatTime(0.001), juce::String("00:00:00:001"));

        beginTest("formatTime handles negative input");
        expectEquals(TimeUtils::formatTime(-5.0), juce::String("00:00:00:000"));
        expectEquals(TimeUtils::formatTime(-0.001), juce::String("00:00:00:000"));

        beginTest("formatTime handles complex times");
        expectEquals(TimeUtils::formatTime(3661.5), juce::String("01:01:01:500"));

        beginTest("formatTime handles boundaries");
        expectEquals(TimeUtils::formatTime(59.999), juce::String("00:00:59:999"));
        expectEquals(TimeUtils::formatTime(3599.999), juce::String("00:59:59:999"));

        beginTest("formatTime handles large values");
        expectEquals(TimeUtils::formatTime(360000.0), juce::String("100:00:00:000"));
        expectEquals(TimeUtils::formatTime(90000.0), juce::String("25:00:00:000"));

        runParseTimeTests();
        runGetStepSizeTests();
    }

    void runParseTimeTests()
    {
        beginTest("parseTime handles valid inputs");
        expectEquals(TimeUtils::parseTime("00:00:00:000"), 0.0);
        expectEquals(TimeUtils::parseTime("00:00:01:000"), 1.0);
        expectEquals(TimeUtils::parseTime("01:00:00:000"), 3600.0);
        expectEquals(TimeUtils::parseTime("00:01:00:500"), 60.5);

        beginTest("parseTime handles negative/remaining time strings");
        expectEquals(TimeUtils::parseTime("-00:00:01:000"), 1.0); // Returns absolute value

        beginTest("parseTime handles invalid inputs");
        expectEquals(TimeUtils::parseTime("invalid"), -1.0);
        expectEquals(TimeUtils::parseTime("00:00:00"), -1.0); // Missing mmm
    }

    void runGetStepSizeTests()
    {
        beginTest("getStepSize handles HH/MM/SS");
        // HH
        expectEquals(TimeUtils::getStepSize(0, false, false, false), Config::Audio::loopStepHours);
        expectEquals(TimeUtils::getStepSize(1, true, true, false), Config::Audio::loopStepHours); // Modifiers ignored for HH

        // MM
        expectEquals(TimeUtils::getStepSize(3, false, false, false), Config::Audio::loopStepMinutes);

        // SS
        expectEquals(TimeUtils::getStepSize(6, false, false, false), Config::Audio::loopStepSeconds);

        beginTest("getStepSize handles mmm (milliseconds)");
        // Default
        expectEquals(TimeUtils::getStepSize(9, false, false, false), Config::Audio::loopStepMilliseconds);
        expectEquals(TimeUtils::getStepSize(11, false, false, false), Config::Audio::loopStepMilliseconds);

        // Shift (Fine)
        expectEquals(TimeUtils::getStepSize(9, true, false, false), Config::Audio::loopStepMillisecondsFine);

        // Shift + Ctrl (Super Fine / Sample Rate)
        // No sample rate provided (default 0.0) -> 0.0001
        expectEquals(TimeUtils::getStepSize(9, true, true, false, 0.0), 0.0001);

        // Sample rate provided
        double sampleRate = 44100.0;
        double expected = 1.0 / sampleRate;
        expectEquals(TimeUtils::getStepSize(9, true, true, false, sampleRate), expected);

        beginTest("getStepSize handles Alt multiplier");
        // Alt * 10
        expectEquals(TimeUtils::getStepSize(9, false, false, true), Config::Audio::loopStepMilliseconds * 10.0);

        // Alt + Shift + Ctrl
        expectEquals(TimeUtils::getStepSize(9, true, true, true, 0.0), 0.0001 * 10.0);
    }
};

static TimeUtilsTest timeUtilsTest;
