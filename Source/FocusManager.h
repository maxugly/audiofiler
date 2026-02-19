/**
 * @file FocusManager.h
 * @brief Manages the "focus" state of the application.
 */

#ifndef AUDIOFILER_FOCUSMANAGER_H
#define AUDIOFILER_FOCUSMANAGER_H

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
#else
    #include <JuceHeader.h>
#endif

class ControlPanel;

/**
 * @brief Represents the specific target that currently key input or display
 * logic should focus on.
 */
enum class FocusTarget {
  None,
  CutIn,     ///< The Cut In marker is being dragged or hovered.
  CutOut,    ///< The Cut Out marker is being dragged or hovered.
  Playback,   ///< Standard playback position.
  MouseManual ///< User is manually scrubbing with the mouse (but not dragging a
              ///< marker).
};

/**
 * @class FocusManager
 * @brief Centralizes the logic for determining "what matters right now" in the
 * UI.
 */
class FocusManager {
public:
  explicit FocusManager(ControlPanel &owner);

  /**
   * @brief Determines the current focus target.
   * @return The active FocusTarget.
   */
  FocusTarget getCurrentTarget() const;

  /**
   * @brief Gets the time value (in seconds) associated with the current focus
   * target.
   */
  double getFocusedTime() const;

  /**
   * @brief Calculates a unified multiplier for value changes.
   */
  static double getStepMultiplier(bool shift, bool ctrl);

private:
  ControlPanel &owner;
};

#endif // AUDIOFILER_FOCUSMANAGER_H
