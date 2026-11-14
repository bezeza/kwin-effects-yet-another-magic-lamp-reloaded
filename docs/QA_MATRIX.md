# QA matrix

Status key:

- `AUTOMATED` — covered by a registered pure Qt test; still needs a successful CI run.
- `MANUAL` — requires a nested KWin session or a real desktop.
- `PENDING` — no verified result is recorded yet.

## 1. Build and compatibility

| ID | Scenario | Expected result | Type | Status |
|---|---|---|---|---|
| B-01 | Build against KWin 6.6 | Plugin and KCM compile | Pinned CI / MANUAL | PENDING |
| B-02 | Build against KWin 6.7 | Plugin and KCM compile | Pinned CI / MANUAL | PENDING |
| B-03 | Build against KWin 6.8 | Plugin and KCM compile, bool paintWindow probe selected | Pinned CI / MANUAL | PENDING |
| B-04 | Fedora | Build and all CTest tests pass | CI | PENDING |
| B-05 | Arch Linux | Build and all CTest tests pass | CI | PENDING |
| B-06 | openSUSE Tumbleweed | Build and all CTest tests pass | CI | PENDING |
| B-07 | Ubuntu rolling with KWin >= 6.6 | Build and all CTest tests pass | CI | PENDING |
| B-08 | Ubuntu image without KWin >= 6.6 | Workflow reports a deliberate skip, not a false pass | CI | PENDING |

**CI coverage note:** each distro workflow installs its image's latest KWin
package. Separate KWin 6.6, 6.7, and 6.8 builds are not pinned yet, so B-01–B-03
remain outside the current CI matrix.

## 2. Historical freeze and mesh safety

| ID | Scenario | Expected result | Type | Status |
|---|---|---|---|---|
| F-01 | `GridResolution=0` in kwinrc | Runtime clamp prevents zero reaching `makeGrid()`; no compositor hang | MANUAL | PENDING |
| F-02 | `GridResolution=1` or 15 | Runtime clamp selects 16 | AUTOMATED + MANUAL | PENDING |
| F-03 | `GridResolution=16` | Smallest supported mesh cell size; animation completes | MANUAL | PENDING |
| F-04 | `GridResolution=30` | Default configuration works | MANUAL | PENDING |
| F-05 | `GridResolution=99` | Largest supported mesh cell size works | MANUAL | PENDING |
| F-06 | `GridResolution=100` or unsigned maximum | Runtime clamp selects 99 | AUTOMATED + MANUAL | PENDING |
| F-07 | Historical `GridResolution=40` | No freeze; record GPU/frame behavior and logs | MANUAL | PENDING |

**Do not close the historical freeze investigation from F-01–F-06 alone.** The recorded value was 40, which is positive
and does not trigger the zero-step loop hazard. F-07 must still be tested, and the main incident remains unexplained.

## 3. Geometry and output layout

| ID | Scenario | Expected result | Type | Status |
|---|---|---|---|---|
| G-01 | Single monitor, bottom panel | Animation ends at the correct task button | MANUAL | PENDING |
| G-02 | Two monitors side by side | Target belongs to the window's own output | AUTOMATED + MANUAL | PENDING |
| G-03 | Two monitors stacked vertically | Upper panel is never selected for a lower-output window | AUTOMATED + MANUAL | PENDING |
| G-04 | Negative global output origin | Coordinates remain correct | AUTOMATED + MANUAL | PENDING |
| G-05 | Mixed 100% / 125% / 150% / 200% scale | No offset or size drift | MANUAL | PENDING |
| G-06 | Rotated output (90°/180°/270°) | Adapter and live target remain correct | AUTOMATED + MANUAL | PENDING |
| G-07 | Mirrored/flip output transforms | Mapping matches KWin's OutputTransform | AUTOMATED + MANUAL | PENDING |
| G-08 | Empty, non-finite, or overflowing target rectangle | Animation is skipped safely | AUTOMATED + MANUAL | PENDING |
| G-09 | Transient window with main window task button | Main-window target is selected on the same output | MANUAL | PENDING |
| G-10 | Multiple/grouped task managers | No guessed target; use normal KWin behavior if absent | MANUAL | PENDING |
| G-11 | All eight rotate/flip transforms | Direct point mapping matches KWin OutputTransform | AUTOMATED | PENDING |

