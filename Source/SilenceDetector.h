

#ifndef AUDIOFILER_SILENCEDETECTOR_H
#define AUDIOFILER_SILENCEDETECTOR_H

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
#else
    #include <JuceHeader.h>
#endif

#include "Config.h"

class ControlPanel;

class SilenceThresholdPresenter;

class SilenceDetector
{
public:

    explicit SilenceDetector(ControlPanel& ownerPanel);

    ~SilenceDetector();

    void detectInSilence();

    void detectOutSilence();

    juce::TextEditor& getInSilenceThresholdEditor() { return inSilenceThresholdEditor; }

    juce::TextEditor& getOutSilenceThresholdEditor() { return outSilenceThresholdEditor; }

    bool getIsAutoCutInActive() const noexcept { return isAutoCutInActive; }

    void setIsAutoCutInActive(bool active) noexcept { isAutoCutInActive = active; }

    bool getIsAutoCutOutActive() const noexcept { return isAutoCutOutActive; }

    void setIsAutoCutOutActive(bool active) noexcept { isAutoCutOutActive = active; }

    float getCurrentInSilenceThreshold() const noexcept { return currentInSilenceThreshold; }
    float getCurrentOutSilenceThreshold() const noexcept { return currentOutSilenceThreshold; }

private:
    friend class SilenceThresholdPresenter;

    ControlPanel& owner;
    juce::TextEditor inSilenceThresholdEditor;
    juce::TextEditor outSilenceThresholdEditor;

    float currentInSilenceThreshold;
    float currentOutSilenceThreshold;

    bool isAutoCutInActive = false;
    bool isAutoCutOutActive = false;

    std::unique_ptr<SilenceThresholdPresenter> thresholdPresenter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SilenceDetector)
};

#endif 
