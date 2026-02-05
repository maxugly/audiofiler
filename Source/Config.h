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
    constexpr int mouseCursorTextSize = 20;

    // Playback and Loop text widths
    constexpr int playbackTextWidth = 220;
    constexpr int loopTextWidth = 165;

    // --- Playback and Loop Text Editor Settings ---
    constexpr int playbackTextHeight = 20; // Height for all single-line text editors showing playback/loop times

    // --- Clear Button Settings ---
    const juce::String clearButtonText = "X";
    const juce::Colour clearButtonColor = juce::Colours::red;
    constexpr int clearButtonWidth = 25;

    // --- Button Settings ---
    constexpr int buttonWidth = 80;
    constexpr int buttonHeight = 30; // Will be used in resized()
    constexpr float buttonCornerRadius = 5.0f;
    constexpr float buttonOutlineThickness = 1.0f;
    const juce::Colour buttonOutlineColour = juce::Colour(0xff808080); // Medium Grey Outline

    const juce::Colour buttonBaseColour = juce::Colour(0xff3a3a3a); // Dark Grey
    const juce::Colour buttonOnColour = juce::Colour(0xff00bfff);   // Deep Sky Blue (for toggled/highlighted state)
    const juce::Colour buttonTextColour = juce::Colour(0xFFFFFFFF); // White
    // New colors for disabled state
    const juce::Colour disabledButtonBackgroundColour = juce::Colour(0xff2a2a2a); // A slightly darker grey for disabled background
    const juce::Colour disabledButtonTextColour = juce::Colour(0xff4a4a4a);     // Darker grey for disabled text, closer to background
    constexpr float buttonHighlightedBrightnessFactor = 0.1f; // How much brighter on hover
    constexpr float buttonPressedDarknessFactor = 0.1f;   // How much darker on press

    constexpr float buttonTextHeightScale = 0.45f;         // Default scale for button text
    constexpr float buttonPlayPauseTextHeightScale = 0.7f; // Scale for Play/Pause symbols

    // --- General Colors ---
    const juce::Colour mainBackgroundColor = juce::Colours::black;

    // --- Exit Button Colors ---
    const juce::Colour exitButtonColor = juce::Colours::darkred;

    // --- Stats Display Colors ---
    const juce::Colour statsDisplayBackgroundColour = juce::Colours::black.withAlpha(0.5f);
    const juce::Colour statsDisplayTextColour = juce::Colours::white;
    const juce::Colour errorTextColour = juce::Colours::red;

    // --- Loop Button Specific Colors ---
    const juce::Colour loopButtonPlacementModeColor = juce::Colour(0xffff1493); // Deep Pink
    const juce::Colour loopButtonActiveColor = juce::Colour(0xff0066cc);   // Moderate Blue

    // --- Waveform Colors ---
    const juce::Colour waveformColor = juce::Colours::deeppink;

    // --- Loop Region Colors ---
    const juce::Colour loopRegionColor = juce::Colour(0xff0066cc).withAlpha(0.3f);
    const juce::Colour loopLineColor = juce::Colours::blue;

    // --- Silence Detection ---
    constexpr float silenceThreshold = 0.01f;
    constexpr float outSilenceThreshold = 0.01f; // Default for outbound silence detection
    const juce::String detectInButtonText = "Detect In";
    const juce::String detectOutButtonText = "Detect Out";
    const juce::String autoplayButtonText = "[A]utoPlay";
    const juce::String autoCutInButtonText = "[AC In]";
    const juce::String autoCutOutButtonText = "[AC Out]";
    const juce::String cutButtonText = "[Cut]";

    // --- Playback Cursor Colors ---
    const juce::Colour playbackCursorColor = juce::Colours::lime;
    // Playback cursor glow colors (start and end alpha for gradient)
    const juce::Colour playbackCursorGlowColorStart = juce::Colours::lime.withAlpha(0.0f);
    const juce::Colour playbackCursorGlowColorEnd = juce::Colours::lime.withAlpha(0.5f);

    // --- Mouse Cursor Colors ---
    const juce::Colour mouseCursorHighlightColor = juce::Colours::darkorange.withAlpha(0.4f);
    const juce::Colour mouseCursorLineColor = juce::Colours::yellow;
    const juce::Colour mouseAmplitudeLineColor = juce::Colours::orange.brighter(0.5f);
    const juce::Colour mouseAmplitudeGlowColor = juce::Colours::yellow;
    const float mouseAmplitudeLineThickness = 1.0f;
    const float mouseAmplitudeGlowThickness = 3.0f;
    const float mouseAmplitudeLineLength = 50.0f;

    // --- Silence Threshold Visualization Colors ---
    const juce::Colour thresholdLineColor = juce::Colour(0xffe600e6); // Reddish purple
    const juce::Colour thresholdRegionColor = juce::Colours::red.withAlpha(0.15f); // A shade of red, translucent

    // --- Inactive Visual Element Dimming Factors ---


    // --- Animation & Glow Settings ---
    constexpr float pulseSpeedFactor = 0.002f;           // Speed of the glowing pulse animation
    constexpr float thresholdGlowThickness = 3.0f;       // Thickness of the glowing horizontal threshold lines
    constexpr float thresholdLineWidth = 100.0f;         // Width of the threshold visualization lines in pixels
    constexpr float loopLineGlowThickness = 3.0f;        // Thickness of the glowing vertical loop lines
    constexpr float loopPulseAlphaMinFactor = 0.1f;      // Minimum alpha multiplier for pulsing vertical loop lines
    constexpr float loopPulseAlphaModulationFactor = 0.8f; // Factor controlling the range of alpha modulation for vertical loop lines
} // namespace Config
