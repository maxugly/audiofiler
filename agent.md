Agent.md: Refactoring and Development Practices for Sorta++
Introduction
This document outlines the core principles, practices, and workflows for refactoring and developing the Sorta++ JUCE-based audio application. Our approach is patient, iterative, and focused on real-world success — treating the codebase like "Lego bricks and wires" for easy understanding, maintenance, and expansion. The goal is a perfectly maintainable, coherent, best-practice C++/JUCE project that's team-friendly, extensible, and tuned for LLM assistance (e.g., short functions, descriptive names, explicit comments).
These practices draw from our conversations and refactoring journey, emphasizing modularity, testing, and gradual change to avoid bugs or "shooting ourselves in the foot."
Core Theory: Separation of Concerns
What is Separation of Concerns?
Separation of Concerns (SoC) is a fundamental software design principle that advocates dividing a program into distinct sections, each addressing a separate "concern" or responsibility. In Sorta++, this means breaking down the monolithic MainComponent (originally a "god class" handling UI, audio, events, states, etc.) into small, single-responsibility classes.

Why SoC?
Improves readability: Each file/class focuses on one thing (e.g., AudioPlayer for playback, MouseHandler for input).
Enhances maintainability: Changes in one area (e.g., adding a button) don't ripple everywhere.
Boosts testability: Small classes are easier to unit-test (e.g., mock AudioPlayer to test SilenceDetector).
Enables extensibility: New features plug in like Lego (e.g., add a PluginManager without touching UI).
LLM-friendly: Short files (<500 lines) with clear comments make AI tools (like Gemini or Jules) more effective for generation/refactoring.

How we apply SoC in Sorta++:
Single Responsibility Principle (SRP): Each class does one job (e.g., WaveformRenderer only renders visuals, KeybindHandler only handles keys).
Ownership & Delegation: Parent classes (e.g., ControlPanel) own child classes via std::unique_ptr and delegate (e.g., controlPanel->mouseHandler->mouseDown(event)).
Interfaces/Refs: Use references for access (e.g., MouseHandler takes ControlPanel& ref to get bounds, avoiding tight coupling).
Granularity: We went "super granular" (e.g., PlaybackCursorGlow for one visual effect) to make pieces like Lego — easy for a noob to see/rewire.


Example: Original MainComponent mixed audio + UI + events. Now: AudioPlayer for sound, ControlPanel for widgets, MouseHandler for input — each a brick connected by wires (methods/refs).
Bringing New Features and Code In Properly
New features must integrate without disrupting modularity. Follow this process to "bring in new code properly":

Plan the Feature as Lego:
Identify concerns: Does it touch audio (extend AudioPlayer)? UI (add to ButtonManager)? Both (new class bridging them)?
Design small: Break into 1-2 new classes/files (e.g., for "export audio", add AudioExporter owned by AudioPlayer).
Use existing hooks: Delegate via owners (e.g., add method to ControlPanel that calls new class).

One Small Addition Per Session:
Similar to refactoring: Add one class/method at a time.
Prompt LLM (e.g., Jules): "Add export feature to AudioPlayer as new method exportLoopToFile(File outFile). Preserve modularity, use Config for formats."
No big bangs — avoid touching multiple files unless necessary.

Integrate with Existing Practices:
Config.h First: Add configurable params (e.g., constexpr juce::String exportFormat = ".wav";).
Doxygen from Day 1: Document with @brief/@param/"why" comments.
Modern C++: Use smart pointers, const, RAII; avoid globals.
Delegation: Wire via refs/owners (e.g., MainComponent delegates to ControlPanel->exportButtonClicked()).
No Monoliths: If feature spans concerns, create a coordinator class (e.g., ExportCoordinator owns UI + audio logic).

Build/Test Cycle:
After code: cmake -G Ninja -B build then ninja -C build.
Test manually: Run app, verify feature (e.g., export button saves file).
Add unit test: Simple one for new class (e.g., mock AudioBuffer, assert exported file exists).
If bugs: Use Git reset; fix iteratively.

Edge Cases & Errors:
Handle failures (e.g., export fails → log to statsDisplay with errorColor).
Preserve states (e.g., new feature doesn't reset loop positions).


Example: Adding "undo loop changes" — Create LoopHistory.h/cpp (stores position stacks), owned by LoopPresenter. Add methods like pushState(), undo(); wire to new button in ButtonManager.
All Our Specific Practices
Refactoring Workflow

One Extraction/Addition Per Session: Small steps (e.g., extract mouse to MouseHandler) to avoid LLM loops/overwhelm.
Prompt Structure: Specific, with guidelines (Doxygen, Config, modern C++), context snippets, removal emphasis, output format.
Build/Test After Each: cmake -G Ninja -B build; ninja -C build; manual UI test (e.g., "test buttons/loops"); await results before next.
Fallbacks: If LLM replace/insert fails, use Python (e.g., script for string ops in files).
Git Safety: Commit WIP before changes; reset if broken.

Code Style & Best Practices

Doxygen Comments: @file/@class/@brief/@param/@return everywhere; "why" inlines (intent over what).
Config.h Central: All tweakables here (texts, colors, thresholds) — no hardcodes.
Modern C++: Smart pointers (unique_ptr for ownership), const-correctness, short functions (<50 lines), RAII.
Modularity: Single-responsibility classes; delegation over inheritance; refs for access.
Testing: Unit test ideas per extraction (e.g., mock for SilenceDetector); aim 80% coverage with gcov.
Tools: Emerge/CodeCharta for visualization; Doxygen for docs; counter.py for line counts.
No New Features During Refactor: Only modularize existing; add later.
Granularity: "Super granular" like Lego — small classes for one job (e.g., PlaybackCursorGlow).