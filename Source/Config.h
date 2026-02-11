#pragma once

/**
 * @file Config.h
 * @brief Centralized configuration for the Sorta application.
 *
 * This file contains a collection of compile-time constants and settings
 * that control the application's appearance, behavior, and default values.
 * By centralizing these parameters, it becomes easier to tweak the application's
 * look and feel, and other core functionalities without modifying the main source code.
 * All settings are contained within the `Config` namespace.
 */
namespace Config {
    //==============================================================================
    // General Application Settings
    //==============================================================================

    /**
     * @def TESTING_MODE
     * @brief A preprocessor definition to enable or disable testing-specific features.
     * When set to 1, it might enable features like automatically loading a default
     * audio file (`t.mp3`) on startup to speed up the development and testing cycle.
     * Set to 0 for production builds.
     */
    #define TESTING_MODE 0 

    /// The margin around the entire window content, in pixels.
    constexpr int windowBorderMargins = 15;

    /// The initial width of the main application window in pixels.
    constexpr int initialWindowWidth = 1200;

    /// The initial height of the main application window in pixels.
    constexpr int initialWindowHeight = 800;

    /// The number of most recent audio files the thumbnail cache can hold.
    constexpr int thumbnailCacheSize = 5;

    /// The resolution of the generated audio thumbnails in pixels.
    constexpr int thumbnailSizePixels = 512;

    /// The amount of time in seconds to skip forward or backward when using keyboard shortcuts (e.g., arrow keys).
    constexpr double keyboardSkipAmountSeconds = 5.0;

    //==============================================================================
    // UI Element Sizing
    //==============================================================================

    /// The height in pixels for buttons.
    constexpr int buttonHeight = 30;
    /// The default width in pixels for most buttons.
    constexpr int buttonWidth = 80;

    /// The width of the main playback time and sample position display.
    constexpr int playbackTextWidth = 260; 
    /// The height of the playback time display, chosen to prevent text from being cut off.
    constexpr int playbackTextHeight = 30; 

    /// The width of the loop start/end time display editors.
    constexpr int loopTextWidth = 165;
    /// The width of the silence threshold percentage editors.
    constexpr int thresholdEditorWidth = 80;

    /// The width of the small 'X' button used to clear loop points.
    constexpr int clearButtonWidth = 25;
    /// The margin used for laying out clear buttons next to other elements.
    constexpr int clearButtonMargin = 25; 

    //==============================================================================
    // UI Element Positioning
    //==============================================================================

    /// The vertical offset from the top for the loop start/end text editors.
    constexpr int loopTextOffsetY = 10; 
    /// The vertical offset from the top for the main playback time display.
    constexpr int playbackTimeTextOffsetY = 25; 

    //==============================================================================
    // Fonts and Text
    //==============================================================================

    /// The default font size for the playback time display.
    constexpr int playbackTextSize = 30;
    /// The font size for text that follows the mouse cursor (e.g., time/amplitude display).
    constexpr int mouseCursorTextSize = 20;

    /// A scaling factor to determine the height of text within a button, relative to button height.
    constexpr float buttonTextHeightScale = 0.45f;         
    /// A larger scaling factor for the play/pause symbols to make them more prominent.
    constexpr float buttonPlayPauseTextHeightScale = 0.7f; 

    //==============================================================================
    // Color Palette - General
    //==============================================================================

    /// The background color of the main application window.
    const juce::Colour mainBackgroundColor = juce::Colours::black;
    /// The primary color for text in the playback time display.
    const juce::Colour playbackTextColor = juce::Colour(0xFF34FA11); // A bright green
    /// The alpha (opacity) for the background of text elements, allowing the waveform to be seen behind them.
    constexpr float playbackTextBackgroundAlpha = 0.7f; 

    //==============================================================================
    // Color Palette - Buttons
    //==============================================================================

    /// The standard background color for an enabled, non-toggled button.
    const juce::Colour buttonBaseColour = juce::Colour(0xff5a5a5a); 
    /// The color of a button when it is in an "on" or "active" toggled state.
    const juce::Colour buttonOnColour = juce::Colour(0xff00bfff);   // Deep Sky Blue
    /// The default color for button text.
    const juce::Colour buttonTextColour = juce::Colour(0xFFFFFFFF); // White
    /// The color of the outline around buttons.
    const juce::Colour buttonOutlineColour = juce::Colour(0xff808080); // Medium Grey Outline

    /// The background color for a disabled (non-interactive) button.
    const juce::Colour disabledButtonBackgroundColour = juce::Colour(0xff2a2a2a); 
    /// The text color for a disabled button, providing low contrast against its background.
    const juce::Colour disabledButtonTextColour = juce::Colour(0xff4a4a4a);     

    /// The background color for the 'Exit' button, indicating a destructive action.
    const juce::Colour exitButtonColor = juce::Colours::darkred;
    /// The color of the small 'X' button used for clearing loop points.
    const juce::Colour clearButtonColor = juce::Colours::red;

