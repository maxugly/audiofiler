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

/**
 * @class ControlPanel
 * @brief Home: View.
 *
 */
class ControlPanel;

/**
 * @brief Represents the specific target that currently key input or display
 * logic should focus on.
 */
enum class FocusTarget {
  None,
  CutIn,     
  CutOut,    
  Playback,   
  MouseManual 
              
};

/**
 * @class FocusManager
 * @brief Centralizes the logic for determining "what matters right now" in the
 * UI.
 */
class FocusManager {
public:
  /**
   * @brief Undocumented method.
   * @param owner [in] Description for owner.
   */
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

#endif 
