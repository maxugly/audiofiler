/**
 * @file WaveformManager.h
 * @brief Defines the WaveformManager class.
 * @ingroup Engine
 */

#ifndef AUDIOFILER_WAVEFORMMANAGER_H
#define AUDIOFILER_WAVEFORMMANAGER_H

#if defined(JUCE_HEADLESS)
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_events/juce_events.h>
#else
#include <JuceHeader.h>
#endif

/**
 * @class WaveformManager
 * @brief Owns and manages waveform data for Cut boundary rendering.
 */
class WaveformManager
{
public:
    /**
     * @brief Undocumented method.
     * @param formatManagerIn [in] Description for formatManagerIn.
     */
    explicit WaveformManager(juce::AudioFormatManager& formatManagerIn);

    /**
     * @brief Undocumented method.
     * @param file [in] Description for file.
     */
    void loadFile(const juce::File& file);

    /**
     * @brief Gets the Thumbnail.
     * @return juce::AudioThumbnail&
     */
    juce::AudioThumbnail& getThumbnail();
    /**
     * @brief Gets the Thumbnail.
     * @return const juce::AudioThumbnail&
     */
    const juce::AudioThumbnail& getThumbnail() const;

    /**
     * @brief Undocumented method.
     * @param listener [in] Description for listener.
     */
    void addChangeListener(juce::ChangeListener* listener);
    /**
     * @brief Undocumented method.
     * @param listener [in] Description for listener.
     */
    void removeChangeListener(juce::ChangeListener* listener);

private:
    juce::AudioFormatManager& formatManager;
    juce::AudioThumbnailCache thumbnailCache { 5 };
    juce::AudioThumbnail thumbnail { 512, formatManager, thumbnailCache };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformManager)
};

#endif 