    /// The color for loop-related buttons when they are in "placement mode" (i.e., waiting for a mouse click).
    const juce::Colour loopButtonPlacementModeColor = juce::Colour(0xffff1493); // Deep Pink
    /// The color for the main 'Loop' button when the loop is active.
    const juce::Colour loopButtonActiveColor = juce::Colour(0xff0066cc);   // Moderate Blue

    //==============================================================================
    // Color Palette - Text Editors
    //==============================================================================
    
    /// The background color for various text editors, semi-transparent.
    const juce::Colour textEditorBackgroundColour = juce::Colours::grey.withAlpha(Config::playbackTextBackgroundAlpha);
    /// The color for text in an editor when the input causes an error.
    const juce::Colour textEditorErrorColor = juce::Colours::red;
    /// The color for text in an editor when the input is a non-critical warning (e.g., out of range).
    const juce::Colour textEditorWarningColor = juce::Colours::orange;
    /// Alias for the warning color, specifically for values outside a valid numerical range.
    const juce::Colour textEditorOutOfRangeColour = juce::Colours::orange; 
    
    //==============================================================================
    // Color Palette - Waveform & Audio Visuals
    //==============================================================================

    /// The main color of the audio waveform display.
    const juce::Colour waveformColor = juce::Colours::deeppink;
    /// The color of the playback cursor line.
    const juce::Colour playbackCursorColor = juce::Colours::lime;
    /// The color of the translucent region that indicates the current loop.
    const juce::Colour loopRegionColor = juce::Colour(0xff0066cc).withAlpha(0.3f);
    /// The color of the vertical lines that mark the loop start and end points.
    const juce::Colour loopLineColor = juce::Colours::blue;
    /// Hover color for loop marker handles.
    const juce::Colour loopMarkerHoverColor = juce::Colours::teal;
    /// Drag color for loop marker handles.
    const juce::Colour loopMarkerDragColor = juce::Colours::green;

    //==============================================================================
    // Color Palette - Mouse Cursor Indicators
    //==============================================================================

    /// The color of the vertical line that follows the mouse cursor over the waveform.
    const juce::Colour mouseCursorLineColor = juce::Colours::yellow;
    /// The color of the highlight box that shows the time-domain region of the cursor.
    const juce::Colour mouseCursorHighlightColor = juce::Colours::darkorange.withAlpha(0.4f);
    /// The color of the horizontal line indicating the amplitude at the mouse cursor's position.
    const juce::Colour mouseAmplitudeLineColor = juce::Colours::orange.brighter(0.5f);
    /// The color of the cursor when in loop point "placement mode".
    const juce::Colour placementModeCursorColor = juce::Colours::deeppink; // Pink lines

    //==============================================================================
    // Color Palette - Silence Threshold Visuals
    //==============================================================================

    /// The color of the horizontal lines that visualize the silence detection thresholds.
    const juce::Colour thresholdLineColor = juce::Colour(0xffe600e6); // Reddish purple
    /// The color of the translucent region below the threshold lines.
    const juce::Colour thresholdRegionColor = juce::Colours::red.withAlpha(0.15f); 

    //==============================================================================
    // Color Palette - Stats & Info Display
    //==============================================================================
    
    /// The background color for the statistics and information overlay.
    const juce::Colour statsDisplayBackgroundColour = juce::Colours::black.withAlpha(0.5f);
    /// The default text color for the statistics display.
    const juce::Colour statsDisplayTextColour = juce::Colours::white;
    /// The color for error messages shown in the statistics display.
    const juce::Colour errorTextColour = juce::Colours::red;

    /// The initial height of the statistics overlay in pixels.
    constexpr int initialStatsDisplayHeight = 150;
    /// Minimum allowed height for the stats overlay.
    constexpr int statsMinHeight = 50;
    /// Maximum allowed height for the stats overlay.
    constexpr int statsMaxHeight = 600;
    
    /// Corner radius for the stats overlay background.
    constexpr float statsCornerRadius = 4.0f;
    /// Height of the area reserved for the resize handle at the bottom.
    constexpr int statsHandleAreaHeight = 12;
    /// Width of the visual resize handle lines.
    constexpr int statsHandleWidth = 40;
    /// Height/thickness of the visual resize handle lines.
    constexpr int statsHandleLineHeight = 2;
    /// Opacity of the resize handle.
    constexpr float statsHandleAlpha = 0.3f;
    
    /// Padding between the overlay border and the text editor.
    constexpr int statsInternalPadding = 2;
    /// Horizontal margin from the content area edges.
    constexpr int statsOverlaySideMargin = 10;
    /// Vertical offset from the top of the content area.
    constexpr int statsOverlayTopMargin = 10;

    // --- Loop Adjustment Steps ---
    constexpr double loopStepHours = 3600.0;
    constexpr double loopStepMinutes = 60.0;
    constexpr double loopStepSeconds = 1.0;
    constexpr double loopStepMilliseconds = 0.01;
    constexpr double loopStepMillisecondsFine = 0.001;

    //==============================================================================
    // Animation, Glow, and Style Settings
    //==============================================================================

