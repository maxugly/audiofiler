import sys

def main():
    try:
        with open('Source/ControlStatePresenter.cpp', 'r') as f:
            lines = f.readlines()
    except FileNotFoundError:
        print("Error: Source/ControlStatePresenter.cpp not found.")
        sys.exit(1)

    new_lines = []

    # Process lines to replace setEnabled(true)
    for line in lines:
        if 'owner.silenceDetector->getInSilenceThresholdEditor().setEnabled(true);' in line:
            new_lines.append(line.replace('true', 'enabled && isCutModeActive'))
        elif 'owner.silenceDetector->getOutSilenceThresholdEditor().setEnabled(true);' in line:
            new_lines.append(line.replace('true', 'enabled && isCutModeActive'))
        else:
            new_lines.append(line)

    # Insert Doxygen comments
    final_lines = []
    for line in new_lines:
        if 'void ControlStatePresenter::refreshStates()' in line:
            final_lines.append('/**\n * @brief Refreshes the enabled/visible state of all controls based on current app state.\n * \n * Checks if a file is loaded and if cut mode is active, then delegates to specific helpers.\n */\n')
        elif 'void ControlStatePresenter::updateGeneralButtonStates(bool enabled)' in line:
             final_lines.append('\n/**\n * @brief Updates the state of general transport and mode buttons.\n * @param enabled True if an audio file is loaded.\n */\n')
        elif 'void ControlStatePresenter::updateCutModeControlStates(bool isCutModeActive, bool enabled)' in line:
             final_lines.append('\n/**\n * @brief Updates the state of cut-mode specific controls (looping, silence detection).\n * @param isCutModeActive True if the UI is in Cut Mode.\n * @param enabled True if an audio file is loaded.\n */\n')

        final_lines.append(line)

    with open('Source/ControlStatePresenter.cpp', 'w') as f:
        f.writelines(final_lines)

if __name__ == '__main__':
    main()
