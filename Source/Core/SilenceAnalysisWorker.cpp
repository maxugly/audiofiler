

#include "Core/SilenceAnalysisWorker.h"
#include "Workers/SilenceAnalysisAlgorithms.h"
#include "Core/AudioPlayer.h"
#include "Workers/SilenceDetectionLogger.h"
#include "Core/SessionState.h"
#include "Core/FileMetadata.h"

#include <algorithm>
#include <cmath>
#include <mutex>

SilenceAnalysisWorker::SilenceAnalysisWorker(SilenceWorkerClient& owner, SessionState& state)
    : Thread("SilenceWorker"), client(owner), sessionState(state)
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

    AudioPlayer& audioPlayer = client.getAudioPlayer();
    assignedFilePath = audioPlayer.getLoadedFile().getFullPathName();
    wasPlayingBeforeScan = audioPlayer.isPlaying();

    if (wasPlayingBeforeScan)
        audioPlayer.stopPlayback();

    startThread();
}

void SilenceAnalysisWorker::run()
{
    busy.store(true);

    AudioPlayer& audioPlayer = client.getAudioPlayer();
    const juce::String filePath = assignedFilePath;

    juce::File fileToAnalyze(filePath);

    std::unique_ptr<juce::AudioFormatReader> localReader(audioPlayer.getFormatManager().createReaderFor(fileToAnalyze));

    juce::int64 result = -1;
    bool success = false;
    juce::int64 sampleRate = 0;
    juce::int64 lengthInSamples = 0;

    if (localReader != nullptr)
    {
        sampleRate = (juce::int64)localReader->sampleRate;
        lengthInSamples = localReader->lengthInSamples;

        if (detectingIn.load())
        {
            result = SilenceAnalysisAlgorithms::findSilenceIn(*localReader, threshold.load(), this);
        }
        else
        {
            result = SilenceAnalysisAlgorithms::findSilenceOut(*localReader, threshold.load(), this);
        }
        success = true;
    }

    std::weak_ptr<bool> weakToken = lifeToken;

    juce::MessageManager::callAsync([this, weakToken, result, success, sampleRate, lengthInSamples, filePath]()
    {

        if (auto token = weakToken.lock())
        {

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
                 client.logStatusMessage(juce::String("Scanning for Cut Points..."));

                 FileMetadata metadata = sessionState.getMetadataForFile(filePath);
                 if (result != -1)
                 {
                     const double resultSeconds = (double)result / (double)sampleRate;
                     if (detectingIn.load())
                     {
                         metadata.cutIn = resultSeconds;
                         client.setCutStart((int)result);
                         client.logStatusMessage(juce::String("Silence Boundary (Start) set to sample ") + juce::String(result));

                         if (client.isCutModeActive())
                             player.setPlayheadPosition(resultSeconds);
                     }
                     else
                     {
                         const juce::int64 tailSamples = (juce::int64)(sampleRate * 0.05); 
                         const juce::int64 endPoint64 = result + tailSamples;
                         const juce::int64 finalEndPoint = std::min(endPoint64, lengthInSamples);
                         const double endSeconds = (double)finalEndPoint / (double)sampleRate;

                         metadata.cutOut = endSeconds;
                         client.setCutEnd((int)finalEndPoint);
                         client.logStatusMessage(juce::String("Silence Boundary (End) set to sample ") + juce::String(finalEndPoint));
                     }
                 }
                 else
                 {
                     client.logStatusMessage("No Silence Boundaries detected.");
                 }

                 metadata.isAnalyzed = true;
                 sessionState.setMetadataForFile(filePath, metadata);
            }

            if (wasPlayingBeforeScan)
                player.startPlayback();

            busy.store(false);
        }
    });
}
