#ifndef AUDIOFILER_INTERACTIONCOORDINATOR_H
#define AUDIOFILER_INTERACTIONCOORDINATOR_H

#include "Core/AppEnums.h"
#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
    #include <juce_gui_basics/juce_gui_basics.h>
#else
    #include <JuceHeader.h>
#endif
#include <utility>

/**
 * @class InteractionCoordinator
 * @brief Manages transient UI interaction states that are not part of the global SessionState.
 * 
 * This class coordinates states like "which marker is being zoomed" or "interaction mode 
 * overrides" to keep ControlPanel focused on layout and presenters focused on logic.
 */
class InteractionCoordinator
{
public:
    InteractionCoordinator() = default;
    ~InteractionCoordinator() = default;

    /** @brief Sets the currently active zoom point (In, Out, or None). */
    void setActiveZoomPoint(AppEnums::ActiveZoomPoint point) { m_activeZoomPoint = point; }

    /** @brief Returns the currently active zoom point. */
    AppEnums::ActiveZoomPoint getActiveZoomPoint() const { return m_activeZoomPoint; }

    /** @brief Sets a manual zoom point override (e.g. from mouse hover). */
    void setManualZoomPoint(AppEnums::ActiveZoomPoint point) { m_manualZoomPoint = point; }

    /** @brief Returns the manual zoom point override. */
    AppEnums::ActiveZoomPoint getManualZoomPoint() const { return m_manualZoomPoint; }

    /** @brief Sets whether a jump to cut-in is pending. */
    void setNeedsJumpToCutIn(bool needs) { m_needsJumpToCutIn = needs; }

    /** @brief Returns true if a jump to cut-in is pending. */
    bool getNeedsJumpToCutIn() const { return m_needsJumpToCutIn; }

    /** @brief Sets the current zoom popup bounds. */
    void setZoomPopupBounds(juce::Rectangle<int> bounds) { m_zoomPopupBounds = bounds; }

    /** @brief Returns the current zoom popup bounds. */
    juce::Rectangle<int> getZoomPopupBounds() const { return m_zoomPopupBounds; }

    /** @brief Sets the current zoom time range. */
    void setZoomTimeRange(double start, double end) { m_zoomTimeRange = {start, end}; }

    /** @brief Returns the current zoom time range. */
    std::pair<double, double> getZoomTimeRange() const { return m_zoomTimeRange; }

    /** @brief Returns true if eye candy (connecting lines, etc.) should be shown. */
    bool shouldShowEyeCandy() const { return m_showEyeCandy; }

    /** @brief Sets whether eye candy should be shown. */
    void setShouldShowEyeCandy(bool shouldShow) { m_showEyeCandy = shouldShow; }

    /** @brief Returns the current placement mode. */
    AppEnums::PlacementMode getPlacementMode() const { return m_placementMode; }

    /** @brief Sets the current placement mode. */
    void setPlacementMode(AppEnums::PlacementMode mode) { m_placementMode = mode; }

    /** @brief Snaps a raw time to relevant boundaries (e.g. zero-crossings or samples). */
    double getSnappedTime(double rawTime, double sampleRate) const;

    /** @brief Validates and constraints a marker position based on project rules. */
    void validateMarkerPosition(AppEnums::ActiveZoomPoint marker, double& newPosition, double cutIn, double cutOut, double duration) const;

    /** @brief Handles the logic for moving a full cut region. */
    void constrainFullRegionMove(double& newIn, double& newOut, double length, double duration) const;

private:
    AppEnums::ActiveZoomPoint m_activeZoomPoint = AppEnums::ActiveZoomPoint::None;
    AppEnums::ActiveZoomPoint m_manualZoomPoint = AppEnums::ActiveZoomPoint::None;
    bool m_needsJumpToCutIn = false;
    juce::Rectangle<int> m_zoomPopupBounds;
    std::pair<double, double> m_zoomTimeRange;
    bool m_showEyeCandy = false;
    AppEnums::PlacementMode m_placementMode = AppEnums::PlacementMode::None;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InteractionCoordinator)
};

#endif
