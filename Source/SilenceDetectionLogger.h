/**
 * @file SilenceDetectionLogger.h
 * @brief Defines the SilenceDetectionLogger class.
 * @ingroup Engine
 */

#ifndef AUDIOFILER_SILENCEDETECTIONLOGGER_H
#define AUDIOFILER_SILENCEDETECTIONLOGGER_H

#if defined(JUCE_HEADLESS)
    #include <juce_core/juce_core.h>
#else
    #include <JuceHeader.h>
#endif

#include "SilenceWorkerClient.h"

/**
 * @class SilenceDetectionLogger
 * @brief Provides consistent logging for silence detection events.
 */
class SilenceDetectionLogger final
{
public:
    /**
     * @brief Undocumented method.
     * @param client [in] Description for client.
     */
    static void logNoAudioLoaded(SilenceWorkerClient& client);
    /**
     * @brief Undocumented method.
     * @param client [in] Description for client.
     * @param direction [in] Description for direction.
     * @param length [in] Description for length.
     */
    static void logReadingSamples(SilenceWorkerClient& client, const juce::String& direction, juce::int64 length);
    /**
     * @brief Undocumented method.
     * @param client [in] Description for client.
     */
    static void logZeroLength(SilenceWorkerClient& client);
    /**
     * @brief Undocumented method.
     * @param client [in] Description for client.
     * @param sampleIndex [in] Description for sampleIndex.
     * @param sampleRate [in] Description for sampleRate.
     */
    static void logCutInSet(SilenceWorkerClient& client, juce::int64 sampleIndex, double sampleRate);
    /**
     * @brief Undocumented method.
     * @param client [in] Description for client.
     * @param sampleIndex [in] Description for sampleIndex.
     * @param sampleRate [in] Description for sampleRate.
     */
    static void logCutOutSet(SilenceWorkerClient& client, juce::int64 sampleIndex, double sampleRate);
    /**
     * @brief Undocumented method.
     * @param client [in] Description for client.
     * @param boundaryDescription [in] Description for boundaryDescription.
     */
    static void logNoSoundFound(SilenceWorkerClient& client, const juce::String& boundaryDescription);
    /**
     * @brief Undocumented method.
     * @param client [in] Description for client.
     */
    static void logAudioTooLarge(SilenceWorkerClient& client);
};

#endif 
