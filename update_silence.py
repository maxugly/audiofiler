import sys

with open('Source/SilenceAnalysisWorker.cpp', 'r') as f:
    content = f.read()

# Include <new>
content = content.replace('#include <limits>', '#include <limits>\n#include <new>')

# define the block to replace
old_block = """    if (lengthInSamples == 0)
    {
        SilenceDetectionLogger::logZeroLength(ownerPanel);
        resumeIfNeeded(audioPlayer, wasPlaying);
        return;
    }

    if (lengthInSamples > std::numeric_limits<int>::max())
    {
        SilenceDetectionLogger::logAudioTooLarge(ownerPanel);
        resumeIfNeeded(audioPlayer, wasPlaying);
        return;
    }

    auto buffer = std::make_unique<juce::AudioBuffer<float>>(reader->numChannels, (int) lengthInSamples);
    reader->read(buffer.get(), 0, (int) lengthInSamples, 0, true, true);"""

new_block = """    if (lengthInSamples <= 0)
    {
        SilenceDetectionLogger::logZeroLength(ownerPanel);
        resumeIfNeeded(audioPlayer, wasPlaying);
        return;
    }

    if (lengthInSamples > std::numeric_limits<int>::max())
    {
        SilenceDetectionLogger::logAudioTooLarge(ownerPanel);
        resumeIfNeeded(audioPlayer, wasPlaying);
        return;
    }

    std::unique_ptr<juce::AudioBuffer<float>> buffer;
    try
    {
        buffer = std::make_unique<juce::AudioBuffer<float>>(reader->numChannels, (int) lengthInSamples);
    }
    catch (const std::bad_alloc&)
    {
        SilenceDetectionLogger::logAudioTooLarge(ownerPanel);
        resumeIfNeeded(audioPlayer, wasPlaying);
        return;
    }

    reader->read(buffer.get(), 0, (int) lengthInSamples, 0, true, true);"""

# Replace all occurrences (should be 2)
new_content = content.replace(old_block, new_block)

if content == new_content:
    print("No changes made! Check the search block.")
    sys.exit(1)

with open('Source/SilenceAnalysisWorker.cpp', 'w') as f:
    f.write(new_content)

print("Successfully updated Source/SilenceAnalysisWorker.cpp")
