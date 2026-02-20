#include <juce_core/juce_core.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "Workers/SilenceAnalysisAlgorithms.h"

// Mock Reader that simulates a large file (e.g. 3 billion samples)
class LargeFileMockReader : public juce::AudioFormatReader
{
public:
    LargeFileMockReader(juce::int64 length, int channels, double rate)
        : juce::AudioFormatReader(nullptr, "MockReader")
    {
        lengthInSamples = length;
        numChannels = channels;
        sampleRate = rate;
        bitsPerSample = 32;
        usesFloatingPointData = true;
    }

    bool readSamples(int* const* destSamples, int numDestChannels, int startOffsetInDestBuffer,
                     juce::int64 startSampleInFile, int numSamples) override
    {
        // Simulate silence everywhere except at specific points
        for (int ch = 0; ch < numDestChannels; ++ch)
        {
            if (destSamples[ch] != nullptr)
            {
                // Fill with silence
                juce::FloatVectorOperations::clear((float*)destSamples[ch] + startOffsetInDestBuffer, numSamples);

                // Inject signal at specific location (e.g. 2.5 billion)
                juce::int64 signalPos = 2500000000;

                // For a chunk, check if signalPos falls within [startSampleInFile, startSampleInFile + numSamples)
                if (signalPos >= startSampleInFile && signalPos < startSampleInFile + numSamples)
                {
                    int offset = (int)(signalPos - startSampleInFile);
                    float* buffer = (float*)destSamples[ch] + startOffsetInDestBuffer;
                    buffer[offset] = 1.0f; // Full volume signal
                }
            }
        }
        return true;
    }
};

class SilenceAnalysisTest : public juce::UnitTest
{
public:
    SilenceAnalysisTest() : juce::UnitTest("Silence Analysis Large File Test") {}

    void runTest() override
    {
        beginTest("Find Silence In - Large File");

        // 3 billion samples > INT_MAX
        juce::int64 largeLength = 3000000000;
        LargeFileMockReader reader(largeLength, 2, 44100.0);

        // Threshold 0.1
        // Should find signal at 2,500,000,000
        juce::int64 foundPos = SilenceAnalysisAlgorithms::findSilenceIn(reader, 0.1f);

        expect(foundPos == 2500000000);

        beginTest("Find Silence Out - Large File");
        // findSilenceOut scans backwards.
        // It looks for the LAST sample > threshold.
        // In our mock, only 2,500,000,000 has signal.
        // So it should return 2,500,000,000.

        juce::int64 foundEnd = SilenceAnalysisAlgorithms::findSilenceOut(reader, 0.1f);
        expect(foundEnd == 2500000000);

        beginTest("Find Silence In - No Signal");
        LargeFileMockReader shortReader(1000, 2, 44100.0);
        // Short reader with same signal logic: signalPos is 2.5B, so it's outside range.
        // Should return -1.
        expect(SilenceAnalysisAlgorithms::findSilenceIn(shortReader, 0.1f) == -1);
    }
};

static SilenceAnalysisTest silenceAnalysisTest;
