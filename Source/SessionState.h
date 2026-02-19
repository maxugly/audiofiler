

#pragma once

#include "MainDomain.h"
#include "FileMetadata.h"
#include <juce_core/juce_core.h>
#include <map>

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

    MainDomain::CutPreferences getCutPrefs() const;

    void setCutActive(bool active);

    void setAutoCutInActive(bool active);

    void setAutoCutOutActive(bool active);

    void setThresholdIn(float threshold);

    void setThresholdOut(float threshold);

    void setCutIn(double value);

    void setCutOut(double value);
    double getCutIn() const;
    double getCutOut() const;
    FileMetadata getMetadataForFile(const juce::String& filePath) const;
    FileMetadata getCurrentMetadata() const;
    void setMetadataForFile(const juce::String& filePath, const FileMetadata& newMetadata);

    bool hasMetadataForFile(const juce::String& filePath) const;
    void setCurrentFilePath(const juce::String& filePath);
    juce::String getCurrentFilePath() const;

private:
    MainDomain::CutPreferences cutPrefs;
    juce::String currentFilePath;
    std::map<juce::String, FileMetadata> metadataCache;
    juce::ListenerList<Listener> listeners;

    mutable juce::CriticalSection stateLock;
};
