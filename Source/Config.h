#pragma once

/*
 * This file contains configurable settings for the Sorta application.
 * It's a central place for tweaking the look and feel and other
 * parameters without having to dig through the source code.
 */

namespace Config{
    // The margin around the entire window content, in pixels.
    constexpr int windowBorderMargins = 15;

    // The initial width of the main window in pixels.
    constexpr int initialWindowWidth = 1200;

    // The initial height of the main window in pixels.
    constexpr int initialWindowHeight = 800;

    // Playback text settings
    const juce::Colour playbackTextColor = juce::Colour(0xFF34FA11); // Changed to const as juce::Colour constructor is not constexpr
    constexpr float playbackTextBackgroundAlpha = 0.7f; // 0.0f (fully transparent) to 1.0f (fully opaque)
    constexpr int playbackTextSize = 30;

    // Playback and Loop text widths
    constexpr int playbackTextWidth = 220;
    constexpr int loopTextWidth = 165;}
