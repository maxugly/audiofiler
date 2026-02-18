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

void CutResetPresenter::resetIn()
{
    owner.setCutInPosition(0.0);
    owner.ensureLoopOrder();
    owner.updateLoopButtonColors();
    owner.updateCutLabels();
    owner.setAutoCutInActive(false);
    owner.repaint();
}

void CutResetPresenter::resetOut()
{
    owner.setCutOutPosition(owner.getAudioPlayer().getThumbnail().getTotalLength());
    owner.ensureLoopOrder();
    owner.updateLoopButtonColors();
    owner.updateCutLabels();
    owner.setAutoCutOutActive(false);
    owner.repaint();
}
