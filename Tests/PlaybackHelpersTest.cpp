#include <juce_core/juce_core.h>
#include "Utils/PlaybackHelpers.h"

class PlaybackHelpersTest : public juce::UnitTest
{
public:
    PlaybackHelpersTest() : juce::UnitTest("PlaybackHelpers Testing") {}

    void runTest() override
    {
        beginTest("constrainPosition handles normal range");
        {
            double in = 10.0;
            double out = 20.0;
            // Inside range
            expectEquals(PlaybackHelpers::constrainPosition(15.0, in, out), 15.0);
            // On boundaries
            expectEquals(PlaybackHelpers::constrainPosition(10.0, in, out), 10.0);
            expectEquals(PlaybackHelpers::constrainPosition(20.0, in, out), 20.0);
            // Outside range
            expectEquals(PlaybackHelpers::constrainPosition(5.0, in, out), 10.0);
            expectEquals(PlaybackHelpers::constrainPosition(25.0, in, out), 20.0);
        }

        beginTest("constrainPosition handles swapped range (in > out)");
        {
            double in = 20.0;
            double out = 10.0;
            // Inside range (10 to 20)
            expectEquals(PlaybackHelpers::constrainPosition(15.0, in, out), 15.0);
            // On boundaries
            expectEquals(PlaybackHelpers::constrainPosition(10.0, in, out), 10.0);
            expectEquals(PlaybackHelpers::constrainPosition(20.0, in, out), 20.0);
            // Outside range
            expectEquals(PlaybackHelpers::constrainPosition(5.0, in, out), 10.0);
            expectEquals(PlaybackHelpers::constrainPosition(25.0, in, out), 20.0);
        }

        beginTest("constrainPosition handles single point range (in == out)");
        {
            double in = 10.0;
            double out = 10.0;
            expectEquals(PlaybackHelpers::constrainPosition(15.0, in, out), 10.0);
            expectEquals(PlaybackHelpers::constrainPosition(5.0, in, out), 10.0);
            expectEquals(PlaybackHelpers::constrainPosition(10.0, in, out), 10.0);
        }
    }
};

static PlaybackHelpersTest playbackHelpersTest;
