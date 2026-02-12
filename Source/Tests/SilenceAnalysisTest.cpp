#include <catch2/catch_test_macros.hpp>
#include <juce_audio_basics/juce_audio_basics.h>
#include "SilenceAlgorithms.h"

TEST_CASE("Silence Analysis - Find Start", "[SilenceAnalysis]") {
    // Create a stereo buffer with 1000 samples
    juce::AudioBuffer<float> buffer(2, 1000);
    buffer.clear();

    SECTION("Finds start on channel 0") {
        buffer.setSample(0, 100, 0.5f); // Sample 100, Channel 0 exceeds threshold
        int result = SilenceAlgorithms::findSilenceStart(buffer, 0.1f);
        REQUIRE(result == 100);
    }

    SECTION("Finds start on channel 1") {
        buffer.setSample(1, 200, 0.5f); // Sample 200, Channel 1 exceeds threshold
        int result = SilenceAlgorithms::findSilenceStart(buffer, 0.1f);
        REQUIRE(result == 200);
    }

    SECTION("Respects threshold") {
        buffer.setSample(0, 50, 0.05f); // Below threshold of 0.1
        buffer.setSample(0, 150, 0.5f); // Above threshold
        int result = SilenceAlgorithms::findSilenceStart(buffer, 0.1f);
        REQUIRE(result == 150);
    }

    SECTION("Returns -1 for silence") {
        int result = SilenceAlgorithms::findSilenceStart(buffer, 0.1f);
        REQUIRE(result == -1);
    }
}

TEST_CASE("Silence Analysis - Find End", "[SilenceAnalysis]") {
    juce::AudioBuffer<float> buffer(2, 1000);
    buffer.clear();

    SECTION("Finds end on channel 0") {
        buffer.setSample(0, 800, 0.5f);
        int result = SilenceAlgorithms::findSilenceEnd(buffer, 0.1f);
        REQUIRE(result == 800);
    }

    SECTION("Finds end on channel 1") {
        buffer.setSample(1, 900, 0.5f);
        int result = SilenceAlgorithms::findSilenceEnd(buffer, 0.1f);
        REQUIRE(result == 900);
    }

    SECTION("Returns -1 for silence") {
        int result = SilenceAlgorithms::findSilenceEnd(buffer, 0.1f);
        REQUIRE(result == -1);
    }
}
