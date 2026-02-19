/**
 * @file CutResetPresenter.cpp
 * @brief Defines the CutResetPresenter class.
 * @ingroup Presenters
 */

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
    owner.ensureCutOrder();
    owner.updateCutButtonColors();
    owner.updateCutLabels();
    owner.setAutoCutInActive(false);
    owner.repaint();
}

void CutResetPresenter::resetOut()
{
    owner.setCutOutPosition(owner.getAudioPlayer().getThumbnail().getTotalLength());
    owner.ensureCutOrder();
    owner.updateCutButtonColors();
    owner.updateCutLabels();
    owner.setAutoCutOutActive(false);
    owner.repaint();
}
