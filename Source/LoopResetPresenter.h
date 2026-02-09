#pragma once

class ControlPanel;

/**
 * @class LoopResetPresenter
 * @brief Handles clear-loop button behavior, keeping ControlPanel lean.
 */
class LoopResetPresenter
{
public:
    explicit LoopResetPresenter(ControlPanel& ownerPanel);

    void clearLoopIn();
    void clearLoopOut();

private:
    ControlPanel& owner;
};

