# Development audit: PR #3–#6

Audit date: 2026-11-14

## Scope and confidence

This is a source-level review of the four open, stacked pull requests and their current head commits. It is not a successful
build report and it is not a KWin runtime certification.

The review covered the changed-file lists and diffs, the effect lifecycle/render path, model geometry validation, output
tracking, task-button resolution, recovery policy, QtTest registration, and the four CI workflow definitions.

The review environment has a C++20 compiler but does not have the Qt 6 or KWin development toolchain. A standalone C++20
smoke test of the KWin-independent animation recovery policy header passes. GitHub currently reports no Actions runs or
commit statuses for the inspected PR #6/#7 heads. Full plugin compilation, QtTest execution, distro compatibility, and
compositor behavior remain unverified here.

## Current PR stack

Merge in this order; each PR is based on the preceding branch:

1. **PR #3** — `fix/guard-grid-cell-size` → `feature/coordinate-mapper-2.0`
2. **PR #4** — `fix/model-geometry-guards` → `fix/guard-grid-cell-size`
3. **PR #5** — `fix/ci-matrix-and-test-gates` → `fix/model-geometry-guards`
4. **PR #6** — `fix/animation-lifecycle-recovery` → `fix/ci-matrix-and-test-gates`
5. **This documentation/defensive-state change** is based on PR #6.

All four existing PRs were open, marked Draft, and reported mergeable at the time of this audit. "Mergeable" is GitHub's
branch-conflict result, not proof that a build or test passed.

## Findings by PR

### PR #3 — mesh subdivision guard

- The runtime value passed to `WindowQuadList::makeGrid()` is clamped to 16–99.
- The KCM control and KConfig schema express the same range.
- This prevents a zero step from reaching the subdivision loop and limits pathological mesh density.
- The pure boundary test covers zero, below-minimum, normal, above-maximum, and unsigned-maximum values.

**Important limitation:** KWin 6.8 `makeGrid(maxQuadSize)` advances its X/Y loops by `maxQuadSize`; zero can therefore
produce a non-advancing loop. The historical configuration recorded 40, a positive value that does not trigger this
specific hazard. Smaller positive values create more geometry. PR #3 closes the zero-value path, but does **not** explain
the 14:02 freeze. See [INCIDENT_POSTMORTEM.md](INCIDENT_POSTMORTEM.md).

### PR #4 — geometry and output lifecycle guards

- Geometry calculations reject non-finite, empty, overflowing, or unusable rectangles before they enter the transform path.
- Rotation tests assert KWin-compatible point mapping for all eight rotate/flip
  transforms, in addition to rectangle round trips.
- Animation duration and bump settings have runtime bounds, not only UI bounds.
- Output snapshots track geometry, scale, mode size, and transform changes.
- Window tracking observes output identity changes and restarts the task-button settling window.
- Geometry selection is constrained to the window's own output.

**Remaining verification:** the tests exercise pure geometry and adapter logic. They do not instantiate a real KWin output
manager, Wayland server, or compositor render path.

### PR #5 — CI matrix

- Fedora, Arch, openSUSE Tumbleweed, and Ubuntu workflow definitions enable `BUILD_TESTING`, build the project, and invoke
  CTest.
- The workflows use read-only repository-content permissions, concurrency cancellation, and bounded job timeouts.
- The updated workflow definitions add `fix/**` branch coverage, but they currently exist only in PR #5's branch.

**Blocking CI/bootstrap finding:** the default branch `master` still contains the older workflow definitions. Their
`push` and `pull_request` filters target `master` only. PR #3 targets `feature/coordinate-mapper-2.0`; PRs #4–#7
target stacked `fix/**` branches. Those PRs therefore do not match the workflows currently present on the default branch.

PR #5 introduces replacement workflows with `fix/**` filters, but a workflow file added only in a non-default stacked
branch cannot bootstrap its own execution for the other stacked PRs. The updated definitions need to reach the default
branch before they can reliably trigger validation for subsequent `fix/**` pull requests.

