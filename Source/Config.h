#ifndef AUDIOFILER_CONFIG_H
#define AUDIOFILER_CONFIG_H

#include <juce_graphics/juce_graphics.h>

/**
 * @file Config.h
 * @brief Centralized configuration for the audiofiler application.
 */
namespace Config {

    #define TESTING_MODE 0

    //==============================================================================
    // Colors
    //==============================================================================
    namespace Colors {
        // General
        extern const juce::Colour background;
        extern const juce::Colour playbackText;

        // Buttons
        extern const juce::Colour buttonBase;
        extern const juce::Colour buttonOn;
        extern const juce::Colour buttonText;
        extern const juce::Colour buttonOutline;
        extern const juce::Colour buttonDisabledBackground;
        extern const juce::Colour buttonDisabledText;
        extern const juce::Colour buttonExit;
        extern const juce::Colour buttonClear;
        extern const juce::Colour buttonLoopPlacement;
        extern const juce::Colour buttonLoopActive;

        // Text Editors
        extern const juce::Colour textEditorBackground;
        extern const juce::Colour textEditorError;
        extern const juce::Colour textEditorWarning;
        extern const juce::Colour textEditorOutOfRange;

        // Waveform & Visuals
        extern const juce::Colour waveform;
        extern const juce::Colour playbackCursor;
        extern const juce::Colour loopRegion;
        extern const juce::Colour loopLine;
        extern const juce::Colour loopMarkerAuto;
        extern const juce::Colour loopMarkerHover;
        extern const juce::Colour loopMarkerDrag;

        // Mouse Cursor
        extern const juce::Colour mouseCursorLine;
        extern const juce::Colour mouseCursorHighlight;
        extern const juce::Colour mouseAmplitudeLine;
        extern const juce::Colour mousePlacementMode;

        // Silence Threshold
        extern const juce::Colour thresholdLine;
        extern const juce::Colour thresholdRegion;

        // Stats Display
        extern const juce::Colour statsBackground;
        extern const juce::Colour statsText;
        extern const juce::Colour statsErrorText;

        // Animation/Glow
        extern const juce::Colour playbackCursorGlowStart;
        extern const juce::Colour playbackCursorGlowEnd;
        extern const juce::Colour mouseAmplitudeGlow;
        extern const juce::Colour placementModeGlow;

        // Zoom Popup
        extern const juce::Colour zoomPopupBorder;
        extern const juce::Colour zoomPopupIndicator;

        // Extra Zoom Colors
        extern const juce::Colour zoomPopupShadowOuter;
        extern const juce::Colour zoomPopupShadowInner;
        extern const juce::Colour zoomPopupTrackingLine;
        extern const juce::Colour zoomPopupPlaybackLine;
        extern const juce::Colour zoomPopupZeroLine;

        // Hazy Box
        extern const juce::Colour hazyBoxFade;
        extern const juce::Colour hazyBoxLine;
    }

    //==============================================================================
    // Layout & Sizing
    //==============================================================================
    namespace Layout {
        // Window
        constexpr int windowBorderMargins = 15;
        constexpr int initialWindowWidth = 1200;
        constexpr int initialWindowHeight = 800;

        // Button
        constexpr int buttonHeight = 30;
        constexpr int buttonWidth = 80;
        constexpr int clearButtonWidth = 25;
        constexpr int clearButtonMargin = 25;
        constexpr float buttonCornerRadius = 5.0f;
        constexpr float buttonOutlineThickness = 1.0f;

        // Text & Editors
        constexpr int loopTextWidth = 165;
        constexpr int thresholdEditorWidth = 80;
        constexpr int loopTextOffsetY = 10;

        namespace Text {
            constexpr int playbackWidth = 520;
            constexpr int playbackHeight = 30;
            constexpr int playbackOffsetY = 25;

            constexpr int playbackSize = 30;
            constexpr int mouseCursorSize = 20;
            constexpr float buttonHeightScale = 0.45f;
            constexpr float buttonPlayPauseHeightScale = 0.7f;

            constexpr float backgroundAlpha = 0.7f;
            constexpr int editorOutlineThickness = 1;
        }

        // Stats Display
        namespace Stats {
            constexpr int initialHeight = 150;
            constexpr int minHeight = 50;
            constexpr int maxHeight = 600;
            constexpr float cornerRadius = 4.0f;
            constexpr int handleAreaHeight = 12;
            constexpr int handleWidth = 40;
            constexpr int handleLineHeight = 2;
            constexpr float handleAlpha = 0.3f;
            constexpr int internalPadding = 2;
            constexpr int sideMargin = 10;
            constexpr int topMargin = 10;
        }

