#include "SessionState.h"
#include "Config.h"

SessionState::SessionState() {
    cutPrefs.active = false;
    cutPrefs.autoCut.inActive = false;
    cutPrefs.autoCut.outActive = false;
    cutPrefs.autoCut.thresholdIn = Config::Audio::silenceThresholdIn;
    cutPrefs.autoCut.thresholdOut = Config::Audio::silenceThresholdOut;

    isLooping = true;
    autoplay = true;
}

void SessionState::addListener(Listener* listener)
{
    listeners.add(listener);
}

void SessionState::removeListener(Listener* listener)
{
    listeners.remove(listener);
}

void SessionState::setCutActive(bool active)
{
    if (cutPrefs.active != active)
    {
        cutPrefs.active = active;
        listeners.call([this](Listener& l) { l.cutPreferenceChanged(cutPrefs); });
        DBG("Broadcasting cutPreferenceChanged: cutActive = " << (active ? "true" : "false"));
    }
}

void SessionState::setAutoCutInActive(bool active)
{
    if (cutPrefs.autoCut.inActive != active)
    {
        cutPrefs.autoCut.inActive = active;
        listeners.call([this](Listener& l) { l.cutPreferenceChanged(cutPrefs); });
        DBG("Broadcasting cutPreferenceChanged: autoCutInActive = " << (active ? "true" : "false"));
    }
}

void SessionState::setAutoCutOutActive(bool active)
{
    if (cutPrefs.autoCut.outActive != active)
    {
        cutPrefs.autoCut.outActive = active;
        listeners.call([this](Listener& l) { l.cutPreferenceChanged(cutPrefs); });
        DBG("Broadcasting cutPreferenceChanged: autoCutOutActive = " << (active ? "true" : "false"));
    }
}

void SessionState::setThresholdIn(float threshold)
{
    if (cutPrefs.autoCut.thresholdIn != threshold)
    {
        cutPrefs.autoCut.thresholdIn = threshold;
        listeners.call([this](Listener& l) { l.cutPreferenceChanged(cutPrefs); });
        DBG("Broadcasting cutPreferenceChanged: thresholdIn = " << threshold);
    }
}

void SessionState::setThresholdOut(float threshold)
{
    if (cutPrefs.autoCut.thresholdOut != threshold)
    {
        cutPrefs.autoCut.thresholdOut = threshold;
        listeners.call([this](Listener& l) { l.cutPreferenceChanged(cutPrefs); });
        DBG("Broadcasting cutPreferenceChanged: thresholdOut = " << threshold);
    }
}
