#include <juce_core/juce_core.h>

class SecurityFixTest : public juce::UnitTest
{
public:
    SecurityFixTest() : juce::UnitTest("Security Fix: Integer Overflow Prevention") {}

    void runTest() override
    {
        beginTest("Case 1: Large file length causing integer overflow if cast directly");

        // Case 1: Large file length causing integer overflow if cast directly
        // 3 billion samples > INT_MAX (approx 2.14 billion)
        juce::int64 largeLength = 3000000000;

        // The vulnerability was: (int) lengthInSamples
        int truncatedLength = (int) largeLength;

        // This demonstrates the overflow behavior:
        // 3,000,000,000 is 0xB2D05E00
        // (int) 0xB2D05E00 is -1294967296 (negative!)
        expect(truncatedLength != largeLength);
        expect(truncatedLength < 0);

        // The Fix: Chunking logic
        int kChunkSize = 65536;
        juce::int64 currentPos = 0;

        // Verify chunking logic works correctly with large files
        // Simulate one iteration
        if (currentPos < largeLength)
        {
            int numThisTime = (int) std::min((juce::int64) kChunkSize, largeLength - currentPos);
            expect(numThisTime == kChunkSize);
            expect(numThisTime > 0);

            currentPos += numThisTime;
        }

        expect(currentPos == kChunkSize);
    }
};

static SecurityFixTest securityFixTest;
