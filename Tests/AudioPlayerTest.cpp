#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "../Source/AudioPlayer.h"

// Simple Mock Source
class MockAudioSource : public juce::PositionableAudioSource
{
public:
    MockAudioSource() { lengthInSamples = 44100 * 60; } // 1 minute

    void setNextReadPosition (juce::int64 newPosition) override { position = newPosition; }
    juce::int64 getNextReadPosition() const override { return position; }
    juce::int64 getTotalLength() const override { return lengthInSamples; }
    bool isLooping() const override { return false; }
    void setLooping (bool) override {}

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        currentSampleRate = sampleRate;
    }
    void releaseResources() override {}
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override {}

    double currentSampleRate = 44100.0;
    juce::int64 lengthInSamples = 0;
    juce::int64 position = 0;
};

class AudioPlayerTest : public juce::UnitTest
{
public:
    AudioPlayerTest() : juce::UnitTest("AudioPlayer Testing") {}

    void runTest() override
    {
        beginTest("setPositionConstrained constrains position correctly");
        {
            // Create AudioPlayer
            AudioPlayer player;

            // Setup Mock Source
            MockAudioSource mockSource;

            // We need to initialize transport source with a source
            player.getTransportSource().setSource(&mockSource, 0, nullptr, 44100.0);
            player.getTransportSource().prepareToPlay(512, 44100.0);

            // Verify initial state
            expectEquals(player.getTransportSource().getCurrentPosition(), 0.0);

            // Mock loop points
            double loopIn = 2.0;
            double loopOut = 8.0;

            // Test case 1: Position within range
            player.setPositionConstrained(5.0, loopIn, loopOut);
            expectEquals(player.getTransportSource().getCurrentPosition(), 5.0);

            // Test case 2: Position outside range (below)
            player.setPositionConstrained(1.0, loopIn, loopOut);
            expectEquals(player.getTransportSource().getCurrentPosition(), loopIn);

            // Test case 3: Position outside range (above)
            player.setPositionConstrained(9.0, loopIn, loopOut);
            expectEquals(player.getTransportSource().getCurrentPosition(), loopOut);

            // Test case 4: Swapped loop points
            player.setPositionConstrained(5.0, loopOut, loopIn);
            expectEquals(player.getTransportSource().getCurrentPosition(), 5.0);

            player.setPositionConstrained(1.0, loopOut, loopIn);
            expectEquals(player.getTransportSource().getCurrentPosition(), loopIn); // Expect min(loopIn, loopOut) which is 2.0

            // Cleanup
            player.getTransportSource().setSource(nullptr);
        }
    }
};

static AudioPlayerTest audioPlayerTest;
