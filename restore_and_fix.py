import sys

def main():
    with open('Source/ControlStatePresenter.cpp', 'r') as f:
        lines = f.readlines()

    # Filter out lines that look like the comments I added
    filtered_lines = []
    skip = False
    for line in lines:
        if line.strip() == '/**':
            skip = True
        if skip:
            if '*/' in line:
                skip = False
            continue
        filtered_lines.append(line)

    # Now apply the transformation again (single pass)
    final_lines = []
    for line in filtered_lines:
        if 'void ControlStatePresenter::refreshStates()' in line:
            final_lines.append('/**\n * @brief Refreshes the enabled/visible state of all controls based on current app state.\n * \n * Checks if a file is loaded and if cut mode is active, then delegates to specific helpers.\n */\n')
        elif 'void ControlStatePresenter::updateGeneralButtonStates(bool enabled)' in line:
             final_lines.append('\n/**\n * @brief Updates the state of general transport and mode buttons.\n * @param enabled True if an audio file is loaded.\n */\n')
        elif 'void ControlStatePresenter::updateCutModeControlStates(bool isCutModeActive, bool enabled)' in line:
             final_lines.append('\n/**\n * @brief Updates the state of cut-mode specific controls (looping, silence detection).\n * @param isCutModeActive True if the UI is in Cut Mode.\n * @param enabled True if an audio file is loaded.\n */\n')

        # Ensure the setEnabled change is correct (it might have been done already)
        if 'owner.silenceDetector->getInSilenceThresholdEditor().setEnabled(true);' in line:
            final_lines.append(line.replace('true', 'enabled && isCutModeActive'))
        elif 'owner.silenceDetector->getOutSilenceThresholdEditor().setEnabled(true);' in line:
            final_lines.append(line.replace('true', 'enabled && isCutModeActive'))
        else:
            final_lines.append(line)

    with open('Source/ControlStatePresenter.cpp', 'w') as f:
        f.writelines(final_lines)

if __name__ == '__main__':
    main()
