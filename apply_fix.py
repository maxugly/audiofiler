import sys

def main():
    file_path = 'Source/SilenceAnalysisWorker.cpp'
    with open(file_path, 'r') as f:
        content = f.read()

    # Fix in detectInSilence
    search_in = """        const int numThisTime = (int) std::min((juce::int64) kChunkSize, lengthInSamples - currentPos);
        reader->read(&buffer, 0, numThisTime, currentPos, true, true);"""

    replace_in = """        const int numThisTime = (int) std::min((juce::int64) kChunkSize, lengthInSamples - currentPos);
        buffer.clear();
        if (!reader->read(&buffer, 0, numThisTime, currentPos, true, true))
        {
            resumeIfNeeded(audioPlayer, wasPlaying);
            return;
        }"""

    # Fix in detectOutSilence
    search_out = """        const int numThisTime = (int) std::min((juce::int64) kChunkSize, currentPos);
        const juce::int64 startSample = currentPos - numThisTime;

        reader->read(&buffer, 0, numThisTime, startSample, true, true);"""

    replace_out = """        const int numThisTime = (int) std::min((juce::int64) kChunkSize, currentPos);
        const juce::int64 startSample = currentPos - numThisTime;

        buffer.clear();
        if (!reader->read(&buffer, 0, numThisTime, startSample, true, true))
        {
            resumeIfNeeded(audioPlayer, wasPlaying);
            return;
        }"""

    if search_in in content:
        content = content.replace(search_in, replace_in)
        print("Applied fix to detectInSilence")
    else:
        print("Could not find search_in pattern")

    if search_out in content:
        content = content.replace(search_out, replace_out)
        print("Applied fix to detectOutSilence")
    else:
        print("Could not find search_out pattern")

    with open(file_path, 'w') as f:
        f.write(content)

if __name__ == "__main__":
    main()
