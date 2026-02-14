#ifndef AUDIOFILER_CONTROLPANELCOPY_H
#define AUDIOFILER_CONTROLPANELCOPY_H

#include <JuceHeader.h>
#include "Config.h"

namespace ControlPanelCopy
{
inline const juce::String& openButtonText() { return Config::Labels::openButton; }
inline const juce::String& playButtonText() { return Config::Labels::playButton; }
inline const juce::String& stopButtonText() { return Config::Labels::stopButton; }
inline const juce::String& viewModeClassicText() { return Config::Labels::viewModeClassic; }
inline const juce::String& viewModeOverlayText() { return Config::Labels::viewModeOverlay; }
inline const juce::String& channelViewMonoText() { return Config::Labels::channelViewMono; }
inline const juce::String& channelViewStereoText() { return Config::Labels::channelViewStereo; }
inline const juce::String& qualityButtonText() { return Config::Labels::qualityButton; }
inline const juce::String& qualityHighText() { return Config::Labels::qualityHigh; }
inline const juce::String& qualityMediumText() { return Config::Labels::qualityMedium; }
inline const juce::String& qualityLowText() { return Config::Labels::qualityLow; }
inline const juce::String& exitButtonText() { return Config::Labels::exitButton; }
inline const juce::String& statsButtonText() { return Config::Labels::statsButton; }
inline const juce::String& loopButtonText() { return Config::Labels::loopButton; }
inline const juce::String& autoplayButtonText() { return Config::Labels::autoplayButton; }
inline const juce::String& autoCutInButtonText() { return Config::Labels::autoCutInButton; }
inline const juce::String& autoCutOutButtonText() { return Config::Labels::autoCutOutButton; }
inline const juce::String& cutButtonText() { return Config::Labels::cutButton; }
inline const juce::String& loopInButtonText() { return Config::Labels::loopInButton; }
inline const juce::String& loopOutButtonText() { return Config::Labels::loopOutButton; }
inline const juce::String& clearButtonText() { return Config::Labels::clearButton; }

inline const juce::String& silenceThresholdInTooltip()
{
    static const juce::String text("Threshold to detect start of sound (0.0 - 1.0)");
    return text;
}

inline const juce::String& silenceThresholdOutTooltip()
{
    static const juce::String text("Threshold to detect end of sound (0.0 - 1.0)");
    return text;
}
} // namespace ControlPanelCopy


#endif // AUDIOFILER_CONTROLPANELCOPY_H
