/**
 * @file SilenceWorkerClient.h
 * @brief Defines the SilenceWorkerClient class.
 * @ingroup Engine
 */

#ifndef AUDIOFILER_SILENCEWORKERCLIENT_H
#define AUDIOFILER_SILENCEWORKERCLIENT_H

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
#else
    #include <JuceHeader.h>
#endif

/**
 * @class AudioPlayer
 * @brief Home: Engine.
 *
 */
class AudioPlayer;

/**
 * @class SilenceWorkerClient
 * @brief Interface for receiving callbacks from the silence analysis worker.
 */
class SilenceWorkerClient {
public:
    virtual ~SilenceWorkerClient() = default;

    
    virtual AudioPlayer& getAudioPlayer() = 0;

    
    virtual void logStatusMessage(const juce::String& message, bool isError = false) = 0;

    
    virtual bool isCutModeActive() const = 0;

    
    virtual void setCutStart(int sampleIndex) = 0;

    
    virtual void setCutEnd(int sampleIndex) = 0;
};

#endif 
