/**
 * @file TransportPresenter.h
 * @brief Defines the TransportPresenter class.
 * @ingroup Presenters
 */

#ifndef AUDIOFILER_TRANSPORTPRESENTER_H
#define AUDIOFILER_TRANSPORTPRESENTER_H

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
#else
    #include <JuceHeader.h>
#endif

/**
 * @class ControlPanel
 * @brief Home: View.
 *
 */
class ControlPanel;

/**
 * @class TransportPresenter
 * @brief Encapsulates the behaviour of transport-related toggles (repeat, autoplay, cut mode).
 */
class TransportPresenter final
{
public:
    /**
     * @brief Constructs the presenter.
     */
    explicit TransportPresenter(ControlPanel& ownerPanel);

    /**
     * @brief Handles the repeat button toggle event.
     */
    void handleRepeatToggle(bool shouldRepeat);

    /**
     * @brief Handles the autoplay button toggle event.
     */
    void handleAutoplayToggle(bool shouldAutoplay);

    /**
     * @brief Handles the cut mode button toggle event.
     */
    void handleCutModeToggle(bool enableCutMode);

private:
    /**
     * @brief Undocumented method.
     */
    void enforceCutBounds() const;

    ControlPanel& owner;
};

#endif // AUDIOFILER_TRANSPORTPRESENTER_H
