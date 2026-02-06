#pragma once

/*
 * This file contains configurable settings for the Sorta application.
 * It's a central place for tweaking the look and feel and other
 * parameters without having to dig through the source code.
 */

namespace Config{
    // Define TESTING_MODE to enable test-specific features like auto-loading t.mp3
    #define TESTING_MODE 0 

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
    constexpr int thresholdEditorWidth = 80;

    // --- Playback and Loop Text Editor Settings ---
    constexpr int playbackTextHeight = 20; // Height for all single-line text editors showing playback/loop times
    constexpr int loopTextOffsetY = 10; // Vertical offset for loop text editors in layout
    constexpr int playbackTimeTextOffsetY = 25; // Vertical offset for playback time text display

    // --- Text Editor Colors ---
    const juce::Colour textEditorErrorColor = juce::Colours::red;
    const juce::Colour textEditorWarningColor = juce::Colours::orange;
    const juce::Colour textEditorOutOfRangeColour = juce::Colours::orange; // For values outside valid range
    /// The background colour for various text editors, derived from a grey with playback text alpha.
    const juce::Colour textEditorBackgroundColour = juce::Colours::grey.withAlpha(Config::playbackTextBackgroundAlpha);
    constexpr int textEditorOutlineThickness = 1;

    // --- Clear Button Settings ---
    const juce::String clearButtonText = "X";
    const juce::Colour clearButtonColor = juce::Colours::red;
    constexpr int clearButtonWidth = 25;
    constexpr int clearButtonMargin = 25; // Margin for clear buttons in layout

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

    // --- Button Texts ---
    const juce::String openButtonText = "[D]ir";
    const juce::String playButtonText = juce::CharPointer_UTF8 ("\xe2\x96\xb6"); // Play symbol
    const juce::String stopButtonText = juce::CharPointer_UTF8 ("\xe2\x8f\xb8"); // Stop symbol
    const juce::String viewModeClassicText = "[V]iew01";
    const juce::String viewModeOverlayText = "[V]iew02";
    const juce::String channelViewMonoText = "[C]han 1";
    const juce::String channelViewStereoText = "[C]han 2";
    const juce::String qualityButtonText = "[Q]ual";
    const juce::String qualityHighText = "[Q]ual H";
    const juce::String qualityMediumText = "[Q]ual M";
    const juce::String qualityLowText = "[Q]ual L";
    const juce::String exitButtonText = "[E]xit";
    const juce::String statsButtonText = "[S]tats";
    const juce::String loopButtonText = "[L]oop";
    const juce::String loopInButtonText = "[I]n";
    const juce::String loopOutButtonText = "[O]ut";

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

    // --- Keyboard Shortcuts ---
    constexpr double keyboardSkipAmountSeconds = 5.0; // Amount to skip when using left/right arrow keys

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

    // --- Audio Settings ---
    /// The number of files the thumbnail cache can hold.
    constexpr int thumbnailCacheSize = 5;
    /// The size of the thumbnail in pixels.
    constexpr int thumbnailSizePixels = 512;
} // namespace Config