        // Waveform
        namespace Waveform {
            constexpr float heightScale = 0.5f;
            constexpr int pixelsPerSampleLow = 4;
            constexpr int pixelsPerSampleMedium = 2;
        }

        // Glow & Lines
        namespace Glow {
             constexpr float offsetFactor = 0.5f;
             constexpr int loopHollowHeightDivisor = 3;
             constexpr float mouseAlpha = 0.3f;
             constexpr int mousePadding = 2;
             constexpr int mouseHighlightOffset = 2;
             constexpr int mouseHighlightSize = 5;
             constexpr float mouseAmplitudeAlpha = 0.7f;
             constexpr int mouseTextOffset = 5;

             // Thicknesses
             constexpr float mouseAmplitudeLineThickness = 1.0f;
             constexpr float mouseAmplitudeGlowThickness = 3.0f;
             constexpr float placementModeGlowThickness = 3.0f;
             constexpr float thresholdGlowThickness = 3.0f;
             constexpr float loopLineGlowThickness = 3.0f;

             // Loop Markers
             constexpr float loopMarkerWidthThin = 1.0f;
             constexpr float loopBoxOutlineThickness = 1.5f;
             constexpr float loopBoxOutlineThicknessInteracting = 3.0f;
             constexpr float loopMarkerBoxWidth = 30.0f;
             constexpr int loopMarkerBoxHeight = 30;
             constexpr float loopMarkerCenterDivisor = 2.0f;

             // Playback Cursor
             constexpr float cursorGlowRadius = 5.0f;
             constexpr float cursorGlowLineThickness = 2.0f;
        }

        // Zoom Popup
        namespace Zoom {
            constexpr float factor = 10.0f;
            constexpr float popupScale = 0.8f;
            constexpr float borderThickness = 2.0f;
            constexpr float indicatorThickness = 1.0f;
        }
    }

    //==============================================================================
    // Animation
    //==============================================================================
    namespace Animation {
        constexpr float pulseSpeedFactor = 0.002f;
        constexpr float loopPulseAlphaMin = 0.1f;
        constexpr float loopPulseAlphaModulation = 0.8f;

        constexpr float buttonHighlightedBrightness = 0.1f;
        constexpr float buttonPressedDarkness = 0.1f;

        constexpr float mouseAmplitudeLineLength = 50.0f;
        constexpr float thresholdLineWidth = 100.0f;

        constexpr float waveBoxHaze = 0.2f;
    }

    //==============================================================================
    // Audio Settings
    //==============================================================================
    namespace Audio {
         constexpr int thumbnailCacheSize = 5;
         constexpr int thumbnailSizePixels = 512;
         constexpr double keyboardSkipSeconds = 5.0;

         constexpr double loopStepHours = 3600.0;
         constexpr double loopStepMinutes = 60.0;
         constexpr double loopStepSeconds = 1.0;
         constexpr double loopStepMilliseconds = 0.01;
         constexpr double loopStepMillisecondsFine = 0.001;

         constexpr float silenceThresholdIn = 0.01f;
         constexpr float silenceThresholdOut = 0.01f;
         constexpr bool lockHandlesWhenAutoCutActive = false;
    }
    
    //==============================================================================
    // Labels
    //==============================================================================
    namespace Labels {
        extern const juce::String openButton;
        extern const juce::String playButton;
        extern const juce::String stopButton;
        extern const juce::String viewModeClassic;
        extern const juce::String viewModeOverlay;
        extern const juce::String channelViewMono;
        extern const juce::String channelViewStereo;
        extern const juce::String qualityButton;
        extern const juce::String qualityHigh;
        extern const juce::String qualityMedium;
        extern const juce::String qualityLow;
        extern const juce::String exitButton;
        extern const juce::String statsButton;
        extern const juce::String loopButton;
        extern const juce::String loopInButton;
        extern const juce::String loopOutButton;
        extern const juce::String clearButton;
        extern const juce::String detectInButton;
        extern const juce::String detectOutButton;
        extern const juce::String autoplayButton;
        extern const juce::String autoCutInButton;
        extern const juce::String autoCutOutButton;
        extern const juce::String cutButton;
    }

} // namespace Config

#endif // AUDIOFILER_CONFIG_H