## 4. Window, panel, and recovery lifecycle

| ID | Scenario | Expected result | Type | Status |
|---|---|---|---|---|
| L-01 | Minimize with task button already available | Animation starts normally | MANUAL | PENDING |
| L-02 | Minimize before Plasma publishes button geometry | Bounded retry starts; no stuck visible reference | MANUAL | PENDING |
| L-03 | Restore while waiting for initial minimize target | Pending entry is removed and window restores normally | MANUAL | PENDING |
| L-04 | Restore during active animation | Animation reverses from its current progress | MANUAL | PENDING |
| L-05 | Restore during recovery pause | Wait for target, then reverse/resume | Policy + MANUAL | PENDING |
| L-06 | Close a window during retry | Pending visible reference is released | MANUAL | PENDING |
| L-07 | Close a window during active/recovering animation | No stale timer, redirect, or visible reference remains | MANUAL | PENDING |
| L-08 | Close a panel without changing monitor topology | Closing panel target is ignored | AUTOMATED selection + MANUAL | PENDING |
| L-09 | Add a replacement panel | Settling window restarts and new button geometry is selected | AUTOMATED policy + MANUAL | PENDING |
| L-10 | Remove/reconnect monitor | Animation retargets or safely falls back | Policy + MANUAL | PENDING |
| L-11 | Change output mode, scale, rotation, or position | Task-button geometry is re-resolved after layout settles | MANUAL | PENDING |
| L-12 | Fullscreen effect takes over | Pending and active Magic Lamp state is released | AUTOMATED policy + MANUAL | PENDING |
| L-13 | Unload/reload the effect during pending timers | No callback reaches destroyed effect state | MANUAL | PENDING |
| L-14 | Rapid minimize/restore/re-minimize sequence | Stale generations cannot affect the newest attempt | MANUAL | PENDING |
| L-15 | Re-minimize during recovery pause | Retarget current button and resume; no frozen animation | MANUAL | PENDING |
| L-16 | Re-minimize with no button target | Cancel stale animation; wait boundedly or fall back | MANUAL | PENDING |
| L-17 | Start with paused running timeline | Clear pause and reverse; restore may explicitly re-pause | AUTOMATED | PENDING |

## 5. Animation settings and performance

| ID | Scenario | Expected result | Type | Status |
|---|---|---|---|---|
| P-01 | Duration 0, 1, 9999, oversized | Runtime duration stays within 1–9999 ms | AUTOMATED + MANUAL | PENDING |
| P-02 | Bump distance 0, 99, and oversized kwinrc value | Runtime bump distance remains within 0–99 px | AUTOMATED + MANUAL | PENDING |
| P-03 | All four panel directions | No NaN/Inf transform; animation reaches target | AUTOMATED geometry + MANUAL | PENDING |
| P-04 | Large 4K window with grid cell size 16 | No freeze; record frame time and GPU load | MANUAL | PENDING |
| P-05 | Large 4K window with grid cell size 99 | No visual tearing; record frame time and GPU load | MANUAL | PENDING |
| P-06 | Repeated animation over several minutes | No growing timer, animation, redirect, or visible-reference counts | MANUAL | PENDING |
| P-07 | Target geometry in all four panel directions | Bump distance and shape factor remain finite and consistent | AUTOMATED | PENDING |

## 6. Required evidence before release

- Successful Fedora, Arch, openSUSE, and applicable Ubuntu Actions runs.
- CTest summary for every successful build.
- Nested KWin results for hotplug, panel close/add, fullscreen, rapid restore, and effect unload.
- A desktop test on the target Fedora/KDE version.
- Logs and exact configuration for any freeze or incorrect target.
- Explicit confirmation of whether the historical `GridResolution=40` scenario reproduces.

A PR being marked mergeable is not a QA result.
