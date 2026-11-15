# Architecture overview

## Runtime components

`YetAnotherMagicLampEffect` coordinates the effect. It owns three helpers:

- `OutputManager` — observes KWin logical outputs and stores validated value snapshots.
- `GeometryResolver` — resolves the minimized task-button rectangle from Plasma's Wayland window-management data.
- `WindowTracker` — tracks each effect window's output identity and how long the task-button layout has had to settle.

`Model` owns the animation stage, timeline, target rectangle, direction, bump distance, and shape factor.

## Minimize flow

1. KWin emits `minimizedChanged`.
2. The effect asks `GeometryResolver` for the task-button rectangle.
3. If no valid target exists, the effect holds a visible reference and schedules a bounded retry.
4. When a target appears, `Model::start(Minimize)` validates the source and target geometry and calculates the animation
   geometry.
5. The effect redirects the window and the render path subdivides the quads using the bounded maximum quad size.

No guessed dock-center or cross-output fallback is used when a valid task-button target is unavailable. The effect lets KWin
perform the normal operation instead.

## Geometry resolution

The resolver reads `minimizedGeometries()` from the window-management interface, finds the panel window for each published
surface, and filters candidates to the minimized window's own logical output.

Plasma's button rectangle is surface-local. `surfaceToGlobal()` translates it by the panel's global logical position. It must
not be multiplied by output scale or rotated again: these task-button coordinates are already in logical surface/global
coordinates.

The resolver may inspect main windows for transient windows when the transient itself has no published task-button geometry.

## Output lifecycle

`OutputManager` observes screen add/remove and logical-output geometry, scale, transform, and mode changes. It emits a common
`outputsChanged` signal after the snapshot changes.

`WindowTracker` observes both effect-window geometry changes and the underlying KWin window's `outputChanged` signal. A
change restarts the settling interval because Plasma may publish or reposition task buttons asynchronously.

## Recovery flow

When output or panel topology changes:

1. The effect pauses active models without discarding their stage or elapsed progress.
2. A generation-tagged timer waits before resolving the new target.
3. A valid target updates the model's direction and geometry without replacing the timeline.
4. The same animation resumes.
5. A new minimize during recovery retargets the model before reversing its timeline.
   The start path also clears the pause state.
6. If the target never returns, the effect releases its redirection and visible
   reference and leaves the operation to KWin.

Panel close events mark the old panel as unavailable. Panel add events restart the settling interval and retry pending work.

## Safety boundaries

- Geometry is checked for finite coordinates, positive extents, and finite edges.
- Animation duration and bump distance are bounded at runtime.
- Mesh cell size is clamped before calling KWin's `makeGrid()`.
- Retry chains have finite limits and generation checks.
- KWin-dependent integration behavior is not considered proven by the pure helper tests.

## Test layers

- **Pure Qt tests:** coordinate adapters, output snapshots, geometry selection, model-geometry calculations, animation
  limits, and recovery decisions.
- **Build/CI tests:** compile the plugin and run all registered CTest targets on each supported distribution image.
- **Nested KWin tests:** validate real signals, panel publication timing, render lifecycle, and output hotplug.
- **Desktop QA:** validate the user's actual Fedora/KDE setup, especially multi-monitor and mixed-scale behavior.
