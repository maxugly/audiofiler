

#pragma once

#include <juce_core/juce_core.h>

struct FileMetadata {
    double cutIn { 0.0 };
    double cutOut { 0.0 };
    bool isAnalyzed { false };
    juce::String hash;
};