There is a second, separate signal: PRs #1 and #2 target `master` and should match the legacy pull-request filters, yet
the repository-wide Actions run list and the inspected commit statuses still contain no runs/checks. That warrants an
owner-side check of whether GitHub Actions and the individual workflows are enabled or restricted by repository policy.

No distro build or QtTest result can be treated as passed. Before relying on CI, verify the repository Actions settings and
ensure a workflow present on the default branch covers the intended PR base branches.

### PR #6 — animation and panel lifecycle recovery

- Active models are paused during output/panel changes and are retargeted before resuming.
- Retry chains are bounded and generation-tagged so stale timers cannot drive a newer attempt.
- A new minimize during recovery retargets the model before reversing it and clears its pause.
- Failed retargets release offscreen redirection.
- If a rapid re-minimize has no task-button target, the old animation is cancelled and unredirected. A bounded pending
  reference keeps the window visible while the resolver retries; if no target appears, KWin hides it normally.
- A new minimize during recovery retargets the existing model before reversing
  it and clears the pause; failed retargets release offscreen redirection.
- A pure start-state helper has regression tests for paused timelines and fresh starts.
- Closing panels are excluded from task-button candidate selection.
- New panels restart the settling window and pending recovery/retry work.
- The recovery policy is isolated in a KWin-independent helper and has focused policy tests.

**Remaining verification:** the recovery tests cover decisions, not actual signal ordering or rendering. A nested KWin
session is still required for panel replacement, hotplug, fullscreen takeover, rapid restore, and effect-unload scenarios.

## Integration path and PR #2 dependency

PR #2 is still open against `master` at head `5a639f8`. The `feature/coordinate-mapper-2.0`
branch used as PR #3's base is a descendant of that PR #2 head and is 11 commits ahead of it.
PR #3–#7 are therefore stacked on a feature branch that already contains PR #2's history;
they are not a direct stack on `master`.

Merging PR #3 into its current base, then #4, #5, #6, and #7 into their respective
intermediate branches, does not by itself deliver the resulting code to `master`.
Before integration, choose one explicit route:

1. Merge PR #2 first, then retarget the root of the remaining stack after checking the resulting diff against the updated `master`; or
2. Use one integration PR from the feature branch (updated by the stacked PRs) to `master`, and treat PR #2 as superseded by that integration.

Do not merge overlapping PR #2 and an integration PR independently. Keep this decision open until the stack has successful CI and the required runtime QA.

A separate draft PR #8 proposes bringing the updated CI workflows onto `master`.
It is not merged, so the default-branch workflow configuration is unchanged.

## Historical incident

The 3 November main-desktop freeze and the morning nested-KWin Window View crashes are separate incidents. The freeze
remains unexplained; the nested crashes strongly implicate KWin's Activities-disabled Window View path. Neither proves
that the Magic Lamp effect caused the 14:02 freeze. See [INCIDENT_POSTMORTEM.md](INCIDENT_POSTMORTEM.md).

## Defensive change in this branch

The `Model` class had several scalar/pointer members without in-class initializers. Normal effect flow calls `start()` before
using them, but default-constructing a model left its state dependent on call order. This branch gives it an inert, completed
default state and explicit geometry/direction defaults. A successful `start()` still initializes the active animation state.

## Open risks

1. The original freeze at `GridResolution=40` has no confirmed root cause in the current evidence.
2. No Actions results exist for PR #3–#6.
3. KWin 6.6, 6.7, and 6.8 compile compatibility has not been independently reproduced by the current CI.
4. Actual output hotplug and Plasma panel lifecycle behavior still require nested-session tests.
5. Mixed-DPI, rotated outputs, and multi-task-manager grouping need manual validation.

## Merge gate

Do not merge this stack based only on the `mergeable` flag. Require successful CI on the intended base line, then complete
the manual QA matrix in `QA_MATRIX.md`. Keep the historical freeze investigation open until a reproducible trigger or a
confirmed root cause is recorded.
