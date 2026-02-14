#include <juce_core/juce_core.h>

int main (int argc, char* argv[])
{
    juce::UnitTestRunner runner;
    runner.runAllTests();
    return 0;
}

namespace juce {
    extern const char* const juce_compilationDate = "2024-01-01";
    extern const char* const juce_compilationTime = "00:00:00";
}
