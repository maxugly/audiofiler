/**
 * @file FocusManager.h
 * @brief Manages the "focus" state of the application, determining which time
 * position or value should be currently displayed or manipulated based on user
 * interaction.
 */

#ifndef AUDIOFILER_FOCUSMANAGER_H
#define AUDIOFILER_FOCUSMANAGER_H

#include <JuceHeader.h>

class ControlPanel;

/**
 * @brief Represents the specific target that currently key input or display
 * logic should focus on.
 */
enum class FocusTarget {
  None,
  LoopIn,     ///< The Loop In marker is being dragged or hovered.
  LoopOut,    ///< The Loop Out marker is being dragged or hovered.
  Playback,   ///< Standard playback position.
  MouseManual ///< User is manually scrubbing with the mouse (but not dragging a
              ///< marker).
};

/**
 * @class FocusManager
 * @brief Centralizes the logic for determining "what matters right now" in the
 * UI.
 *
 * The FocusManager implements a strict **Hierarchy of Intent** to resolve
 * conflicts between multiple potential sources of focus (e.g., playback running
 * while user hovers a text box).
 *
 * **Hierarchy of Intent (Highest to Lowest Priority):**
 * 1. **Dragging**: The user is actively dragging a handle (Loop In/Out).
 *    - This overrides everything else. If you are moving a marker, we show that
 * marker's time.
 * 2. **Scrubbing**: The user is manually scrubbing the timeline (MouseManual).
 *    - Example: Right-click + Drag on the waveform.
 * 3. **Hovering**: The user is hovering over a specific control.
 *    - Example: Mouse over the "Loop In" timer box highlights that time.
 * 4. **Playback**: The default state.
 *    - If no user interaction is happening, we follow the playhead.
 */
class FocusManager {
public:
  /**
   * @brief Constructor.
   * @param owner The ControlPanel that owns this manager and provides access to
   * other components.
   */
  explicit FocusManager(ControlPanel &owner);

  /**
   * @brief Determines the current focus target based on the Hierarchy of
   * Intent.
   * @return The active FocusTarget (e.g., LoopIn, Playback).
   */
  FocusTarget getCurrentTarget() const;

  /**
   * @brief Gets the time value (in seconds) associated with the current focus
   * target.
   *
   * - If Dragging LoopIn -> Returns LoopIn time.
   * - If Scrubbing -> Returns mouse scrub time.
   * - If Playback -> Returns current transport position.
   *
   * @return The relevant time in seconds.
   */
  double getFocusedTime() const;

  /**
   * @brief Calculates a unified multiplier for value changes
   * (scrolling/dragging) based on modifier keys.
   *
   * - No Modifiers: 1.0 (Coarse)
   * - Shift: 0.1 (Fine)
   * - Shift + Ctrl: 0.01 (Precise)
   *
   * @param shift Whether the Shift key is held.
   * @param ctrl Whether the Ctrl (or Cmd on Mac) key is held.
   * @return The multiplier factor.
   */
  static double getStepMultiplier(bool shift, bool ctrl);

private:
  ControlPanel &owner;
};

#endif // AUDIOFILER_FOCUSMANAGER_H
