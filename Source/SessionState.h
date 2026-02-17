/*
  ==============================================================================

    SessionState.h
    Created: 17 Feb 2026
    Author:  Jules

  ==============================================================================
*/

#pragma once

struct SessionState
{
    bool autoplay = false;
    bool cutModeActive = false;
    bool autoCutIn = true;
    bool autoCutOut = true;

    float thresholdIn = 0.15f;
    float thresholdOut = 0.15f;

    double getEffectiveDuration(double rawTotal, double start, double end) const
    {
        return cutModeActive ? juce::jmax(0.0, end - start) : rawTotal;
    }
};
