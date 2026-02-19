#include "SilenceDetector.h"
#include "ControlPanel.h"
#include "AudioPlayer.h"
#include "SilenceThresholdPresenter.h"
#include "SilenceDetectionPresenter.h"

SilenceDetector::SilenceDetector(ControlPanel& ownerPanel)
    : owner(ownerPanel),
      currentInSilenceThreshold(Config::Audio::silenceThresholdIn),
      currentOutSilenceThreshold(Config::Audio::silenceThresholdOut)
{
    thresholdPresenter = std::make_unique<SilenceThresholdPresenter>(*this, owner);
}

SilenceDetector::~SilenceDetector() = default;

void SilenceDetector::detectInSilence()
{
    if (auto* presenter = owner.getSilenceDetectionPresenter())
        presenter->startSilenceAnalysis(currentInSilenceThreshold, true);
}

void SilenceDetector::detectOutSilence()
{
    if (auto* presenter = owner.getSilenceDetectionPresenter())
        presenter->startSilenceAnalysis(currentOutSilenceThreshold, false);
}
