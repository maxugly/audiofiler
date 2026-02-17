#include "SilenceAnalysisWorker.h"
#include "SilenceAnalysisAlgorithms.h"
#include "AudioPlayer.h"
#include "SilenceDetectionLogger.h"

#include <algorithm>
#include <cmath>

SilenceAnalysisWorker::SilenceAnalysisWorker(SilenceWorkerClient& owner)
    : Thread("SilenceWorker"), client(owner)
{
    lifeToken = std::make_shared<bool>(true);
}

SilenceAnalysisWorker::~SilenceAnalysisWorker()
{
    stopThread(4000);
}

bool SilenceAnalysisWorker::isBusy() const
{
    return busy.load() || isThreadRunning();
}

void SilenceAnalysisWorker::startAnalysis(float thresholdVal, bool isIn)
{
    if (isBusy())
        return;

    threshold.store(thresholdVal);
    detectingIn.store(isIn);

    // Pause audio on the main thread before starting background work
    AudioPlayer& audioPlayer = client.getAudioPlayer();
    wasPlayingBeforeScan = audioPlayer.isPlaying();

    
    if (wasPlayingBeforeScan)
        audioPlayer.getTransportSource().stop();

    startThread();
}

void SilenceAnalysisWorker::run()
{
    busy.store(true);

    // Capture necessary state
    // Note: We assume the file is not unloaded during scan.
    AudioPlayer& audioPlayer = client.getAudioPlayer();
    juce::AudioFormatReader* reader = audioPlayer.getAudioFormatReader();
    
    juce::int64 result = -1;
    bool success = false;
    juce::int64 sampleRate = 0;
    juce::int64 lengthInSamples = 0;

    if (reader != nullptr)
    {
        sampleRate = (juce::int64)reader->sampleRate;
        lengthInSamples = reader->lengthInSamples;

        // Run the heavy algorithm
        if (detectingIn.load())
        {
            result = SilenceAnalysisAlgorithms::findSilenceIn(*reader, threshold.load());
        }
        else
        {
            result = SilenceAnalysisAlgorithms::findSilenceOut(*reader, threshold.load());
        }
        success = true;
    }

    // Prepare a weak pointer to our life token
    std::weak_ptr<bool> weakToken = lifeToken;

    // Report back to the UI thread
    juce::MessageManager::callAsync([this, weakToken, result, success, sampleRate, lengthInSamples]()
    {
        // Check if the worker is still alive
        if (auto token = weakToken.lock())
        {
            // Worker is alive, so client is guaranteed to be alive (assuming client owns worker)
            AudioPlayer& player = client.getAudioPlayer();

            if (!success || lengthInSamples <= 0)
            {
                 if (lengthInSamples <= 0 && success) 
                     client.logStatusMessage("Error: Audio file has zero length.", true);
                 else
                     client.logStatusMessage("No audio loaded.", true);
            }
            else
            {
                 client.logStatusMessage(juce::String("Reading ") + (detectingIn.load() ? "start" : "end") + " of sample...");

                 if (result != -1)
                 {
                     if (detectingIn.load())
                     {
                         client.setCutInPosition((double)result / (double)sampleRate);
                         client.logStatusMessage(juce::String("Cut start set to sample ") + juce::String(result));

                         if (client.isCutModeActive())
                             player.getTransportSource().setPosition(client.getCutInPosition());
                     }
                     else
                     {
                         const juce::int64 tailSamples = (juce::int64)(sampleRate * 0.05); // 50ms tail
                         const juce::int64 endPoint64 = result + tailSamples;
                         const juce::int64 finalEndPoint = std::min(endPoint64, lengthInSamples);

                         client.setCutOutPosition((double)finalEndPoint / (double)sampleRate);
                         client.logStatusMessage(juce::String("Cut end set to sample ") + juce::String(finalEndPoint));
                     }
                 }
                 else
                 {
                     client.logStatusMessage("No silence found.");
                 }
            }

            // Resume playback if it was playing
            if (wasPlayingBeforeScan)
                player.getTransportSource().start();

            busy.store(false);
        }
    });
}
