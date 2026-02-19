/**
 * @file ControlPanelCopy.h
 * @brief Defines the ControlPanelCopy class.
 * @ingroup Views
 */

#ifndef AUDIOFILER_CONTROLPANELCOPY_H
#define AUDIOFILER_CONTROLPANELCOPY_H

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
#else
    #include <JuceHeader.h>
#endif

#include "Config.h"

namespace ControlPanelCopy
{
/**
 * @brief Undocumented method.
 * @return const juce::String&
 */
inline const juce::String& openButtonText() { return Config::Labels::openButton; }
/**
 * @brief Undocumented method.
 * @return const juce::String&
 */
inline const juce::String& playButtonText() { return Config::Labels::playButton; }
/**
 * @brief Undocumented method.
 * @return const juce::String&
 */
inline const juce::String& stopButtonText() { return Config::Labels::stopButton; }
/**
 * @brief Undocumented method.
 * @return const juce::String&
 */
inline const juce::String& viewModeClassicText() { return Config::Labels::viewModeClassic; }
/**
 * @brief Undocumented method.
 * @return const juce::String&
 */
inline const juce::String& viewModeOverlayText() { return Config::Labels::viewModeOverlay; }
/**
 * @brief Undocumented method.
 * @return const juce::String&
 */
inline const juce::String& channelViewMonoText() { return Config::Labels::channelViewMono; }
/**
 * @brief Undocumented method.
 * @return const juce::String&
 */
inline const juce::String& channelViewStereoText() { return Config::Labels::channelViewStereo; }
/**
 * @brief Undocumented method.
 * @return const juce::String&
 */
inline const juce::String& qualityButtonText() { return Config::Labels::qualityButton; }
/**
 * @brief Undocumented method.
 * @return const juce::String&
 */
inline const juce::String& qualityHighText() { return Config::Labels::qualityHigh; }
/**
 * @brief Undocumented method.
 * @return const juce::String&
 */
inline const juce::String& qualityMediumText() { return Config::Labels::qualityMedium; }
/**
 * @brief Undocumented method.
 * @return const juce::String&
 */
inline const juce::String& qualityLowText() { return Config::Labels::qualityLow; }
/**
 * @brief Undocumented method.
 * @return const juce::String&
 */
inline const juce::String& exitButtonText() { return Config::Labels::exitButton; }
/**
 * @brief Undocumented method.
 * @return const juce::String&
 */
inline const juce::String& statsButtonText() { return Config::Labels::statsButton; }
/**
 * @brief Undocumented method.
 * @return const juce::String&
 */
inline const juce::String& repeatButtonText() { return Config::Labels::repeatButton; }
/**
 * @brief Undocumented method.
 * @return const juce::String&
 */
inline const juce::String& autoplayButtonText() { return Config::Labels::autoplayButton; }
/**
 * @brief Undocumented method.
 * @return const juce::String&
 */
inline const juce::String& autoCutInButtonText() { return Config::Labels::autoCutInButton; }
/**
 * @brief Undocumented method.
 * @return const juce::String&
 */
inline const juce::String& autoCutOutButtonText() { return Config::Labels::autoCutOutButton; }
/**
 * @brief Undocumented method.
 * @return const juce::String&
 */
inline const juce::String& cutButtonText() { return Config::Labels::cutButton; }
/**
 * @brief Undocumented method.
 * @return const juce::String&
 */
inline const juce::String& cutInButtonText() { return Config::Labels::cutInButton; }
/**
 * @brief Undocumented method.
 * @return const juce::String&
 */
inline const juce::String& cutOutButtonText() { return Config::Labels::cutOutButton; }
/**
 * @brief Undocumented method.
 * @return const juce::String&
 */
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
} 


#endif 
