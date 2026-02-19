/**
 * @file Config.cpp
 * @brief Defines the Config class.
 * @ingroup Engine
 */

#include "Config.h"

namespace Config {

    namespace Colors {
        #if !defined(JUCE_HEADLESS)
        const juce::Colour Window::background { juce::Colours::black };
        const juce::Colour Button::base { 0xff5a5a5a };
        const juce::Colour Button::on { 0xff00bfff };
        const juce::Colour Button::text { 0xFFFFFFFF };
        const juce::Colour Button::outline { 0xff808080 };
        const juce::Colour Button::disabledBackground { 0xff2a2a2a };
        const juce::Colour Button::disabledText { 0xff4a4a4a };
        const juce::Colour Button::exit { juce::Colours::darkred };
        const juce::Colour Button::clear { juce::Colours::red };
        const juce::Colour Button::cutPlacement { 0xffff1493 };
        const juce::Colour Button::cutActive { 0xff0066cc };

        const juce::Colour playbackText = juce::Colour(0xFF34FA11);
        const juce::Colour textEditorBackground = juce::Colours::grey.withAlpha(Config::Layout::Text::backgroundAlpha);
        const juce::Colour textEditorError = juce::Colours::red;
        const juce::Colour textEditorWarning = juce::Colours::orange;
        const juce::Colour textEditorOutOfRange = juce::Colours::orange;
        const juce::Colour waveform = juce::Colours::deeppink;
        const juce::Colour playbackCursor = juce::Colours::lime;
        const juce::Colour cutRegion = juce::Colour(0xff0066cc).withAlpha(0.3f);
        const juce::Colour cutLine = juce::Colours::blue;
        const juce::Colour cutMarkerAuto = juce::Colour(0xff00bfff);
        const juce::Colour cutMarkerHover = juce::Colours::teal;
        const juce::Colour cutMarkerDrag = juce::Colours::green;
        const juce::Colour mouseCursorLine = juce::Colours::yellow;
        const juce::Colour mouseCursorHighlight = juce::Colours::darkorange.withAlpha(0.4f);
        const juce::Colour mouseAmplitudeLine = juce::Colours::orange.brighter(0.5f);
        const juce::Colour mousePlacementMode = juce::Colours::deeppink;
        const juce::Colour thresholdLine = juce::Colour(0xffe600e6);
        const juce::Colour thresholdRegion = juce::Colours::red.withAlpha(0.15f);
        const juce::Colour statsBackground = juce::Colours::black.withAlpha(0.5f);
        const juce::Colour statsText = juce::Colours::white;
        const juce::Colour statsErrorText = juce::Colours::red;
        const juce::Colour mouseAmplitudeGlow = juce::Colours::yellow;
        const juce::Colour placementModeGlow = juce::Colours::red.withAlpha(0.7f);
        const juce::Colour zoomPopupBorder = juce::Colours::blue;
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
        const juce::String repeatButton = "[R]epeat";
        const juce::String cutInButton = "[I]n";
        const juce::String cutOutButton = "[O]ut";
        const juce::String clearButton = "X";
        const juce::String autoplayButton = "[A]utoPlay";
        const juce::String autoCutInButton = "[AC In]";
        const juce::String autoCutOutButton = "[AC Out]";
        const juce::String cutButton = "[Cut]";
    }

}
