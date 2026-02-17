#include "LoopResetPresenter.h"

#include "ControlPanel.h"
#include "ControlPanelCopy.h"
#include "Config.h"
#include "SilenceDetector.h"
#include "AudioPlayer.h"

LoopResetPresenter::LoopResetPresenter(ControlPanel& ownerPanel)
    : owner(ownerPanel)
{
}

void LoopResetPresenter::clearLoopIn()
{
    owner.setCutInPosition(0.0);
    owner.ensureCutOrder();
    owner.updateCutButtonColors();
    owner.updateCutLabels();
    owner.silenceDetector->setIsAutoCutInActive(false);
    owner.updateComponentStates();
    owner.repaint();
}

void LoopResetPresenter::clearLoopOut()
{
    owner.setCutOutPosition(owner.getAudioPlayer().getThumbnail().getTotalLength());
    owner.ensureCutOrder();
    owner.updateCutButtonColors();
    owner.updateCutLabels();
    owner.silenceDetector->setIsAutoCutOutActive(false);
    owner.updateComponentStates();
    owner.repaint();
}

