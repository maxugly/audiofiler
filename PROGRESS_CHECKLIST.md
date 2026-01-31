## Sorta++ Progress Checklist

This document tracks the implementation status of features for Sorta++, based on the original project overview.

### Core Sorting & File Management
- [ ] **Source Directory Selection**: User can select a single source directory using a native folder picker dialog.
- [ ] **Audio File Discovery**: Automatically scan the selected source directory for audio files with specific extensions (`.mp3`, `.wav`, `.flac`, `.ogg`, `.m4a`, `.wma`).
- [ ] **File Queue**: Maintain an in-memory queue of discovered audio files to be sorted.
- [ ] **Output Directories**: User can define a space-separated list of subdirectory names where files will be sorted.
- [ ] **Sorting Action**: For the currently active file, the user can trigger a sort action into any of the defined output directories.
- [ ] **Sort Modes**:
    - [ ] **Move Mode**: The sorted file is moved from the source directory to the target subdirectory. This is the default mode.
    - [ ] **Copy Mode**: The sorted file is copied to the target subdirectory, leaving the original file in place.
- [ ] **Skip File**: User can skip the current file, removing it from the current session's queue without moving or copying it.
- [ ] **File System Monitoring**:
    - [ ] The application can watch the source directory for changes.
    - [ ] If files are added or removed, the queue automatically refreshes after a short delay.
    - [ ] This feature can be toggled on or off by the user.

### Audio Playback & Visualization
- [ ] **Waveform Generation**: Display a visual waveform for the current audio file using WaveSurfer.js.
- [ ] **Playback Controls**:
    - [ ] Play/Pause the current audio file.
    - [ ] The audio automatically plays when a new file is loaded.
- [x] **Playback Behavior**:
    - [ ] **Auto-Advance**: After a file is sorted or skipped, automatically load and play the next file in the queue.
    - [x] **Repeat Mode**: When enabled, the current file will loop and play again upon finishing instead of being paused. Can be toggled on/off.
    - [x] **Loop In/Out Points**: User can define specific 'in' and 'out' points within the audio for looping a segment.
- [ ] **Seek/Scrub**: User can skip forward/backward in the current audio track in 5-second increments.
- [x] **Visual Cue - Mouse Seek Line**: A vertical line follows the mouse cursor over the waveform.
- [x] **Visual Cue - Loop Selection**: Lines and translucent shade indicate the selected loop region.
- [ ] **Codec Display**: Show the file extension/codec of the current audio file.

### User Interface & Interaction
- [ ] **File Queue Display**: List all files remaining in the queue.
- [ ] **Remaining Count**: Display a live count of how many files are left to be sorted.
- [ ] **Dynamic Tagline**: Display a random tagline in the header on mouseover.
- [x] **Keyboard Shortcuts**: The application is primarily controlled via the keyboard:
    - [ ] **`1`-`9`, `0`**: Sort the current file into the corresponding output directory.
    - [ ] **`Spacebar`**: Play/Pause audio.
    - [ ] **`ArrowLeft` / `ArrowRight`**: Seek backward/forward.
    - [ ] **`N` or `S`**: Skip the current file.
    - [x] **`D`**: Open the source directory selection dialog. (Originally 'O')
    - [x] **`L`**: Toggle Loop (Repeat) mode.
    - [x] **`I`**: Set Loop In point.
    - [x] **`O`**: Set Loop Out point.
    - [ ] **`R`**: Toggle Repeat mode. (Replaced by `L` for Loop)
    - [ ] **`F`**: Toggle Filesystem monitoring.
    - [ ] **`M`**: Toggle between Move and Copy sort modes.
    - [ ] **`Q`**: Quit the application.
- [ ] **Exit Button**: A dedicated UI button to close the application.

### Customization & Theming
- [ ] **UI Theming**:
    - [ ] Cycle through a predefined list of color themes (`blue`, `red`, `green`, etc.).
    - [ ] The selected theme is applied globally to the UI.
    - [ ] The chosen theme is persisted in local storage and loaded on startup.
- [ ] **UI Zoom**:
    - [ ] Increase or decrease the overall size of the UI elements.
    - [ ] Controlled by buttons or `+` / `]` keys.
- [ ] **Waveform Height**: Adjust the vertical height of the waveform display using a slider control or `[` / `]` keys.
