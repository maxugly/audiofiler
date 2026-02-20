#pragma once

#include "MainDomain.h"
#include "FileMetadata.h"
#include <juce_core/juce_core.h>
#include <map>

/**
 * @ingroup State
 * @class SessionState
 * @brief The central data model for the application.
 * @details This class holds the current application state, including file metadata,
 *          cut preferences, and other user settings. It acts as a "hub" for communication,
 *          allowing components to listen for state changes without tight coupling.
 *
 *          It uses `juce::ListenerList` to notify registered listeners when properties change.
 *
 * @see AudioPlayer
 * @see ControlPanel
 */
class SessionState {
public:

    class Listener {
    public:
        virtual ~Listener() = default;

        virtual void cutPreferenceChanged(const MainDomain::CutPreferences& prefs) { juce::ignoreUnused(prefs); }
        virtual void cutInChanged(double value) { juce::ignoreUnused(value); }
        virtual void cutOutChanged(double value) { juce::ignoreUnused(value); }
    };

    SessionState();

    void addListener(Listener* listener);

    void removeListener(Listener* listener);

    MainDomain::CutPreferences getCutPrefs() const;

    void setCutActive(bool active);

    void setAutoPlayActive(bool active);

    void setAutoCutInActive(bool active);

    void setAutoCutOutActive(bool active);

    void setThresholdIn(float threshold);

    void setThresholdOut(float threshold);

    void setCutIn(double value);

    void setCutOut(double value);
    double getCutIn() const;
    double getCutOut() const;

    void setTotalDuration(double duration);
    double getTotalDuration() const;

    FileMetadata getMetadataForFile(const juce::String& filePath) const;
    FileMetadata getCurrentMetadata() const;
    void setMetadataForFile(const juce::String& filePath, const FileMetadata& newMetadata);

    bool hasMetadataForFile(const juce::String& filePath) const;
    void setCurrentFilePath(const juce::String& filePath);
    juce::String getCurrentFilePath() const;

private:
    MainDomain::CutPreferences cutPrefs;
    juce::String currentFilePath;
    double totalDuration { 0.0 };
    std::map<juce::String, FileMetadata> metadataCache;
    juce::ListenerList<Listener> listeners;

    mutable juce::CriticalSection stateLock;
};
