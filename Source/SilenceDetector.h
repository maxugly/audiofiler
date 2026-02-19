/**
 * @file SilenceDetector.h
 * @brief Defines the SilenceDetector class.
 * @ingroup Engine
 */

#ifndef AUDIOFILER_SILENCEDETECTOR_H
#define AUDIOFILER_SILENCEDETECTOR_H

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
#else
    #include <JuceHeader.h>
#endif

#include "Config.h"

/**
 * @class ControlPanel
 * @brief Home: View.
 *
 */
class ControlPanel;
/**
 * @class SilenceThresholdPresenter
 * @brief Home: Presenter.
 *
 * @see ControlPanel
 */
class SilenceThresholdPresenter;

/**
 * @class SilenceDetector
 * @brief Logic for automatically finding sound boundaries (cut-in/out).
 */
class SilenceDetector
{
public:
    /**
     * @brief Undocumented method.
     * @param ownerPanel [in] Description for ownerPanel.
     */
    explicit SilenceDetector(ControlPanel& ownerPanel);
    /**
     * @brief Undocumented method.
     */
    ~SilenceDetector();

    /**
     * @brief Undocumented method.
     */
    void detectInSilence();
    /**
     * @brief Undocumented method.
     */
    void detectOutSilence();

    /**
     * @brief Gets the InSilenceThresholdEditor.
     * @return juce::TextEditor&
     */
    juce::TextEditor& getInSilenceThresholdEditor() { return inSilenceThresholdEditor; }
    /**
     * @brief Gets the OutSilenceThresholdEditor.
     * @return juce::TextEditor&
     */
    juce::TextEditor& getOutSilenceThresholdEditor() { return outSilenceThresholdEditor; }

    bool getIsAutoCutInActive() const noexcept { return isAutoCutInActive; }
    /**
     * @brief Sets the IsAutoCutInActive.
     * @param active [in] Description for active.
     */
    void setIsAutoCutInActive(bool active) noexcept { isAutoCutInActive = active; }

    bool getIsAutoCutOutActive() const noexcept { return isAutoCutOutActive; }
    /**
     * @brief Sets the IsAutoCutOutActive.
     * @param active [in] Description for active.
     */
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

#endif // AUDIOFILER_SILENCEDETECTOR_H
