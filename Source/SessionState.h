#pragma once

#include "MainDomain.h"
#include <juce_core/juce_core.h>

class SessionState {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void cutPreferenceChanged(const MainDomain::CutPreferences& prefs) = 0;
    };

    SessionState();

    void addListener(Listener* listener);
    void removeListener(Listener* listener);

    // Getters
    const MainDomain::CutPreferences& getCutPrefs() const { return cutPrefs; }

    // Setters
    void setCutActive(bool active);
    void setAutoCutInActive(bool active);
    void setAutoCutOutActive(bool active);
    void setThresholdIn(float threshold);
    void setThresholdOut(float threshold);
    void setCutIn(double value);
    void setCutOut(double value);

    bool isLooping;
    bool autoplay;

private:
    MainDomain::CutPreferences cutPrefs;
    juce::ListenerList<Listener> listeners;
};
