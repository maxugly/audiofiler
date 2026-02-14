#include <juce_core/juce_core.h>

namespace juce {
    const char* juce_compilationDate = "2025-01-01";
    const char* juce_compilationTime = "00:00:00";
}

int main (int argc, char* argv[])
{
    juce::UnitTestRunner runner;
    runner.runAllTests();
    return 0;
}