    /// The corner radius for buttons, giving them a rounded look.
    constexpr float buttonCornerRadius = 5.0f;
    /// The thickness of the outline around buttons.
    constexpr float buttonOutlineThickness = 1.0f;
    /// A brightness multiplier applied to buttons on mouse hover for interactive feedback.
    constexpr float buttonHighlightedBrightnessFactor = 0.1f; 
    /// A darkness multiplier applied to buttons when they are clicked.
    constexpr float buttonPressedDarknessFactor = 0.1f;   

    /// Thickness of the outline drawn around text editors.
    constexpr int textEditorOutlineThickness = 1;

    // --- Waveform Rendering ---
    constexpr int pixelsPerSampleLow = 4;
    constexpr int pixelsPerSampleMedium = 2;
    constexpr float waveformHeightScale = 0.5f;

    // --- Glow and Overlays ---
    constexpr float glowOffsetFactor = 0.5f;
    constexpr int loopHollowHeightDivisor = 3;
    
    // --- Mouse Cursor Overlays ---
    constexpr float mouseGlowAlpha = 0.3f;
    constexpr int mouseGlowPadding = 2;
    constexpr int mouseHighlightOffset = 2;
    constexpr int mouseHighlightSize = 5;
    constexpr float mouseAmplitudeGlowAlpha = 0.7f;
    constexpr int mouseTextOffset = 5;

    // --- Glow Effects ---
    /// The start color for the gradient glow around the playback cursor (fully transparent).
    const juce::Colour playbackCursorGlowColorStart = juce::Colours::lime.withAlpha(0.0f);
    /// The end color for the gradient glow around the playback cursor (semi-transparent).
    const juce::Colour playbackCursorGlowColorEnd = juce::Colours::lime.withAlpha(0.5f);
    /// The color of the glow effect for the horizontal amplitude line following the mouse.
    const juce::Colour mouseAmplitudeGlowColor = juce::Colours::yellow;
    /// The glow color for the cursor when in loop point "placement mode".
    const juce::Colour placementModeGlowColor = juce::Colours::red.withAlpha(0.7f); 

    // --- Line Thickness ---
    /// The thickness of the horizontal amplitude line.
    const float mouseAmplitudeLineThickness = 1.0f;
    /// The thickness of the glow effect around the amplitude line.
    const float mouseAmplitudeGlowThickness = 3.0f;
    /// The thickness of the glow effect for the cursor in placement mode.
    constexpr float placementModeGlowThickness = 3.0f;
    /// The thickness of the glow effect for the horizontal silence threshold lines.
    constexpr float thresholdGlowThickness = 3.0f;       
    /// The thickness of the glow effect for the vertical loop start/end lines.
    constexpr float loopLineGlowThickness = 3.0f;        
    /// Thickness of the main vertical loop marker line (thin part).
    constexpr float loopMarkerWidthThin = 1.0f;
    /// Thickness of the top/bottom caps of the loop marker line.
    constexpr float loopMarkerWidthThick = 10.0f;
    /// Height of the thick top/bottom caps.
    constexpr int loopMarkerCapHeight = 80;
    /// Divisor used to center marker lines.
    constexpr float loopMarkerCenterDivisor = 2.0f;

    // --- Animation Parameters ---
    /// Controls the speed of the pulsing glow animation on the loop lines.
    constexpr float pulseSpeedFactor = 0.002f;           
    /// The minimum alpha multiplier for the pulsing loop lines, ensuring they never fully disappear.
    constexpr float loopPulseAlphaMinFactor = 0.1f;      
    /// The range of the alpha modulation, controlling the intensity of the pulse.
    constexpr float loopPulseAlphaModulationFactor = 0.8f; 
    /// The length in pixels of the horizontal line that shows the audio amplitude at the cursor position.
    const float mouseAmplitudeLineLength = 50.0f;
    /// The width in pixels for the visualization of the silence threshold lines.
    constexpr float thresholdLineWidth = 100.0f;         

    // --- Hazy Wave Box (Not currently used) ---
    const juce::Colour hazyWaveBoxFadeColor = juce::Colours::black.withAlpha(0.0f); // Fully transparent black for fade effect
    const juce::Colour hazyWavyBoxLineColor = juce::Colours::cyan; // A distinct color for the lines
    constexpr float waveBoxHaze = 0.2f; // Represents the proportion of the shaded box's width to fade

    //==============================================================================
    // Button Labels
    //==============================================================================
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
    const juce::String clearButtonText = "X";
    const juce::String detectInButtonText = "Detect In";
    const juce::String detectOutButtonText = "Detect Out";
    const juce::String autoplayButtonText = "[A]utoPlay";
    const juce::String autoCutInButtonText = "[AC In]";
    const juce::String autoCutOutButtonText = "[AC Out]";
    const juce::String cutButtonText = "[Cut]";

    //==============================================================================
    // Audio Processing and Silence Detection
    //==============================================================================

    /// The default amplitude threshold to be considered "silence" when detecting the start of audio.
    /// A value of 0.01 means any sample with an absolute amplitude below 1% is considered silent.
    constexpr float silenceThreshold = 0.01f;

    /// The default amplitude threshold for detecting the end of audio. Can be different from the 'in' threshold.
    constexpr float outSilenceThreshold = 0.01f;

} // namespace Config
