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
    owner.setLoopInPosition(0.0);
    owner.ensureLoopOrder();
    owner.updateLoopButtonColors();
    owner.updateLoopLabels();
    owner.silenceDetector->setIsAutoCutInActive(false);
    owner.updateComponentStates();
    owner.repaint();
}

void LoopResetPresenter::clearLoopOut()
{
    owner.setLoopOutPosition(owner.getAudioPlayer().getThumbnail().getTotalLength());
    owner.ensureLoopOrder();
    owner.updateLoopButtonColors();
    owner.updateLoopLabels();
    owner.silenceDetector->setIsAutoCutOutActive(false);
    owner.updateComponentStates();
    owner.repaint();
}

