

#ifndef AUDIOFILER_FOCUSMANAGER_H
#define AUDIOFILER_FOCUSMANAGER_H

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
#else
    #include <JuceHeader.h>
#endif

class ControlPanel;

enum class FocusTarget {
  None,
  CutIn,     
  CutOut,    
  Playback,   
  MouseManual 

};

class FocusManager {
public:

  explicit FocusManager(ControlPanel &owner);

  FocusTarget getCurrentTarget() const;

  double getFocusedTime() const;

  static double getStepMultiplier(bool shift, bool ctrl);

private:
  ControlPanel &owner;
};

#endif 
