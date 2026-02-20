#ifndef AUDIOFILER_MARKERSTRIP_H
#define AUDIOFILER_MARKERSTRIP_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

#include "Core/AudioPlayer.h"
#include "Core/SessionState.h"
#include "Workers/SilenceDetector.h"
#include "UI/Components/TransportButton.h"
#include "Presenters/RepeatPresenter.h"
#include "Utils/Config.h"

/**
 * @ingroup UI
 * @class MarkerStrip
 * @brief A modular component for controlling a cut point (In or Out).
 * @details Follows the Symmetry Rule:
 *          In Strip: [In(L), Timer, Reset, Threshold, AutoCut(R)]
 *          Out Strip: [AutoCut(L), Threshold, Reset, Timer, Out(R)]
 */
class MarkerStrip : public juce::Component
{
public:
    enum class MarkerType { In, Out };

    MarkerStrip(MarkerType type, AudioPlayer& player, SessionState& state, SilenceDetector& detector);
    ~MarkerStrip() override = default;

    void resized() override;

    void updateTimerText(const juce::String& text);
    void updateAutoCutState(bool isActive);
    void updateMarkerButtonColor(juce::Colour color);

    void setPresenter(RepeatPresenter* p) { repeatPresenter = p; }

    std::function<void()> onMarkerRightClick;

    TransportButton& getMarkerButton() { return markerButton; }
    juce::TextEditor& getTimerEditor()  { return timerEditor; }
    juce::TextButton& getResetButton()  { return resetButton; }
    juce::TextEditor& getThresholdEditor() { return thresholdEditor; }
    juce::TextButton& getAutoCutButton() { return autoCutButton; }

private:
    MarkerType markerType;
    AudioPlayer& audioPlayer;
    SessionState& sessionState;
    SilenceDetector& silenceDetector;
    RepeatPresenter* repeatPresenter = nullptr;

    TransportButton markerButton;
    juce::TextEditor timerEditor;
    juce::TextButton resetButton;
    juce::TextEditor thresholdEditor;
    juce::TextButton autoCutButton;

    void initialiseComponents();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MarkerStrip)
};

#endif
