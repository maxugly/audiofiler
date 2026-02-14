#include <juce_core/juce_core.h>
#include "../PlaybackHelpers.h"
#include <iostream>

namespace juce {
    const char* juce_compilationDate = __DATE__;
    const char* juce_compilationTime = __TIME__;
}

class PlaybackHelpersTest : public juce::UnitTest
{
public:
    PlaybackHelpersTest() : juce::UnitTest("PlaybackHelpers Testing") {}

    void runTest() override
    {
        beginTest("constrainPosition");

        // Test Case 1: Standard Bounds
        expectEquals(PlaybackHelpers::constrainPosition(5.0, 2.0, 8.0), 5.0, "Standard Position");

        // Test Case 2: Clamping Low
        expectEquals(PlaybackHelpers::constrainPosition(1.0, 2.0, 8.0), 2.0, "Clamped Low");

        // Test Case 3: Clamping High
        expectEquals(PlaybackHelpers::constrainPosition(9.0, 2.0, 8.0), 8.0, "Clamped High");

        // Test Case 4: Swapped Bounds (loopIn > loopOut)
        expectEquals(PlaybackHelpers::constrainPosition(5.0, 8.0, 2.0), 5.0, "Swapped Bounds");

        // Test Case 5: Swapped Clamping (loopIn > loopOut)
        expectEquals(PlaybackHelpers::constrainPosition(1.0, 8.0, 2.0), 2.0, "Swapped Clamping Low");
    }
};

int main()
{
    PlaybackHelpersTest test;
    juce::UnitTestRunner runner;
    runner.runAllTests();

    for (int i = 0; i < runner.getNumResults(); ++i) {
        const auto* result = runner.getResult(i);
        if (result->failures > 0) {
            std::cerr << "Test Failed: " << result->unitTestName << std::endl;
            for (const auto& msg : result->messages) {
                std::cerr << "  " << msg << std::endl;
            }
            return 1;
        }
    }

    std::cout << "All tests passed!" << std::endl;
    return 0;
}
