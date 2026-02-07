#include "AudioPlayer.h"

AudioPlayer::AudioPlayer()
    : thumbnailCache(5),
      thumbnail(Config::thumbnailSizePixels, formatManager, thumbnailCache)
{
    formatManager.registerBasicFormats();
    transportSource.addChangeListener(this);
}

AudioPlayer::~AudioPlayer()
{
    transportSource.removeChangeListener(this);
}

juce::Result AudioPlayer::loadFile(const juce::File& file)
{
    auto* reader = formatManager.createReaderFor(file);

    if (reader != nullptr)
    {
        auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
        thumbnail.setSource(new juce::FileInputSource(file));
        readerSource.reset(newSource.release());
        return juce::Result::ok();
    }

    return juce::Result::fail("Failed to read audio file: " + file.getFileName());
}

void AudioPlayer::togglePlayStop()
{
    if (transportSource.isPlaying())
        transportSource.stop();
    else
        transportSource.start();
}

bool AudioPlayer::isPlaying() const
{
    return transportSource.isPlaying();
}

bool AudioPlayer::isLooping() const
{
    return looping;
}

void AudioPlayer::setLooping(bool shouldLoop)
{
    looping = shouldLoop;
}

juce::AudioThumbnail& AudioPlayer::getThumbnail()
{
    return thumbnail;
}

juce::AudioTransportSource& AudioPlayer::getTransportSource()
{
    return transportSource;
}

juce::AudioFormatManager& AudioPlayer::getFormatManager()
{
    return formatManager;
}

void AudioPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void AudioPlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    transportSource.getNextAudioBlock(bufferToFill);

    if (transportSource.hasStreamFinished() && isLooping())
    {
        transportSource.setPosition(0);
        // Optional: If you want immediate playback restart without user input
        // transportSource.start(); 
    }
}

void AudioPlayer::releaseResources()
{
    transportSource.releaseResources();
}

void AudioPlayer::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &transportSource)
    {
        // The transportSource handles its own state changes internally.
        // We just need to forward the change message to listeners.
        sendChangeMessage(); // Forward the change to listeners (MainComponent)
    }
}

juce::AudioFormatReader* AudioPlayer::getAudioFormatReader() const
{
    if (readerSource != nullptr)
        return readerSource->getAudioFormatReader();
    return nullptr;
}
