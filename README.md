# AUDIOFILER Engineering Specification
### The Single Authority for All Contributors — Human, LLM, or Otherwise

> **v3.0 — Living Document**
> Before writing a single line of code, any contributor — regardless of type — must answer YES to these three questions:
> 1. Does my change comply with the Four Laws (Section I)?
> 2. Does my change pass the Done-State tests (Section II)?
> 3. Does my change keep file sizes within the size budget (Section III)?
>
> A single **NO** is a blocker. Stop and refactor first.

---

## Table of Contents

1. [The Four Architectural Laws](#section-i--the-four-architectural-laws)
2. [Done-State Contracts](#section-ii--done-state-contracts)
3. [File Size Budget](#section-iii--file-size-budget)
4. [Operational Constraints](#section-iv--operational-constraints)
5. [Modularity and Portability](#section-v--modularity-and-portability)
6. [Component Ownership Map](#section-vi--component-ownership-map)
7. [Documentation Requirements](#section-vii--documentation-requirements)
8. [Contributor Guardrails](#section-viii--contributor-guardrails)
9. [Authoritative Terminology](#section-ix--authoritative-terminology)
10. [Version History](#section-x--version-history)

---

## Section I — The Four Architectural Laws

Every proposed change must be validated against these four laws. They are non-negotiable. Any code that violates them is incorrect by definition, regardless of whether it compiles or runs.

---

### Law 1 — SoC: Separation of Concerns

| | |
|---|---|
| **The Law** | Divide the software into distinct sections where each section addresses a single concern. |
| **Audio** | All file-reading and playback logic lives exclusively in `AudioPlayer`. |
| **State** | All settings and cross-file metadata live exclusively in `SessionState`. |
| **Layout** | All component positioning lives exclusively in `LayoutManager` or `resized()` methods. |
| **Study Value** | A student must be able to isolate the Audio Engine and read it without UI noise. |

---

### Law 2 — SRP: Single Responsibility Principle

| | |
|---|---|
| **The Law** | A class has one, and only one, reason to change. |
| **Example (Good)** | `WaveformView` only renders pixels. It does not calculate the sample-to-pixel ratio. |
| **Example (Good)** | `CoordinateMapper` owns all sample-to-pixel math. Nothing else touches it. |
| **Size Signal** | If a file exceeds 400–500 lines, it has likely violated SRP and must be decomposed. |
| **The Test** | Can you describe what the file does without using the word "and"? If not, split it. |

---

### Law 3 — MVP: Model-View-Presenter

| | |
|---|---|
| **The Law** | Decouple data (Model) from representation (View) via a middleman (Presenter). |
| **Model** | `SessionState` and `FileMetadata`. They have zero knowledge that a UI exists. |
| **View** | Dumb JUCE components (`WaveformView`, `ZoomView`). Only `paint()` and `resized()`. No business logic. |
| **Presenter** | Logic bricks (`TransportPresenter`, `CutPresenter`). The glue between Model and View. |
| **Peer Rule** | Presenters must be peers. One Presenter must never own another. They communicate via `SessionState` or shared Listeners only. |
| **God Object Rule** | If a header has more than 10 `std::unique_ptr` sub-components, decompose it into peer groups. |

---

### Law 4 — DRY: Don't Repeat Yourself

| | |
|---|---|
| **The Law** | Every piece of knowledge has a single, unambiguous representation in the system. |
| **Colors** | All hex color values live in `Config.h`. Never hardcode a color in a `.cpp` file. |
| **Dimensions** | All pixel widths and UI constants live in `Config.h` or `ControlPanelCopy.h`. |
| **Strings** | All user-facing UI strings are centralized. Never write a raw string literal in a component. |

---

## Section II — Done-State Contracts

Each major feature area has a binding contract. A feature is not "done" until it passes all three checks: **Goal**, **Test**, and **Done State**. Passing one or two is not sufficient.

---

### Contract 1 — MVP Structural Migration

| | |
|---|---|
| **Goal** | Move from Stacked Hierarchy to Peer-to-Peer communication. `ControlPanel.h` must contain only layout code. |
| **Test** | Can you delete `ControlPanel` and run `WaveformView` in a standalone window? If the answer is **No**, the work is not done. |
| **Done State** | Every UI feature (Transport, Cuts, Repeat) has a dedicated Presenter (the Brain) and a Passive View (the Display). All `juce::TextButton` and `juce::TextEditor` members are moved out of `ControlPanel.h` into their respective Presenter-managed View groups. |

---

### Contract 2 — Total Naming Purity (The No-Loop Zone)

| | |
|---|---|
| **Goal** | The word "Loop" must not appear in the source code except where it refers to a literal programmatic loop (e.g., a `for` loop or `while` loop). |
| **Test** | A case-insensitive global search for `Loop` in the `Source/` folder returns zero results. **Zero means zero.** |
| **Cuts** | Use: Boundaries, Regions, Metadata, Cut Mode. |
| **Repeat** | Use: Repeat, Repeat Enabled flag, Playback Cycling, Transport Button. |
| **Done State** | Both terminology domains above are fully enforced with no legacy "Loop" references remaining. |

---

### Contract 3 — Metadata Resource Pool

| | |
|---|---|
| **Goal** | `SessionState` is a high-speed Data Hub that serves multiple files, not only the currently active one. |
| **Test** | Can you assign silence points to a file that is not currently playing? If the answer is **No**, the work is not done. |
| **Done State** | The `metadataCache` map is the Single Source of Truth. The background worker writes to this map. The `CutPresenter` reads from it. No other path exists. |

---

### Contract 4 — Thread-Safe Air Gaps

| | |
|---|---|
| **Goal** | Zero direct contact between the Background Worker and any UI component. |
| **Test** | Load a directory of 100 files. If the app crashes or freezes, the contract is violated. |
| **Done State** | The `SilenceAnalysisWorker` communicates exclusively via `juce::MessageManager::callAsync`. It never holds a UI component pointer. It never touches the `AudioPlayer` mutex. It creates its own private `AudioFormatReader` for each file. |

---

## Section III — File Size Budget

File length is a proxy for responsibility. Long files hide bugs and signal architectural violations. Every file must remain within its budget. Exceeding the budget is a **refactor trigger**, not a warning.

| File | Current | Target | Refactor Action |
|---|---|---|---|
| `ControlPanel.cpp` | 579 lines | < 150 lines | Must delegate 100% of logic to sub-presenters. Parent is layout only. |
| `MouseHandler.cpp` | 497 lines | Split into 2+ | Separate into `MarkerMouseHandler` and `WaveformMouseHandler`. One type of interaction per file. |
| `RepeatPresenter.cpp` | 417 lines | < 200 lines | Extract time-string formatting and coordinate math into `PlaybackUtils` or `MathUtils`. |
| Any Presenter | 400–500+ lines | < 300 lines | Offload helper math/utilities to a dedicated Utils file. |
| Any View | > 150 lines | < 100 lines | Views contain only `paint()` and `resized()`. Move all logic upstream. |

---

## Section IV — Operational Constraints

### ⛔ The Observer Law — No Polling

| | |
|---|---|
| **Forbidden** | Using `timerCallback()` to check whether a state value has changed. This is polling and is strictly prohibited. |
| **Required** | Use the Observer Pattern. All components that need to react to state changes must implement the `SessionState::Listener` interface and register themselves. |
| **Broadcast Flow** | User interacts with View → Presenter handles event → Presenter updates `SessionState` → `SessionState` broadcasts message → `AudioPlayer` and all Listeners react instantly. |

---

### ⛔ The Threading Law — Anti-Freeze Protocol

| | |
|---|---|
| **Private Reader** | `SilenceAnalysisWorker` must instantiate its own independent `AudioFormatReader`. It must never use or touch the `AudioPlayer` reader. |
| **Mutex Avoidance** | The background worker must not acquire or contend with any mutex held by `AudioPlayer`. |
| **Async Return** | When background work completes, results are pushed to the Message Thread via `juce::MessageManager::callAsync` only. Never block the UI thread. |
| **Crash Test** | If the app crashes while loading a directory of 100 files, the threading contract is violated. |

---

## Section V — Modularity and Portability

### The Lego Standard

Every feature is a self-contained brick. It can be wired in or out without disturbing unrelated systems. Violating this rule means the codebase is becoming a monolith.

| | |
|---|---|
| **New Feature Rule** | Plan every new feature as a separate class before integrating it into any parent owner. |
| **Ownership Rule** | Parent components own children via `std::unique_ptr` only. Raw pointer ownership is not permitted. |
| **Independence Test** | Can this brick be compiled and tested in isolation? If **No**, it is too tightly coupled. |

---

### Headless Portability

The engine must be fully testable without a GUI. The automated test suite (`tests` target) runs in a terminal-only environment. Any code that cannot run headless will break the test pipeline.

| | |
|---|---|
| **Guard Macro** | Wrap all GUI-dependent code in `#if !defined(JUCE_HEADLESS)` guards. |
| **Test Target** | The `tests` build target must compile cleanly with `JUCE_HEADLESS` defined. |
| **Logic Purity** | Core logic (`SessionState`, `AudioPlayer`, `SilenceAnalysisWorker`) must have zero GUI dependencies. |

---

## Section VI — Component Ownership Map

The application follows a strict top-down ownership hierarchy. This determines construction order, destruction order, and lifetime guarantees.

| Component | Role | Responsibilities |
|---|---|---|
| `MainComponent` | Root Shell | Application entry point. Owns and constructs all top-level components. No business logic. |
| `SessionState` | The Brain | Holds all settings, preferences, and cross-file metadata. Acts as the Radio Station (broadcast source). No UI knowledge. |
| `AudioPlayer` | Audio Engine | Strictly handles file reading and playback. Listens to `SessionState`. Has zero knowledge of UI components. |
| `ControlPanel` | Visual Container | Acts only as a parent shell for sub-views and presenters. Contains layout code only. Target: < 150 lines. |
| Presenters | Logic Bricks | Manage specific UI behaviour (e.g., `CutPresenter`, `TransportPresenter`, `StatsPresenter`). Glue between Model and View. Must be peers — never hierarchical. |
| Views | Display Canvas | Dumb JUCE components. Only implement `paint()` and `resized()`. Zero business logic, zero math. |

---

## Section VII — Documentation Requirements

Every commit must satisfy the Doxygen parser. These requirements maintain the **Interactive Study Map** — a living dependency graph of the entire codebase.

| Tag | Requirement |
|---|---|
| `@file` | Required at the top of every header file. Enables Directory Graphs in the study map. |
| `@ingroup` | Must assign the class to a logical module. Valid groups: `Engine`, `UI`, `State`, `Utils`, `Threading`. |
| `@see` | Mandatory for cross-referencing peer Presenters or related Models. Every Presenter must `@see` its paired View and its data source. |
| **Enforcement** | A class submitted without these three tags is considered incomplete and must be updated before merge. |

---

## Section VIII — Contributor Guardrails

Pre-commit checklist. Run through every item before submitting or finalising any change. A single **NO** is a blocker.

- [ ] Can I describe what this file does without using the word "and"?
- [ ] Does this file stay within its line-count budget?
- [ ] Is any logic inside a `paint()` or `resized()` method? *(Must be No)*
- [ ] Is any hex color or pixel width hardcoded in a `.cpp` file? *(Must be No)*
- [ ] Does any `timerCallback()` poll for a state change? *(Must be No)*
- [ ] Does the word "Loop" appear anywhere in `Source/` (case-insensitive)? *(Must be No)*
- [ ] Does the background worker touch any UI pointer directly? *(Must be No)*
- [ ] Does every new class include `@file`, `@ingroup`, and `@see` Doxygen tags?
- [ ] Do all Presenters communicate via `SessionState` rather than direct references?
- [ ] Is every new feature class wired in via `std::unique_ptr`?

---

## Section IX — Authoritative Terminology

Use only the terms in the **Correct** column. If you encounter legacy naming, update it as part of your change. There is no grace period for deprecated terms.

| ~~Deprecated~~ | **Correct** | Context |
|---|---|---|
| ~~Loop~~ | Boundary / Region / Cut | Describing a marked audio segment |
| ~~Loop Mode~~ | Cut Mode | The mode for setting region boundaries |
| ~~Loop Point~~ | Boundary | A start or end point of a region |
| ~~Loop Enabled~~ | Repeat Enabled | The playback cycling flag |
| ~~Loop Button~~ | Repeat (transport button) | The UI control for cycling playback |
| ~~Loop back~~ | Cycle / Repeat | The act of playback returning to start |

---

## Section X — Version History

This is a living document. Append entries here when architectural decisions are made, constraints change, or new patterns are adopted. **Never remove prior entries** — mark them superseded if needed.

| Version | Date | Changes |
|---|---|---|
| v1.0 | — | Initial SoC, SRP, MVP, DRY laws established. |
| v2.1 | — | Architectural Map formalised. Peer-to-Peer Presenter rule added. |
| v2.2 | — | Engineering Specification (NERD edition) published. Doxygen requirements added. |
| v3.0 | 2025-02-19 | Unified into single authoritative document. Done-State Contracts, Size Budget, Naming Purity, Threading, Modularity, Terminology, and Guardrail Checklist consolidated. |

---

*Audiofiler Engineering Specification • v3.0 • Living Document — Append, Never Delete*
