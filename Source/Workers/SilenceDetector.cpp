

#include "Workers/SilenceDetector.h"
#include "UI/ControlPanel.h"
#include "Core/AudioPlayer.h"
#include "Presenters/SilenceThresholdPresenter.h"
#include "Presenters/SilenceDetectionPresenter.h"

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
