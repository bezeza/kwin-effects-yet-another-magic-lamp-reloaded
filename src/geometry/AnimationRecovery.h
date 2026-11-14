/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, either version 2 of
 * the License, or (at your option) any later version.
 */

#pragma once

namespace CoordinateMapper {

enum class AnimationRecoveryDecision {
    Retry,
    Resume,
    Drop,
};

// Decide whether a paused animation can be recovered without depending on
// KWin objects or timers. A missing task-button target is retried while the
// task manager is still settling, even if the user restored the window during
// the pause. Once the layout has settled, the effect falls back to KWin.
constexpr AnimationRecoveryDecision decideAnimationRecovery(
    bool windowAlive,
    bool fullScreenEffectActive,
    bool targetReady,
    bool taskButtonsSettled,
    int attempt,
    int maxAttempts) noexcept
{
    if (!windowAlive || fullScreenEffectActive) {
        return AnimationRecoveryDecision::Drop;
    }
    if (targetReady) {
        return AnimationRecoveryDecision::Resume;
    }
    if (!taskButtonsSettled && maxAttempts > 0 && attempt < maxAttempts) {
        return AnimationRecoveryDecision::Retry;
    }
    return AnimationRecoveryDecision::Drop;
}

} // namespace CoordinateMapper
