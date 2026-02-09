#pragma once

#include <JuceHeader.h>

class ControlPanel;

/**
 * @class LoopButtonPresenter
 * @brief Handles the color state of the loop in/out buttons.
 */
class LoopButtonPresenter
{
public:
    explicit LoopButtonPresenter(ControlPanel& ownerPanel);

    void updateColours();

private:
    ControlPanel& owner;
};

