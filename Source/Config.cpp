#include "Config.h"

namespace Config {

    namespace Colors {
        #if !defined(JUCE_HEADLESS)
        // General
        const juce::Colour playbackText = juce::Colour(0xFF34FA11); // Bright green

        // Text Editors
        const juce::Colour textEditorBackground = juce::Colours::grey.withAlpha(Config::Layout::Text::backgroundAlpha);
        const juce::Colour textEditorError = juce::Colours::red;
        const juce::Colour textEditorWarning = juce::Colours::orange;
        const juce::Colour textEditorOutOfRange = juce::Colours::orange;

        // Waveform & Visuals
        const juce::Colour waveform = juce::Colours::deeppink;
        const juce::Colour playbackCursor = juce::Colours::lime;
        const juce::Colour loopRegion = juce::Colour(0xff0066cc).withAlpha(0.3f);
        const juce::Colour loopLine = juce::Colours::blue;
        const juce::Colour loopMarkerAuto = juce::Colour(0xff00bfff);
        const juce::Colour loopMarkerHover = juce::Colours::teal;
        const juce::Colour loopMarkerDrag = juce::Colours::green;

        // Mouse Cursor
        const juce::Colour mouseCursorLine = juce::Colours::yellow;
        const juce::Colour mouseCursorHighlight = juce::Colours::darkorange.withAlpha(0.4f);
        const juce::Colour mouseAmplitudeLine = juce::Colours::orange.brighter(0.5f);
        const juce::Colour mousePlacementMode = juce::Colours::deeppink;

        // Silence Threshold
        const juce::Colour thresholdLine = juce::Colour(0xffe600e6);
        const juce::Colour thresholdRegion = juce::Colours::red.withAlpha(0.15f);

        // Stats Display
        const juce::Colour statsBackground = juce::Colours::black.withAlpha(0.5f);
        const juce::Colour statsText = juce::Colours::white;
        const juce::Colour statsErrorText = juce::Colours::red;

        // Animation/Glow
        const juce::Colour mouseAmplitudeGlow = juce::Colours::yellow;
        const juce::Colour placementModeGlow = juce::Colours::red.withAlpha(0.7f);

        // Zoom Popup
        const juce::Colour zoomPopupBorder = juce::Colours::blue;


        // Additional colors for UI elements found in code
        const juce::Colour zoomPopupTrackingLine = juce::Colours::dodgerblue;
        const juce::Colour zoomPopupPlaybackLine = juce::Colours::lime;
        const juce::Colour zoomPopupZeroLine = juce::Colours::grey.withAlpha(0.3f);
        #endif
    }

    namespace Labels {
        const juce::String openButton = "[D]ir";
        const juce::String playButton = juce::CharPointer_UTF8 ("\xe2\x96\xb6");
        const juce::String stopButton = juce::CharPointer_UTF8 ("\xe2\x8f\xb8");
        const juce::String viewModeClassic = "[V]iew01";
        const juce::String viewModeOverlay = "[V]iew02";
        const juce::String channelViewMono = "[C]han 1";
        const juce::String channelViewStereo = "[C]han 2";
        const juce::String qualityButton = "[Q]ual";
        const juce::String qualityHigh = "[Q]ual H";
        const juce::String qualityMedium = "[Q]ual M";
        const juce::String qualityLow = "[Q]ual L";
        const juce::String exitButton = "[E]xit";
        const juce::String statsButton = "[S]tats";
        const juce::String loopButton = "[L]oop";
        const juce::String loopInButton = "[I]n";
        const juce::String loopOutButton = "[O]ut";
        const juce::String clearButton = "X";
        const juce::String autoplayButton = "[A]utoPlay";
        const juce::String autoCutInButton = "[AC In]";
        const juce::String autoCutOutButton = "[AC Out]";
        const juce::String cutButton = "[Cut]";

        const juce::String silenceThresholdInTooltip = "Threshold to detect start of sound (0.0 - 1.0)";
        const juce::String silenceThresholdOutTooltip = "Threshold to detect end of sound (0.0 - 1.0)";
    }

}

// Refactored definitions
#if !defined(JUCE_HEADLESS)
const juce::Colour Config::Colors::Window::background { juce::Colours::black };
const juce::Colour Config::Colors::Button::base { 0xff5a5a5a };
const juce::Colour Config::Colors::Button::on { 0xff00bfff };
const juce::Colour Config::Colors::Button::text { 0xFFFFFFFF };
const juce::Colour Config::Colors::Button::outline { 0xff808080 };
const juce::Colour Config::Colors::Button::disabledBackground { 0xff2a2a2a };
const juce::Colour Config::Colors::Button::disabledText { 0xff4a4a4a };
const juce::Colour Config::Colors::Button::exit { juce::Colours::darkred };
const juce::Colour Config::Colors::Button::clear { juce::Colours::red };
const juce::Colour Config::Colors::Button::loopPlacement { 0xffff1493 };
const juce::Colour Config::Colors::Button::loopActive { 0xff0066cc };
#endif
