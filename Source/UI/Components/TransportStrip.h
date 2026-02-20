#ifndef AUDIOFILER_TRANSPORTSTRIP_H
#define AUDIOFILER_TRANSPORTSTRIP_H

#if defined(JUCE_HEADLESS)
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif

#include "Core/AudioPlayer.h"
#include "Core/SessionState.h"
#include "Utils/Config.h"

/**
 * @ingroup UI
 * @class TransportStrip
 * @brief A modular component containing the main transport controls.
 * @details Owns Play, Stop, Auto-Play, Repeat, and Cut buttons.
 *          Handles its own layout and callbacks.
 */
class TransportStrip : public juce::Component
{
public:
    TransportStrip(AudioPlayer& player, SessionState& state);
    ~TransportStrip() override = default;

    void resized() override;

    void updatePlayButtonText(bool isPlaying);
    void updateCutModeState(bool isCutModeActive);
    void updateAutoplayState(bool isAutoplayActive);
    void updateRepeatState(bool isRepeating);

    juce::TextButton& getPlayStopButton() { return playStopButton; }
    juce::TextButton& getStopButton()     { return stopButton; }
    juce::TextButton& getAutoplayButton() { return autoplayButton; }
    juce::TextButton& getRepeatButton()   { return repeatButton; }
    juce::TextButton& getCutButton()      { return cutButton; }

private:
    AudioPlayer& audioPlayer;
    SessionState& sessionState;

    juce::TextButton playStopButton;
    juce::TextButton stopButton;
    juce::TextButton autoplayButton;
    juce::TextButton repeatButton;
    juce::TextButton cutButton;

    void initialiseButtons();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportStrip)
};

#endif
