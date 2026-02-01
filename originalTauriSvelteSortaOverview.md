## Application Overview

The application, "Sorta," is a desktop utility designed for the rapid, manual sorting of audio files. The user selects a source directory containing audio files. The application then loads one file at a time, displaying its waveform and allowing the user to play it. Based on the audio content, the user can sort the file into one of several pre-defined subdirectories using single-key shortcuts. The core workflow is optimized for speed and keyboard-based operation, enabling a user to quickly categorize a large queue of files. The application also includes features for customizing the user interface, such as theming and zoom controls.

## Functionality Checklist

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
- [ ] **Playback Behavior**:
    - [ ] **Auto-Advance**: After a file is sorted or skipped, automatically load and play the next file in the queue.
    - [ ] **Repeat Mode**: When enabled, the current file will loop and play again upon finishing instead of being paused. Can be toggled on/off.
- [ ] **Seek/Scrub**: User can skip forward/backward in the current audio track in 5-second increments.
- [ ] **Codec Display**: Show the file extension/codec of the current audio file.

### User Interface & Interaction
- [ ] **File Queue Display**: List all files remaining in the queue.
- [ ] **Remaining Count**: Display a live count of how many files are left to be sorted.
- [ ] **Dynamic Tagline**: Display a random tagline in the header on mouseover.
- [ ] **Keyboard Shortcuts**: The application is primarily controlled via the keyboard:
    - [ ] **`1`-`9`, `0`**: Sort the current file into the corresponding output directory.
    - [ ] **`Spacebar`**: Play/Pause audio.
    - [ ] **`ArrowLeft` / `ArrowRight`**: Seek backward/forward.
    - [ ] **`N` or `S`**: Skip the current file.
    - [ ] **`O`**: Open the source directory selection dialog.
    - [ ] **`R`**: Toggle Repeat mode.
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
    - [ ] Controlled by buttons or `+` / `-` keys.
- [ ] **Waveform Height**: Adjust the vertical height of the waveform display using a slider control or `[` / `]` keys.



Put Another Way
Project Vision: Sorta++ (The Data Hoarder’s Scalpel)
Core Identity: Sorta++ is a high-performance, multi-threaded audio management and "triage" utility built in C++20 using the JUCE framework. It is designed for "Data Hoarders"—users who need to rapidly sort, preview, and non-destructively edit massive directories of audio files with zero friction.

The "Sloppy" Brand Philosophy: The UI rejects "corporate clean" for a "procrastinator/hacker" aesthetic.

Procedural UI: Utilizes a custom DrunkDrawnLookAndFeel where every button and panel is drawn with randomized "wobble" (procedural Bézier paths) and slight rotations.

Cyberpunk Palette: Black backgrounds, Deep Pink waveforms, and High-Contrast Cyan text/outlines.

One-Handed Workflow: All primary sorting (Bins 0-9), navigation (Arrow Keys), and views are bound to keyboard shortcuts for a "mouse-optional" experience.

Technical Architecture:

The Assembly Line (Threading):

A background worker pool pre-calculates thumbnails, detects silence (Auto-In/Out points), and finds peaks (Normalization) for files ahead of the current playhead.

RAM Cache: A user-defined memory limit (e.g., 10MB) to pre-load upcoming files for instantaneous "rapid-fire" playback.

Smart Sorting:

Move/Copy Modes: Support for session-based "Skip" lists and atomic file operations.

FileSystem Monitoring: Real-time tracking of folder changes using a watcher thread.

The "Lazy Editor":

Threshold-based silence removal with Snap-to-Zero-Crossing logic to prevent pops.

Non-destructive exports (defaulting to _chopped suffixes).

Data Persistence:

Integration with SQLite to store file hashes (fingerprints), BPM, Key, and user-defined tags, ensuring metadata survives even if files are moved.

Current Stack:

Language: C++20

Framework: JUCE (AudioAppComponent, ListBox, AudioThumbnail)

Build System: CMake (utilizing CPM for dependency management)

Diagnostic Tools: Melatonin Inspector (for UI debugging)

The Objective: To move beyond a simple player and create an "Intelligence Engine" that assists the user in organizing chaos, turning a library of 10,000 messy samples into a curated, tagged, and "chopped" professional database.
