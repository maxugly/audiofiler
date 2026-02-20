

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

class WaveformManager
{
public:

    explicit WaveformManager(juce::AudioFormatManager& formatManagerIn);

    void loadFile(const juce::File& file);

    juce::AudioThumbnail& getThumbnail();

    const juce::AudioThumbnail& getThumbnail() const;

    void addChangeListener(juce::ChangeListener* listener);

    void removeChangeListener(juce::ChangeListener* listener);

private:
    juce::AudioFormatManager& formatManager;
    juce::AudioThumbnailCache thumbnailCache { 5 };
    juce::AudioThumbnail thumbnail { 512, formatManager, thumbnailCache };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformManager)
};

#endif 
