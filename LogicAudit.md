# Logic Audit: Zoom Focus & Button State

## Table 1: The Zoom Focus Map

| Trigger Event | Target Coordinate | Function Reference | Gatekeeper (The "If") |
| :--- | :--- | :--- | :--- |
| **Drag Handle (In)** | `loopInPosition` | `WaveformRenderer::drawZoomPopup` (Priority 1) | `mouseHandler.getDraggedHandle() == In` |
| **Drag Handle (Out)** | `loopOutPosition` | `WaveformRenderer::drawZoomPopup` (Priority 1) | `mouseHandler.getDraggedHandle() == Out` |
| **Placement Mode (In)** | `loopInPosition` | `ControlButtonsPresenter::initialiseLoopButtons` (via `mouseHandler.setCurrentPlacementMode`) | Right-Click Loop Button |
| **Placement Mode (Out)** | `loopOutPosition` | `ControlButtonsPresenter::initialiseLoopButtons` (via `mouseHandler.setCurrentPlacementMode`) | Right-Click Loop Button |
| **Text Editor Hover (In)** | `ActiveZoomPoint::In` | `LoopPresenter::mouseEnter` | `!owner.isZKeyDown()` |
| **Text Editor Hover (Out)** | `ActiveZoomPoint::Out` | `LoopPresenter::mouseEnter` | `!owner.isZKeyDown()` |
| **Text Editor Focus (In)** | `ActiveZoomPoint::In` | `LoopPresenter::textEditorFocusGained` | None |
| **Text Editor Focus (Out)** | `ActiveZoomPoint::Out` | `LoopPresenter::textEditorFocusGained` | None |
| **Text Editor Blur** | `ActiveZoomPoint::None` | `LoopPresenter::textEditorFocusLost` | None |
| **Hover Exit** | `ActiveZoomPoint::None` | `LoopPresenter::mouseExit` | `!editor.hasKeyboardFocus(false)` |
| **Playback (Default)** | `transportSource.getCurrentPosition()` | `WaveformRenderer::drawZoomPopup` (Priority 3) | `ActiveZoomPoint == None` AND `draggedHandle == None` |
| **Z Key Hold** | Force Show Popup | `WaveformRenderer::drawZoomPopup` | `controlPanel.isZKeyDown()` |

## Table 2: The Button State Matrix

| Button | State | Function Reference | Condition for "Enabled" |
| :--- | :--- | :--- | :--- |
| **[Cut]** | Always Enabled | `ControlStatePresenter::updateGeneralButtonStates` | `true` (Always clickable, toggles mode) |
| **[AC In]** | Conditional | `ControlStatePresenter::updateCutModeControlStates` | `isCutModeActive` |
| **[AC Out]** | Conditional | `ControlStatePresenter::updateCutModeControlStates` | `isCutModeActive` |
| **[Loop]** | Always Enabled | `ControlStatePresenter::updateGeneralButtonStates` | `true` (Always clickable, toggles looping) |
| **[Loop In]** | Conditional | `ControlStatePresenter::updateCutModeControlStates` | `totalLength > 0` AND `isCutModeActive` |
| **[Loop Out]** | Conditional | `ControlStatePresenter::updateCutModeControlStates` | `totalLength > 0` AND `isCutModeActive` |
| **[Clear In]** | Conditional | `ControlStatePresenter::updateCutModeControlStates` | `totalLength > 0` AND `isCutModeActive` |
| **[Clear Out]** | Conditional | `ControlStatePresenter::updateCutModeControlStates` | `totalLength > 0` AND `isCutModeActive` |

## State Conflict Deep Dive

### Scenario 1: The "Hover vs. Focus" Disconnect
**The Conflict:** A user clicks into the "Loop In" text box to edit it (Keyboard Focus -> Zoom Focus: In). Then, they move the mouse to check a value near the "Loop Out" text box without clicking.
**Result:** `LoopPresenter::mouseEnter` fires for "Loop Out". It sets `ActiveZoomPoint` to **Out**.
**The Problem:** The user is typing into the **In** field, but the Zoom Popup is showing the **Out** point. The visual feedback (Zoom) contradicts the active input channel (Keyboard).
**Fragility Check:** `LoopPresenter::mouseEnter` does not check if *another* editor currently has focus before hijacking the Zoom Point.

### Scenario 2: The "Drag vs. Hover" Race
**The Conflict:** A user is dragging the "Loop In" handle on the waveform. In doing so, their mouse cursor accidentally crosses over the "Loop Out" text editor box.
**Result:** `LoopPresenter::mouseEnter` fires and sets `ActiveZoomPoint` to **Out**.
**Resolution:** `WaveformRenderer` saves the day by checking `draggedHandle` (Priority 1) before `ActiveZoomPoint` (Priority 2).
**Fragility Check:** While the *visual* result is correct (Zoom stays on drag), the *underlying state* `ActiveZoomPoint` is being thrashed in the background. If any other logic depended on `ActiveZoomPoint` (e.g., a "Jump to Point" command), it would be incorrect.

### Scenario 3: The "Z-Key" Override
**The Conflict:** The user holds 'Z' to inspect the waveform. While holding 'Z', they hover over a text editor.
**Result:** `LoopPresenter::mouseEnter` explicitly checks `!owner.isZKeyDown()` and *refuses* to update the Zoom Point.
**The Problem:** This is a hard-coded exception in `LoopPresenter`. If we add a new way to trigger zoom (e.g., a new "Inspect Mode"), we have to update this check manually. It's a "Negative Gatekeeper" that makes the system brittle.

### Ghost State Identification
1.  **`currentPlacementMode` (MouseHandler):** Stores whether we are placing an In or Out point. Acts as a "modal" state that overrides normal drag behavior.
2.  **`dragStartMouseOffset` (MouseHandler):** Implicit state maintained during a drag. If a drag is interrupted (e.g., by a popup), this state might become stale.
3.  **`isZKeyDown` (ControlPanel):** A global modifier state that `LoopPresenter` peeks at to decide its own behavior. This couples the Presenter to the Panel's raw input state.
