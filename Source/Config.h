#ifndef AUDIOFILER_CONFIG_H
#define AUDIOFILER_CONFIG_H

#if !defined(JUCE_HEADLESS)
#include <juce_graphics/juce_graphics.h>
#else
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h> 
#endif

/**
 * @ingroup State
 * @namespace Config
 * @brief Global configuration constants and settings.
 * @details This namespace contains static configuration for colors, layout, animation,
 *          and audio settings. It provides a centralized place to tweak the application's
 *          look and feel and behavior.
 */
namespace Config {

    namespace Colors {
        struct Window {
            static const juce::Colour background;
        };

        struct Button {
            static const juce::Colour base;
            static const juce::Colour on;
            static const juce::Colour text;
            static const juce::Colour outline;
            static const juce::Colour disabledBackground;
            static const juce::Colour disabledText;
            static const juce::Colour exit;
            static const juce::Colour clear;
            static const juce::Colour cutPlacement;
            static const juce::Colour cutActive;
        };
        extern const juce::Colour playbackText;
        extern const juce::Colour textEditorBackground;
        extern const juce::Colour textEditorError;
        extern const juce::Colour textEditorWarning;
        extern const juce::Colour textEditorOutOfRange;
        extern const juce::Colour waveform;
        extern const juce::Colour playbackCursor;
        extern const juce::Colour cutRegion;
        extern const juce::Colour cutLine;
        extern const juce::Colour cutMarkerAuto;
        extern const juce::Colour cutMarkerHover;
        extern const juce::Colour cutMarkerDrag;
        extern const juce::Colour mouseCursorLine;
        extern const juce::Colour mouseCursorHighlight;
        extern const juce::Colour mouseAmplitudeLine;
        extern const juce::Colour mousePlacementMode;
        extern const juce::Colour thresholdLine;
        extern const juce::Colour thresholdRegion;
        extern const juce::Colour statsBackground;
        extern const juce::Colour statsText;
        extern const juce::Colour statsErrorText;
        extern const juce::Colour mouseAmplitudeGlow;
        extern const juce::Colour placementModeGlow;
        extern const juce::Colour zoomPopupBorder;
        extern const juce::Colour zoomPopupTrackingLine;
        extern const juce::Colour zoomPopupPlaybackLine;
        extern const juce::Colour zoomPopupZeroLine;
    }

    struct Layout {
        struct Window {
            static constexpr int width = 1200;
            static constexpr int height = 800;
        };
        static constexpr int windowBorderMargins = 15;
        static constexpr int buttonHeight = 30;
        static constexpr int buttonWidth = 80;
        static constexpr int clearButtonWidth = 25;
        static constexpr float buttonCornerRadius = 5.0f;
        static constexpr float buttonOutlineThickness = 1.0f;
        static constexpr int cutTextWidth = 165;
        static constexpr int thresholdEditorWidth = 80;

        struct Text {
            static constexpr int playbackWidth = 520;
            static constexpr int playbackHeight = 30;
            static constexpr int playbackOffsetY = 25;
            static constexpr int playbackSize = 30;
            static constexpr int mouseCursorSize = 20;
            static constexpr float buttonHeightScale = 0.45f;
            static constexpr float buttonPlayPauseHeightScale = 0.7f;
            static constexpr float backgroundAlpha = 0.7f;
            static constexpr int editorOutlineThickness = 1;
        };

        struct Stats {
            static constexpr int initialHeight = 150;
            static constexpr int minHeight = 50;
            static constexpr int maxHeight = 600;
            static constexpr float cornerRadius = 4.0f;
            static constexpr int handleAreaHeight = 12;
            static constexpr int handleWidth = 40;
            static constexpr int handleLineHeight = 2;
            static constexpr float handleAlpha = 0.3f;
            static constexpr int internalPadding = 2;
            static constexpr int sideMargin = 10;
            static constexpr int topMargin = 10;
        };

        struct Waveform {
            static constexpr float cutRegionFadeProportion = 0.2f;
            static constexpr float heightScale = 0.5f;
            static constexpr int pixelsPerSampleLow = 4;
            static constexpr int pixelsPerSampleMedium = 2;
        };

        struct Glow {
             static constexpr float offsetFactor = 0.5f;
             static constexpr float mouseAlpha = 0.3f;
             static constexpr int mousePadding = 2;
             static constexpr int mouseHighlightOffset = 2;
             static constexpr int mouseHighlightSize = 5;
             static constexpr float mouseAmplitudeAlpha = 0.7f;
             static constexpr int mouseTextOffset = 5;
             static constexpr float thickness = 3.0f;
             static constexpr float mouseAmplitudeGlowThickness = 3.0f;
             static constexpr float placementModeGlowThickness = 3.0f;
             static constexpr float thresholdGlowThickness = 3.0f;
             static constexpr float cutLineGlowThickness = 3.0f;
             static constexpr float cutMarkerWidthThin = 1.0f;
             static constexpr float cutBoxOutlineThickness = 1.5f;
             static constexpr float cutBoxOutlineThicknessInteracting = 3.0f;
             static constexpr float cutMarkerBoxWidth = 30.0f;
             static constexpr int cutMarkerBoxHeight = 30;
             static constexpr float cutMarkerCenterDivisor = 2.0f;
        };

        struct Zoom {
            static constexpr float popupScale = 0.8f;
            static constexpr float borderThickness = 2.0f;
        };
    };

    namespace Animation {
        constexpr float buttonHighlightedBrightness = 0.1f;
        constexpr float buttonPressedDarkness = 0.1f;
        constexpr float mouseAmplitudeLineLength = 50.0f;
        constexpr float thresholdLineWidth = 100.0f;
    }

    namespace Audio {
         constexpr int thumbnailCacheSize = 5;
         constexpr int thumbnailSizePixels = 512;
         constexpr double keyboardSkipSeconds = 5.0;
         constexpr double cutStepHours = 3600.0;
         constexpr double cutStepMinutes = 60.0;
         constexpr double cutStepSeconds = 1.0;
         constexpr double cutStepMilliseconds = 0.01;
         constexpr double cutStepMillisecondsFine = 0.001;
         constexpr int readAheadBufferSize = 32768;
         constexpr float silenceThresholdIn = 0.01f;
         constexpr float silenceThresholdOut = 0.01f;
         constexpr bool lockHandlesWhenAutoCutActive = false;
    }

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
        extern const juce::String repeatButton;
        extern const juce::String cutInButton;
        extern const juce::String cutOutButton;
        extern const juce::String clearButton;
        extern const juce::String autoplayButton;
        extern const juce::String autoCutInButton;
        extern const juce::String autoCutOutButton;
        extern const juce::String cutButton;
    }

} 

#endif
