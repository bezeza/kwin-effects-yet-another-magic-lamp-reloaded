# Incident note: Fedora KDE freeze and nested KWin crashes

Date of original incident: 3 November 2026

## Executive conclusion

The evidence describes **two separate incidents**. They must not be merged into one explanation.

1. The main Fedora KDE desktop stopped responding around 14:02 EET. Its root cause remains unknown.
2. Six nested KWin processes crashed between 09:26 and 09:48 while exercising KWin's built-in Window View effect with Activities integration disabled. Their immediate crash path is strongly identified, but those morning crashes do not explain the later desktop freeze.

The current code hardens several real failure paths. It is not accurate to claim that the 14:02 freeze has been reproduced, attributed to this effect, or conclusively fixed.

## 1. Main desktop freeze

The main KWin process logged a 308 ms keyboard delay and libinput debounce expiry delays of roughly 740–2903 ms shortly before journal records stopped. Independent background services also logged delayed heartbeats. The last recorded journal message is at 14:02:31 EET; that timestamp is not proof of the exact moment the machine froze.

The available evidence did **not** show an AMDGPU ring timeout, GPU fault/recovery, kernel panic, hard/soft lockup, hung-task report, or OOM kill near the event. No contemporaneous main-KWin thread stack, DRM fence trace, or historical plugin maps/loaded-effects state was available. SSH, ping, VT, and SysRq responsiveness were not tested during the freeze.

The system plugin's metadata warning proves that KWin discovered plugin metadata. It does not prove that an effect instance was created or that the plugin was executing at 14:02. The system binary's Build ID differed from the local build associated with commit `c2cf2b6`, and historical maps for the main KWin process were unavailable. Therefore the evidence does not establish that `c2cf2b6` was loaded or caused the freeze.

**Conclusion:** compositor/rendering stall, system resource pressure, driver trouble, plugin lifecycle/ABI interaction, and later kernel/hardware lockup remain hypotheses. None is confirmed.

## 2. Separate nested KWin crashes

The six morning crashes came from nested KWin processes using a separate Wayland socket and the `--no-kactivities` option. Their primary-thread stacks share this path:

```text
WindowView DBus activation
  -> QuickSceneEffect / QML scene creation
  -> DesktopBackgroundItem::updateWindow
  -> QString::operator=
```

The KWin 6.8.2 binary disassembly shows the result of `Workspace::activities()` being used as a source QString without a null check. This is consistent with Activities integration being disabled by `--no-kactivities`. The immediate mechanism is strongly supported, but the inaccessible core files did not provide registers or `si_addr` to prove the exact null pointer value.

The QML scene involved is KWin's built-in Window View, not Magic Lamp QML. Subsequent Konsole crashes followed the nested compositor's Wayland connection breaking; they are a client-side cascade, not the initial KWin fault.

The "no-effect" runs unloaded the Magic Lamp instance before the trigger, but the plugin shared object remained mapped in the process. They are not a never-loaded control. This limits how strongly those runs can exclude constructor/unload side effects.

**Conclusion:** treat this as a separate KWin Activities-disabled / nested-session issue. Do not use it as an explanation for the main desktop freeze.

## 3. What `GridResolution=40` means

The saved configuration contained `GridResolution=40`; the source default was 30. In KWin 6.8, `WindowQuadList::makeGrid(int maxQuadSize)` uses the argument as a maximum mesh-cell edge size and increments its X/Y loops by that value:

- A zero value can prevent the loops from advancing and can hang the compositor.
- A positive value such as 40 does not trigger that zero-step failure.
- Smaller positive values create more quads and increase rendering cost; the setting is not a count of rows or columns.

KWin source: [itemgeometry.cpp, makeGrid()](https://github.com/KDE/kwin/blob/Plasma/6.8/src/scene/itemgeometry.cpp#L105-L156).

PR #3 clamps the runtime value to 16–99 before calling `makeGrid()`, and tests the boundaries. This closes the zero/unsafe-configuration path. It does **not** prove that the recorded value 40 caused the 14:02 freeze or that this mitigation fixes that incident.

The KCM label and tooltip describe the setting as maximum quad size in logical pixels. The legacy configuration key remains `GridResolution` for compatibility.

## 4. Current safeguards and their limits

- Runtime mesh-cell size is bounded before KWin's grid loop.
- Animation and geometry inputs are checked for finite, positive values before entering transform calculations.
- Task-button discovery retries are bounded; stale timers are generation-checked.
- Output/panel changes pause and retarget animations; unavailable targets fall back to normal KWin behavior.
- A rapid re-minimize with no current task-button target now cancels and unredirects the stale animation while a bounded pending reference holds visibility.

These are source-level safeguards. They are not evidence of a successful build, a successful QtTest run, or a desktop reproduction test.

## 5. Required next evidence

1. Obtain successful distro CI builds and CTest results for the stacked PR sequence.
2. On a desktop test system, record the exact plugin Build ID and prove whether the effect is never loaded, loaded but idle, or actively animating.
3. Test the same minimize/restore workload with the effect genuinely never loaded, not merely unloaded after startup.
4. If the freeze recurs, collect main-KWin thread stacks and kernel/DRM evidence while the system is unresponsive; test SSH, ping, VT, and SysRq so a compositor stall can be distinguished from a full kernel lockup.
5. Keep the nested `--no-kactivities` Window View crash as a separate test case. Do not use it as a general-purpose Magic Lamp integration test.

Until those results exist, keep the historical freeze investigation open.
