

#include "SessionState.h"

SessionState::SessionState()
{

    cutPrefs.cutIn = 0.0;
    cutPrefs.cutOut = 0.0;
}

void SessionState::addListener(Listener* listener)
{
    listeners.add(listener);
}

void SessionState::removeListener(Listener* listener)
{
    listeners.remove(listener);
}

MainDomain::CutPreferences SessionState::getCutPrefs() const
{
    const juce::ScopedLock lock(stateLock);
    return cutPrefs;
}

void SessionState::setCutActive(bool active)
{
    const juce::ScopedLock lock(stateLock);
    if (cutPrefs.active != active)
    {
        cutPrefs.active = active;
        listeners.call([this](Listener& l) { l.cutPreferenceChanged(cutPrefs); });
    }
}

void SessionState::setAutoCutInActive(bool active)
{
    const juce::ScopedLock lock(stateLock);
    if (cutPrefs.autoCut.inActive != active)
    {
        cutPrefs.autoCut.inActive = active;
        listeners.call([this](Listener& l) { l.cutPreferenceChanged(cutPrefs); });
    }
}

void SessionState::setAutoCutOutActive(bool active)
{
    const juce::ScopedLock lock(stateLock);
    if (cutPrefs.autoCut.outActive != active)
    {
        cutPrefs.autoCut.outActive = active;
        listeners.call([this](Listener& l) { l.cutPreferenceChanged(cutPrefs); });
    }
}

void SessionState::setThresholdIn(float threshold)
{
    const juce::ScopedLock lock(stateLock);
    if (cutPrefs.autoCut.thresholdIn != threshold)
    {
        cutPrefs.autoCut.thresholdIn = threshold;
        listeners.call([this](Listener& l) { l.cutPreferenceChanged(cutPrefs); });
    }
}

void SessionState::setThresholdOut(float threshold)
{
    const juce::ScopedLock lock(stateLock);
    if (cutPrefs.autoCut.thresholdOut != threshold)
    {
        cutPrefs.autoCut.thresholdOut = threshold;
        listeners.call([this](Listener& l) { l.cutPreferenceChanged(cutPrefs); });
    }
}

void SessionState::setCutIn(double value)
{
    const juce::ScopedLock lock(stateLock);
    if (cutPrefs.cutIn != value)
    {
        cutPrefs.cutIn = value;
        if (!currentFilePath.isEmpty())
            metadataCache[currentFilePath].cutIn = value;
        listeners.call([this](Listener& l) { l.cutPreferenceChanged(cutPrefs); });
    }
}

void SessionState::setCutOut(double value)
{
    const juce::ScopedLock lock(stateLock);
    if (cutPrefs.cutOut != value)
    {
        cutPrefs.cutOut = value;
        if (!currentFilePath.isEmpty())
            metadataCache[currentFilePath].cutOut = value;
        listeners.call([this](Listener& l) { l.cutPreferenceChanged(cutPrefs); });
    }
}

double SessionState::getCutIn() const
{
    const juce::ScopedLock lock(stateLock);
    return getMetadataForFile(currentFilePath).cutIn;
}

double SessionState::getCutOut() const
{
    const juce::ScopedLock lock(stateLock);
    return getMetadataForFile(currentFilePath).cutOut;
}

FileMetadata SessionState::getMetadataForFile(const juce::String& filePath) const
{
    const juce::ScopedLock lock(stateLock);
    const auto it = metadataCache.find(filePath);
    if (it != metadataCache.end())
        return it->second;
    return FileMetadata{};
}

FileMetadata SessionState::getCurrentMetadata() const
{
    const juce::ScopedLock lock(stateLock);
    return getMetadataForFile(currentFilePath);
}

bool SessionState::hasMetadataForFile(const juce::String& filePath) const
{
    const juce::ScopedLock lock(stateLock);
    return metadataCache.find(filePath) != metadataCache.end();
}

void SessionState::setCurrentFilePath(const juce::String& filePath)
{
    const juce::ScopedLock lock(stateLock);
    currentFilePath = filePath;
}

juce::String SessionState::getCurrentFilePath() const
{
    const juce::ScopedLock lock(stateLock);
    return currentFilePath;
}

void SessionState::setMetadataForFile(const juce::String& filePath, const FileMetadata& newMetadata)
{
    const juce::ScopedLock lock(stateLock);
    metadataCache[filePath] = newMetadata;

    if (filePath == currentFilePath)
    {
        cutPrefs.cutIn = newMetadata.cutIn;
        cutPrefs.cutOut = newMetadata.cutOut;
        listeners.call([this](Listener& l) { l.cutPreferenceChanged(cutPrefs); });
    }
}
