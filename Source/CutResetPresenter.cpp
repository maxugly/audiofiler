#include "CutResetPresenter.h"

#include "ControlPanel.h"
#include "ControlPanelCopy.h"
#include "Config.h"
#include "SilenceDetector.h"
#include "AudioPlayer.h"

CutResetPresenter::CutResetPresenter(ControlPanel& ownerPanel)
    : owner(ownerPanel)
{
}

void CutResetPresenter::clearLoopIn()
{
    owner.setLoopInPosition(0.0);
    owner.ensureLoopOrder();
    owner.updateLoopButtonColors();
    owner.updateLoopLabels();
    owner.setAutoCutInActive(false);
    owner.repaint();
}

void CutResetPresenter::clearLoopOut()
{
    owner.setLoopOutPosition(owner.getAudioPlayer().getThumbnail().getTotalLength());
    owner.ensureLoopOrder();
    owner.updateLoopButtonColors();
    owner.updateLoopLabels();
    owner.setAutoCutOutActive(false);
    owner.repaint();
}
